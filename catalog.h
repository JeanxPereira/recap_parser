#pragma once

#include <string>
#include <stack>
#include <vector>
#include <map>
#include <memory>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <filesystem>
#include <pugixml.hpp>
#include <fmt/core.h>
#include <fmt/format.h>

enum class DataType {
    BOOL,
    INT,
    INT16,
    INT64,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    FLOAT,
    VECTOR2,
    VECTOR3,
    QUATERNION,
    CHAR,
    CHAR_PTR,
    KEY,
    ASSET,
    NULLABLE,
    ARRAY,
    ENUM,
    STRUCT
};

struct TypeDefinition {
    std::string name;
    DataType type;
    size_t size;
    std::string targetType;

    TypeDefinition(const std::string& name, DataType type, size_t size)
        : name(name), type(type), size(size), targetType("") {
    }

    TypeDefinition(const std::string& name, DataType type, size_t size, const std::string& targetType)
        : name(name), type(type), size(size), targetType(targetType) {
    }
};

class OffsetManager {
private:
    size_t primaryOffset = 0;
    size_t secondaryOffset = 0;
    std::ifstream& fileStream;

public:
    OffsetManager(std::ifstream& fs) : fileStream(fs) {}

    void setPrimaryOffset(size_t offset) {
        primaryOffset = offset;
    }

    void setSecondaryOffset(size_t offset) {
        secondaryOffset = offset;
    }

    size_t getPrimaryOffset() const {
        return primaryOffset;
    }

    size_t getSecondaryOffset() const {
        return secondaryOffset;
    }

    void advancePrimary(size_t bytes) {
        primaryOffset += bytes;
    }

    void advanceSecondary(size_t bytes) {
        secondaryOffset += bytes;
    }

    bool isValidOffset(size_t offset, size_t size) {
        fileStream.seekg(0, std::ios::end);
        size_t fileSize = fileStream.tellg();
        return (offset + size <= fileSize);
    }

    template<typename T>
    T readPrimary() {
        if (!isValidOffset(primaryOffset, sizeof(T))) {
            throw std::runtime_error("Attempted to read beyond end of file at offset " +
                std::to_string(primaryOffset));
        }

        T value;
        fileStream.seekg(primaryOffset);
        fileStream.read(reinterpret_cast<char*>(&value), sizeof(T));
        primaryOffset += sizeof(T);
        return value;
    }

    template<typename T>
    T readSecondary() {
        T value;
        fileStream.seekg(secondaryOffset);
        fileStream.read(reinterpret_cast<char*>(&value), sizeof(T));
        secondaryOffset += sizeof(T);
        return value;
    }

    std::string readString(bool useSecondary = false) {
        size_t currentOffset = useSecondary ? secondaryOffset : primaryOffset;
        fileStream.seekg(currentOffset);

        std::string result;
        char c;
        while (fileStream.get(c) && c != '\0') {
            result += c;
        }

        size_t newOffset = currentOffset + result.length() + 1;
        if (useSecondary) {
            secondaryOffset = newOffset;
        }
        else {
            primaryOffset = newOffset;
        }

        return result;
    }
};

struct StructMember {
    std::string name;
    std::string typeName;
    size_t offset;
    bool useSecondaryOffset;

    StructMember(const std::string& name, const std::string& typeName, size_t offset, bool useSecondaryOffset = false)
        : name(name), typeName(typeName), offset(offset), useSecondaryOffset(useSecondaryOffset) {
    }
};

class StructDefinition {
private:
    std::string name;
    size_t fixedSize;
    std::vector<StructMember> members;

public:
    StructDefinition(const std::string& name, size_t fixedSize = 0)
        : name(name), fixedSize(fixedSize) {
    }

    const std::string& getName() const {
        return name;
    }

    size_t getFixedSize() const {
        return fixedSize;
    }

    StructDefinition* add(const std::string& name, const std::string& typeName, size_t offset, bool useSecondaryOffset = false) {
        members.emplace_back(name, typeName, offset, useSecondaryOffset);
        return this;
    }

    StructDefinition* add(const std::string& name, const std::string& typeName,
        std::shared_ptr<StructDefinition> targetStruct, size_t offset,
        bool useSecondaryOffset = false) {
        if (typeName == "nullable") {
            std::string specificType = "nullable:" + targetStruct->getName();
            members.emplace_back(name, specificType, offset, useSecondaryOffset);
        }
        else if (typeName == "array") {
            std::string specificType = "array:" + targetStruct->getName();
            members.emplace_back(name, specificType, offset, useSecondaryOffset);
        }
        else {
            members.emplace_back(name, typeName, offset, useSecondaryOffset);
        }
        return this;
    }

