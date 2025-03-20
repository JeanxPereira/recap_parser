#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <iostream>
#include <variant>

namespace parser {
    class DataType;
    class Field;
    class StructDefinition;
    class StructType;
    class NullableType;

    using FieldValue = std::variant<bool, int, float, uint32_t, uint64_t, int64_t, std::string>;

    struct StructCreationResult {
        std::shared_ptr<StructDefinition> definition;
        std::shared_ptr<DataType> type;
    };

    class DataType {
    public:
        DataType(const std::string& name, size_t size) : name(name), size(size) {}

        virtual ~DataType() = default;

        std::string getName() const { return name; }
        size_t getSize() const { return size; }

        virtual bool isStructType() const { return false; }
        virtual std::string getStructName() const { return ""; }

        virtual bool isNullableType() const { return false; }
        virtual std::shared_ptr<DataType> getContainedType() const { return nullptr; }

    private:
        std::string name;
        size_t size;
    };

    class StructType : public DataType {
    public:
        StructType(const std::string& structName)
            : DataType("struct", 0), structName(structName) {
        }

        bool isStructType() const override { return true; }
        std::string getStructName() const override { return structName; }

    private:
        std::string structName;
    };

    class NullableType : public DataType {
    public:
        NullableType(std::shared_ptr<DataType> containedType)
            : DataType("nullable", 4), containedType(containedType) {
        }

        bool isNullableType() const override { return true; }
        std::shared_ptr<DataType> getContainedType() const override { return containedType; }

    private:
        std::shared_ptr<DataType> containedType;
    };

    class dNullableType : public DataType {
    public:
        dNullableType(std::shared_ptr<DataType> containedType)
            : DataType("dNullable", 4), containedType(containedType) {
        }

        bool isNullableType() const override { return true; }
        std::shared_ptr<DataType> getContainedType() const override { return containedType; }

    private:
        std::shared_ptr<DataType> containedType;
    };

    class Field {
    public:
        Field(const std::string& name, std::shared_ptr<DataType> type, size_t offset, bool usesSecondaryPointer = false)
            : name(name), type(type), offset(offset), usesSecondaryPointer(usesSecondaryPointer), paddingSize(0) {
        }
        bool hasPadding() const { return paddingBytes > 0; }
        size_t getPaddingBytes() const { return paddingBytes; }
        void setPadding(size_t bytes) { paddingBytes = bytes; }


        std::string getName() const { return name; }
        std::shared_ptr<DataType> getType() const { return type; }
        size_t getOffset() const { return offset; }
        bool getUsesSecondaryPointer() const { return usesSecondaryPointer; }

        void setPaddingSize(size_t size) { paddingSize = size; }
        size_t getPaddingSize() const { return paddingSize; }
        bool isPadding() const { return type->getName() == "padding"; }

    private:
        std::string name;
        std::shared_ptr<DataType> type;
        size_t offset;
        bool usesSecondaryPointer;
        size_t paddingBytes = 0;
        size_t paddingSize;
    };

    class Catalog;

    class StructDefinition {
    public:
        StructDefinition(const std::string& name, size_t fixedSize = 0)
            : name(name), fixedSize(fixedSize) {
        }

        size_t getFixedSize() const { return fixedSize; }

        void addField(const std::string& name, std::shared_ptr<DataType> type,
            size_t offset, bool usesSecondaryPointer = false, size_t paddingAfter = 0) {
            auto field = std::make_shared<Field>(name, type, offset, usesSecondaryPointer);
            if (paddingAfter > 0) {
                field->setPadding(paddingAfter);
            }
            fields.push_back(field);
        }

        void addField(const std::string& name,
            std::shared_ptr<DataType> type,
            std::shared_ptr<DataType> containedType,
            size_t offset,
            bool usesSecondaryPointer);

        std::string getName() const { return name; }
        const std::vector<std::shared_ptr<Field>>& getFields() const { return fields; }

        void addStructField(const std::string& name, const std::string& structName, size_t offset, bool usesSecondaryPointer = false);

        struct PaddingInfo {
            size_t bytes;
            bool usesSecondaryPointer;
        };

