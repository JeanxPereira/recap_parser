#pragma once

#include "exporter.h"
#include <string>
#include <stack>
#include <vector>
#include <map>
#include <memory>
#include <unordered_map>
#include <functional>
#include <fstream>

#ifndef _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#endif

#ifndef _SILENCE_ALL_MS_EXT_DEPRECATION_WARNING
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNING
#endif

#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

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
    GUID,
    VECTOR2,
    VECTOR3,
    QUATERNION,
    LOCALIZEDASSETSTRING,
    CHAR,
    CHAR_PTR,
    KEY,
    ASSET,
    CKEYASSET,
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
    size_t displaySecondaryOffset = 0;

public:
    OffsetManager(std::ifstream& fs) : fileStream(fs) {}

    void setPrimaryOffset(size_t offset) {
        primaryOffset = offset;
    }

    void setSecondaryOffset(size_t offset) {
        secondaryOffset = offset;
    }

    void setDisplaySecondaryOffset(size_t offset) {
        displaySecondaryOffset = offset;
    }

    size_t getPrimaryOffset() const {
        return primaryOffset;
    }

    size_t getSecondaryOffset() const {
        return displaySecondaryOffset > 0 ? displaySecondaryOffset : secondaryOffset;
    }

    size_t getRealSecondaryOffset() const {
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

    template<typename T>
    T readAt(size_t offset) {
        if (!isValidOffset(offset, sizeof(T))) {
            throw std::runtime_error("Attempted to read beyond end of file at offset " +
                std::to_string(offset));
        }

        std::streampos currentPos = fileStream.tellg();

        fileStream.seekg(offset);
        T value;
        fileStream.read(reinterpret_cast<char*>(&value), sizeof(T));

        fileStream.seekg(currentPos);

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
    std::string elementType;
    bool hasCustomName;
    size_t countOffset;

    StructMember(const std::string& name, const std::string& typeName, size_t offset,
        bool useSecondaryOffset = false, bool hasCustomName = false, size_t countOffset = 0)
        : name(name), typeName(typeName), offset(offset),
        useSecondaryOffset(useSecondaryOffset), elementType(""),
        hasCustomName(hasCustomName), countOffset(countOffset) {
    }

    StructMember(const std::string& name, const std::string& typeName,
        const std::string& elementType, size_t offset,
        bool useSecondaryOffset = false, bool hasCustomName = false, size_t countOffset = 0)
        : name(name), typeName(typeName), offset(offset),
        useSecondaryOffset(useSecondaryOffset), elementType(elementType),
        hasCustomName(hasCustomName), countOffset(countOffset) {
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


    StructDefinition* addArray(const std::string& name, const std::string& elementType, size_t offset,
        bool useSecondaryOffset = false, size_t countOffset = 0) {
        members.emplace_back(name, "array", elementType, offset, useSecondaryOffset, false, countOffset);
        return this;
    }

    StructDefinition* addStructArray(const std::string& name, std::shared_ptr<StructDefinition> elementStruct, size_t offset,
        size_t countOffset = 0, bool useSecondaryOffset = false) {
        members.emplace_back(name, "array", elementStruct->getName(), offset, useSecondaryOffset, false, countOffset);
        return this;
    }

    StructDefinition* add(std::shared_ptr<StructDefinition> structDef, size_t offset) {
        members.emplace_back(structDef->getName(), "struct:" + structDef->getName(), offset, false);
        return this;
    }

    StructDefinition* add(const std::string& name, std::shared_ptr<StructDefinition> structDef, size_t offset) {
        members.emplace_back(name, "struct:" + structDef->getName(), offset, false, true);
        return this;
    }

    StructDefinition* add(const std::string& name, const std::string& typeName,
        std::shared_ptr<StructDefinition> targetStruct, size_t offset,
        bool useSecondaryOffset = false) {
        if (typeName == "nullable") {
            std::string specificType = "nullable:" + targetStruct->getName();
            bool customName = (name != targetStruct->getName());
            members.emplace_back(name, specificType, offset, useSecondaryOffset, customName);
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
    std::unordered_map<std::string, FileTypeInfo> exactFileNames;

public:
    Catalog();

    TypeDefinition* addType(const std::string& name, DataType type, size_t size) {
        types.emplace(name, TypeDefinition(name, type, size));
        return &types.at(name);
    }

    TypeDefinition* addArrayType(const std::string& name, const std::string& elementType, size_t size) {
        std::string arrayTypeName = "array:" + elementType;
        return addType(arrayTypeName, DataType::ARRAY, size, elementType);
    }

    TypeDefinition* addType(const std::string& name, DataType type, size_t size, const std::string& targetType) {
        types.emplace(name, TypeDefinition(name, type, size, targetType));
        return &types.at(name);
    }

    std::shared_ptr<StructDefinition> add_struct(const std::string& name, size_t fixedSize = 0) {
        auto structDef = std::make_shared<StructDefinition>(name, fixedSize);
        structs[name] = structDef;

        // direct
        addType("struct:" + name, DataType::STRUCT, fixedSize, name);

        // nullable
        registerNullableType(name);

        return structDef;
    }

    std::vector<std::string> getRegisteredFileTypes() const {
        std::vector<std::string> result;

        for (const auto& pair : fileTypes) {
            std::string ext = pair.first;
            if (!ext.empty() && ext[0] != '.') {
                ext = "." + ext;
            }

            std::string structInfo;
            if (!pair.second.structTypes.empty()) {
                structInfo = " (" + pair.second.structTypes[0];
                for (size_t i = 1; i < pair.second.structTypes.size(); i++) {
                    structInfo += ", " + pair.second.structTypes[i];
                }
                structInfo += ")";
            }

            result.push_back(ext + structInfo);
        }

        for (const auto& pair : exactFileNames) {
            std::string info = pair.first;

            if (!pair.second.structTypes.empty()) {
                info += " (" + pair.second.structTypes[0];
                for (size_t i = 1; i < pair.second.structTypes.size(); i++) {
                    info += ", " + pair.second.structTypes[i];
                }
                info += ")";
            }

            result.push_back(info + " [exact]");
        }

        std::sort(result.begin(), result.end());

        return result;
    }

    void registerFileType(const std::string& extension, const std::vector<std::string>& structTypes, size_t secondaryOffsetStart = 0) {
        std::string ext = extension;
        for (auto& c : ext) {
            c = std::tolower(c);
        }
        fileTypes.emplace(ext, FileTypeInfo(structTypes, secondaryOffsetStart));
    }

    void registerFileName(const std::string& fileName, const std::vector<std::string>& structTypes, size_t secondaryOffsetStart = 0) {
        std::string name = fileName;
        for (auto& c : name) {
            c = std::tolower(c);
        }
        exactFileNames.emplace(name, FileTypeInfo(structTypes, secondaryOffsetStart));
    }

    void registerNullableType(const std::string& targetStructName) {
        std::string nullableTypeName = "nullable:" + targetStructName;
        if (types.find(nullableTypeName) == types.end()) {
            addType(nullableTypeName, DataType::NULLABLE, 4, targetStructName);
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

    const FileTypeInfo* getFileTypeByName(const std::string& filename) const {
        std::string name = fs::path(filename).filename().string();
        for (auto& c : name) {
            c = std::tolower(c);
        }

        auto it = exactFileNames.find(name);
        if (it != exactFileNames.end()) {
            return &it->second;
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

    size_t totalArraySize = 0;

    bool isInsideNullable = false;
    bool secOffsetStruct = false;
    std::stack<size_t> structOffsetStack;

    size_t structBaseStartOffset = 0;
    size_t startNullableOffset = 0;
    size_t structBaseOffset = 0;
    size_t currentStructBaseOffset = 0;
    std::stack<size_t> structBaseOffsetStack;

    bool processingArrayElement = false;
    bool isProcessingRootTag = false;
    bool debugMode;
    bool exportMode;
    int indentLevel = 0;

    std::unique_ptr<FormatExporter> exporter;

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

    void parseStruct(const std::string& structName, int arrayIndex = -1);
    void parseMember(const StructMember& member, const std::shared_ptr<StructDefinition>& parentStruct, size_t arraySize = 0);
public:
    Parser(const Catalog& catalog, const std::string& filename, bool debugMode = false, const std::string& exportFormat = "xml")
        : catalog(catalog), offsetManager(fileStream), filename(filename),
        debugMode(debugMode), exportMode(exportFormat != "none") {

        if (exportFormat != "none") {
            exporter = ExporterFactory::createExporter(exportFormat);
        }
    }

    bool parse();
    void exportToFile(const std::string& outputFile);
};