    const std::vector<StructMember>& getMembers() const {
        return members;
    }
};

struct FileTypeInfo {
    std::vector<std::string> structTypes;
    size_t secondaryOffsetStart;

    FileTypeInfo(const std::vector<std::string>& types, size_t offsetStart = 0)
        : structTypes(types), secondaryOffsetStart(offsetStart) {
    }
};

class Catalog {
private:
    std::unordered_map<std::string, TypeDefinition> types;
    std::unordered_map<std::string, std::shared_ptr<StructDefinition>> structs;
    std::unordered_map<std::string, FileTypeInfo> fileTypes;

public:
    Catalog();

    TypeDefinition* addType(const std::string& name, DataType type, size_t size) {
        types.emplace(name, TypeDefinition(name, type, size));
        return &types.at(name);
    }

    TypeDefinition* addType(const std::string& name, DataType type, size_t size, const std::string& targetType) {
        types.emplace(name, TypeDefinition(name, type, size, targetType));
        return &types.at(name);
    }

    std::shared_ptr<StructDefinition> add_struct(const std::string& name, size_t fixedSize = 0) {
        auto structDef = std::make_shared<StructDefinition>(name, fixedSize);
        structs[name] = structDef;
        return structDef;
    }

    void registerFileType(const std::string& extension, const std::vector<std::string>& structTypes, size_t secondaryOffsetStart = 0) {
        std::string ext = extension;
        for (auto& c : ext) {
            c = std::tolower(c);
        }
        fileTypes.emplace(ext, FileTypeInfo(structTypes, secondaryOffsetStart));
    }

    void registerNullableType(const std::string& targetStructName) {
        std::string nullableTypeName = "nullable:" + targetStructName;
        // Só registra se ainda não existir
        if (types.find(nullableTypeName) == types.end()) {
            addType(nullableTypeName, DataType::NULLABLE, 4, targetStructName);
        }
    }

    void registerArrayType(const std::string& targetStructName) {
        std::string arrayTypeName = "array:" + targetStructName;
        // Registra apenas se não existir
        if (types.find(arrayTypeName) == types.end()) {
            addType(arrayTypeName, DataType::ARRAY, 4, targetStructName);
        }
    }

    const TypeDefinition* getType(const std::string& name) const {
        auto it = types.find(name);
        if (it != types.end()) {
            return &it->second;
        }
        return nullptr;
    }

    std::shared_ptr<StructDefinition> getStruct(const std::string& name) const {
        auto it = structs.find(name);
        if (it != structs.end()) {
            return it->second;
        }
        return nullptr;
    }
    const FileTypeInfo* getFileType(const std::string& extension) const {
        std::string ext = extension;
        for (auto& c : ext) {
            c = std::tolower(c);
        }

        auto it = fileTypes.find(ext);
        if (it != fileTypes.end()) {
            return &it->second;
        }

        if (!ext.empty() && ext[0] == '.') {
            ext = ext.substr(1);
            it = fileTypes.find("." + ext);
            if (it != fileTypes.end()) {
                return &it->second;
            }
        }

        return nullptr;
    }

    void initialize();
};

class Parser {
private:
    const Catalog& catalog;
    OffsetManager offsetManager;
    std::ifstream fileStream;
    std::string filename;

    bool secOffsetStruct = false;
    size_t currentStructBaseOffset = 0;
    std::stack<size_t> structBaseOffsetStack;

    bool processingArrayElement = false;
    size_t arrayElementBaseOffset = 0;

    bool debugMode;
    bool xmlMode;
    int indentLevel = 0;
    pugi::xml_document xmlDoc;
    pugi::xml_node currentXmlNode;

    std::string getIndent() const {
        return std::string(indentLevel * 4, ' ');
    }

    void logParse(const std::string& message) {
        if (debugMode) {
            fmt::print("({}, {}) {}{}\n",
                offsetManager.getPrimaryOffset(),
                offsetManager.getSecondaryOffset(),
                getIndent(),
                message);
        }
        else {
            fmt::print("{}{}\n", getIndent(), message);
        }
    }

    void parseStruct(const std::string& structName);
    void parseMember(const StructMember& member, const std::shared_ptr<StructDefinition>& parentStruct);

public:
    Parser(const Catalog& catalog, const std::string& filename, bool debugMode = false, bool xmlMode = false)
        : catalog(catalog), offsetManager(fileStream), filename(filename), debugMode(debugMode), xmlMode(xmlMode) {
    }

    bool parse();
    void exportXml(const std::string& outputFile);
};