        void addPadding(size_t paddingBytes, bool usesSecondaryPointer = false) {
            paddingInfos.push_back({ paddingBytes, usesSecondaryPointer });
        }

        const std::vector<PaddingInfo>& getPaddingInfos() const { return paddingInfos; }

    private:
        std::string name;
        std::vector<std::shared_ptr<Field>> fields;
        std::vector<PaddingInfo> paddingInfos;
        size_t fixedSize;
    };

    struct FileTypeConfig {
        std::vector<std::string> rootStructs;
        size_t secondaryPointerOffset = 0;
        bool hasRepeatingStructs = false;
        std::string headerStruct;
        std::string entryStruct;
        size_t headerSize = 0;
        size_t entrySize = 0;
        size_t entryCountOffset = 0;
    };

    class Catalog {
    public:
        static Catalog& getInstance() {
            static Catalog instance;
            return instance;
        }


        std::shared_ptr<DataType> addType(const std::string& name, size_t size) {
            auto type = std::make_shared<DataType>(name, size);
            dataTypes[name] = type;
            return type;
        }

        std::shared_ptr<DataType> addNullableType(std::shared_ptr<DataType> containedType) {
            auto type = std::make_shared<NullableType>(containedType);
            return type;
        }

        std::shared_ptr<DataType> addDirectNullableType(std::shared_ptr<DataType> containedType) {
            auto type = std::make_shared<dNullableType>(containedType);
            return type;
        }

        std::shared_ptr<DataType> addStructType(const std::string& structName) {
            auto type = std::make_shared<StructType>(structName);
            dataTypes["struct:" + structName] = type;
            return type;
        }

        std::shared_ptr<StructDefinition> addStruct(const std::string& name, size_t fixedSize = 0) {
            auto structDef = std::make_shared<StructDefinition>(name, fixedSize);
            structDefinitions[name] = structDef;

            auto structType = std::make_shared<StructType>(name);
            dataTypes["struct:" + name] = structType;

            return structDef;
        }

        std::shared_ptr<DataType> getType(const std::string& name) const {
            auto it = dataTypes.find(name);
            if (it != dataTypes.end()) {
                return it->second;
            }
            return nullptr;
        }

        std::shared_ptr<StructDefinition> getStruct(const std::string& name) const {
            auto it = structDefinitions.find(name);
            if (it != structDefinitions.end()) {
                return it->second;
            }
            return nullptr;
        }

        const std::unordered_map<std::string, std::shared_ptr<StructDefinition>>& getAllStructs() const {
            return structDefinitions;
        }

        void registerFileType(const std::string& extension, 
                     std::vector<std::string> rootStructs, 
                     size_t secondaryPointerOffset,
                     bool hasRepeatingStructs = false,
                     const std::string& headerStruct = "",
                     const std::string& entryStruct = "",
                     size_t headerSize = 0,
                     size_t entrySize = 0,
                     size_t entryCountOffset = 0) {
    fileTypes[extension] = { 
        rootStructs, 
        secondaryPointerOffset,
        hasRepeatingStructs,
        headerStruct,
        entryStruct,
        headerSize,
        entrySize,
        entryCountOffset
    };
}

        const FileTypeConfig& getFileTypeConfig(const std::string& extension) const {
            static FileTypeConfig empty = { {}, 0 };
            auto it = fileTypes.find(extension);
            if (it != fileTypes.end()) {
                return it->second;
            }
            return empty;
        }

        StructCreationResult createStruct(const std::string& name) {
            auto structDef = std::make_shared<StructDefinition>(name);
            structDefinitions[name] = structDef;

            auto structType = std::make_shared<StructType>(name);
            dataTypes["struct:" + name] = structType;

            return StructCreationResult{ structDef, structType };
        }

    private:
        Catalog() = default;

        std::unordered_map<std::string, std::shared_ptr<DataType>> dataTypes;
        std::unordered_map<std::string, std::shared_ptr<StructDefinition>> structDefinitions;
        std::unordered_map<std::string, FileTypeConfig> fileTypes;
    };

    void initializeCatalog();

} // namespace parser