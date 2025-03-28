#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <variant>
#include <optional>
#include <functional>

// Forward declarations
class DataType;
class StructType;
class Catalog;

// Base class for all data types
class DataType {
public:
    DataType(const std::string& name, size_t size);
    virtual ~DataType() = default;

    const std::string& getName() const;
    size_t getSize() const;
    virtual bool isSpecialType() const; // Types that use secondary offset

private:
    std::string name;
    size_t size;
};

// Special type for types that use secondary offset
class SpecialType : public DataType {
public:
    SpecialType(const std::string& name, size_t size);

    bool isSpecialType() const override;
};

// Class for struct types
class StructType : public DataType {
public:
    StructType(const std::string& name, size_t size = 0);

    struct Field {
        std::string name;
        std::shared_ptr<DataType> type;
        size_t offset;
        bool useSecondaryOffset;
        std::variant<std::monostate, std::string, std::shared_ptr<StructType>> subType;
    };

    void addField(const std::string& name, std::shared_ptr<DataType> type, size_t offset,
        bool useSecondaryOffset = false,
        const std::variant<std::monostate, std::string, std::shared_ptr<StructType>>& subType = {});

    const std::vector<Field>& getFields() const;

private:
    std::vector<Field> fields;
};

// Class for file type definitions
class FileType {
public:
    FileType(const std::string& extension, const std::vector<std::string>& types, size_t secondaryOffset = 0);

    const std::string& getExtension() const;
    const std::vector<std::string>& getTypes() const;
    size_t getSecondaryOffset() const;

private:
    std::string extension;
    std::vector<std::string> types;
    size_t secondaryOffset;
};

// Main catalog class
class Catalog {
public:
    Catalog();

    void registerFileType(const std::string& extension, const std::vector<std::string>& types, size_t secondaryOffset = 0);

    std::shared_ptr<DataType> add(const std::string& name, size_t size);
    std::shared_ptr<StructType> addStruct(const std::string& name, size_t size = 0);

    std::shared_ptr<DataType> getType(const std::string& name) const;
    std::shared_ptr<StructType> getStructType(const std::string& name) const;
    const FileType* getFileType(const std::string& extension) const;

    void initializeStandardTypes();

private:
    std::unordered_map<std::string, std::shared_ptr<DataType>> types;
    std::unordered_map<std::string, FileType> fileTypes;
};