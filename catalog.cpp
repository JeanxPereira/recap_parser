#include "catalog.h"
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <pugixml.hpp>

namespace recap {
    Type::Type(const std::string& name, size_t size) : name_(name), size_(size) {}

    const std::string& Type::getName() const { return name_; }
    size_t Type::getSize() const { return size_; }

    std::string Type::formatLog(const std::string& name, const std::string& value) const {
        return "parse_member_" + name_ + "(" + name + ", " + value + ")";
    }

    void Type::formatXml(pugi::xml_node& parent, const std::string& name, const std::string& value) const {
        parent.append_child(name.c_str()).text().set(value.c_str());
    }
    Field::Field(const std::string& name, TypePtr type, size_t offset, bool useSecondaryOffset)
        : name_(name), type_(type), offset_(offset), useSecondaryOffset_(useSecondaryOffset) {
    }

    const std::string& Field::getName() const { return name_; }
    TypePtr Field::getType() const { return type_; }
    size_t Field::getOffset() const { return offset_; }
    bool Field::usesSecondaryOffset() const { return useSecondaryOffset_; }
    StructType::StructType(const std::string& name, size_t fixedSize, Catalog* catalog)
        : Type(name, fixedSize), fixedSize_(fixedSize), catalog_(catalog) {
    }

    StructType* StructType::addField(const std::string& name, TypePtr type, size_t offset, bool useSecondaryOffset) {
        fields_.push_back(std::make_shared<Field>(name, type, offset, useSecondaryOffset));
        return this;
    }

    StructType* StructType::addNullableField(const std::string& name, TypePtr contentType, size_t offset) {
        if (!catalog_) {
            std::cerr << "Error: Cannot create nullable field without catalog reference" << std::endl;
            return this;
        }
        auto nullableType = catalog_->createNullableType();
        if (auto nt = std::dynamic_pointer_cast<NullableType>(nullableType)) {
            nt->setContentType(contentType);
        }
        fields_.push_back(std::make_shared<Field>(name, nullableType, offset, true));

        return this;
    }

    std::string StructType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        return "struct:" + name_;
    }

    std::string StructType::formatLog(const std::string& name, const std::string& value) const {
        return "parse_struct(" + name_ + ")";
    }

    void StructType::formatXml(pugi::xml_node& parent, const std::string& name, const std::string& value) const {
        parent.append_child(name.c_str());
    }

    const std::vector<FieldPtr>& StructType::getFields() const {
        return fields_;
    }
    FileType::FileType(const std::string& extension, const std::vector<std::string>& rootStructs, size_t secondaryOffset)
        : extension_(extension), rootStructs_(rootStructs), secondaryOffset_(secondaryOffset) {
    }

    const std::string& FileType::getExtension() const { return extension_; }
    const std::vector<std::string>& FileType::getRootStructs() const { return rootStructs_; }
    size_t FileType::getSecondaryOffset() const { return secondaryOffset_; }
    PrimitiveType::PrimitiveType(const std::string& name, size_t size) : Type(name, size) {}

    std::string PrimitiveType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        return "primitive";
    }

    BoolType::BoolType() : PrimitiveType("bool", 1) {}

    std::string BoolType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        uint8_t value = 0;
        file.seekg(primaryOffset);
        file.read(reinterpret_cast<char*>(&value), sizeof(value));
        primaryOffset += sizeof(value);
        return value ? "true" : "false";
    }

    IntegerType::IntegerType(const std::string& name, size_t size) : PrimitiveType(name, size) {}

    std::string IntegerType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        int64_t value = 0;
        file.seekg(primaryOffset);
        file.read(reinterpret_cast<char*>(&value), size_);
        primaryOffset += size_;
        return std::to_string(value);
    }

    FloatType::FloatType() : PrimitiveType("float", 4) {}

    std::string FloatType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        float value = 0.0f;
        file.seekg(primaryOffset);
        file.read(reinterpret_cast<char*>(&value), sizeof(value));
        primaryOffset += sizeof(value);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(5) << value;
        return ss.str();
    }

    Uint64Type::Uint64Type() : IntegerType("uint64_t", 8) {
    }

    std::string Uint64Type::formatLog(const std::string& name, const std::string& value) const {
        uint64_t intValue = 0;
        try {
            intValue = std::stoull(value);
        }
        catch (...) {
            return IntegerType::formatLog(name, value);
        }
        std::stringstream hexStream;
        hexStream << "0x" << std::hex << std::uppercase << intValue;

        return "parse_member_" + name_ + "(" + name + ", " + hexStream.str() + ")";
    }

    void Uint64Type::formatXml(pugi::xml_node& parent, const std::string& name, const std::string& value) const {
        uint64_t intValue = 0;
        try {
            intValue = std::stoull(value);
        }
        catch (...) {
            IntegerType::formatXml(parent, name, value);
            return;
        }
        std::stringstream hexStream;
        hexStream << "0x" << std::hex << std::uppercase << intValue;

        parent.append_child(name.c_str()).text().set(hexStream.str().c_str());
    }

    DoubleType::DoubleType() : PrimitiveType("double", 8) {}

    std::string DoubleType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        double value = 0.0;
        file.seekg(primaryOffset);
        file.read(reinterpret_cast<char*>(&value), sizeof(value));
        primaryOffset += sizeof(value);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(5) << value;
        return ss.str();
    }

    CharType::CharType() : PrimitiveType("char", 1) {}

    std::string CharType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        char value = 0;
        file.seekg(primaryOffset);
        file.read(&value, sizeof(value));
        primaryOffset += sizeof(value);
        return std::string(1, value);
    }

    StringType::StringType() : Type("char*", 0) {}

    std::string StringType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        std::string result;
        file.seekg(primaryOffset);
        char c;
        while (file.read(&c, 1) && c != '\0') {
            result += c;
            primaryOffset++;
        }
        primaryOffset++;
        return "\"" + result + "\"";
    }

    VectorType::VectorType(const std::string& name, size_t dimensions)
        : Type(name, dimensions * sizeof(float)), dimensions_(dimensions) {
    }

    std::string VectorType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        size_t originalPos = file.tellg();
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        if (primaryOffset + (dimensions_ * sizeof(float)) > fileSize) {
            file.seekg(originalPos);
            return "(invalid offset)";
        }
        file.seekg(primaryOffset);
        std::vector<float> components(dimensions_);
        file.read(reinterpret_cast<char*>(components.data()), sizeof(float) * dimensions_);
        primaryOffset += sizeof(float) * dimensions_;
        std::stringstream ss;
        ss << "(";
        for (size_t i = 0; i < dimensions_; ++i) {
            ss << std::fixed << std::setprecision(5) << components[i];
            if (i < dimensions_ - 1) ss << ", ";
        }
        ss << ")";
        file.seekg(originalPos);

        return ss.str();
    }

    void VectorType::formatXml(pugi::xml_node& parent, const std::string& name, const std::string& value) const {
        pugi::xml_node vectorNode = parent.append_child(name.c_str());
        std::string cleanValue = value.substr(1, value.length() - 2);
        std::stringstream ss(cleanValue);
        std::string component;
        std::vector<std::string> components;

        while (std::getline(ss, component, ',')) {
            components.push_back(component);
        }
        std::string axisNames = "XYZ";
        for (size_t i = 0; i < std::min(dimensions_, components.size()); ++i) {
            vectorNode.append_child(std::string(1, axisNames[i]).c_str()).text().set(components[i].c_str());
        }
    }

    QuaternionType::QuaternionType() : Type("cSPQuaternion", 16) {}

    std::string QuaternionType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        float components[4];
        file.seekg(primaryOffset);
        file.read(reinterpret_cast<char*>(components), sizeof(components));
        primaryOffset += sizeof(components);

        std::stringstream ss;
        ss << "(" << std::fixed << std::setprecision(5)
            << components[0] << ", " << components[1] << ", "
            << components[2] << ", " << components[3] << ")";
        return ss.str();
    }

    void QuaternionType::formatXml(pugi::xml_node& parent, const std::string& name, const std::string& value) const {
        pugi::xml_node quatNode = parent.append_child(name.c_str());
        std::string cleanValue = value.substr(1, value.length() - 2);
        std::stringstream ss(cleanValue);
        std::string component;
        std::vector<std::string> components;

        while (std::getline(ss, component, ',')) {
            components.push_back(component);
        }
        std::vector<std::string> componentNames = { "W", "X", "Y", "Z" };
        for (size_t i = 0; i < std::min(size_t(4), components.size()); ++i) {
            quatNode.append_child(componentNames[i].c_str()).text().set(components[i].c_str());
        }
    }

    EnumType::EnumType() : Type("enum", 4) {}

    std::string EnumType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        uint32_t value = 0;
        file.seekg(primaryOffset);
        file.read(reinterpret_cast<char*>(&value), sizeof(value));
        primaryOffset += sizeof(value);
        return std::to_string(value);
    }
    SpecialType::SpecialType(const std::string& name, size_t size, TypePtr contentType)
        : Type(name, size), contentType_(contentType) {
    }

    void SpecialType::setContentType(TypePtr contentType) {
        contentType_ = contentType;
    }

    TypePtr SpecialType::getContentType() const {
        return contentType_;
    }

    std::string SpecialType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        return "special";
    }

    KeyType::KeyType() : SpecialType("key", 4) {}

    std::string KeyType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        size_t originalPos = file.tellg();
        file.seekg(primaryOffset);
        uint32_t indicator = 0;
        file.read(reinterpret_cast<char*>(&indicator), sizeof(indicator));
        primaryOffset += sizeof(indicator);
        if (indicator == 0) {
            file.seekg(originalPos);
            return "IGNORE";
        }

        file.seekg(secondaryOffset);

        std::string result;
        char c;
        while (file.read(&c, 1) && c != '\0') {
            result += c;
        }

        secondaryOffset += result.size() + 1;

        file.seekg(originalPos);

        return "\"" + result + "\"";
    }

    AssetType::AssetType() : SpecialType("asset", 4) {}

    std::string AssetType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        size_t originalPos = file.tellg();
        file.seekg(primaryOffset);
        uint32_t indicator = 0;
        file.read(reinterpret_cast<char*>(&indicator), sizeof(indicator));
        primaryOffset += sizeof(indicator);
        if (indicator == 0) {
            file.seekg(originalPos);
            return "IGNORE";
        }

        file.seekg(secondaryOffset);

        std::string result;
        char c;
        while (file.read(&c, 1) && c != '\0') {
            result += c;
        }
        
        // Atualizar o offset secundário: comprimento da string + 1 para o null terminator
        secondaryOffset += result.size() + 1;

        file.seekg(originalPos);

        return "\"" + result + "\"";
    }

    NullableType::NullableType() : SpecialType("nullable", 4) {}

    std::string NullableType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        size_t originalPos = file.tellg();
        file.seekg(primaryOffset);
        uint32_t indicator = 0;
        file.read(reinterpret_cast<char*>(&indicator), sizeof(indicator));
        primaryOffset += sizeof(indicator);

        if (indicator == 0 || !contentType_) {
            file.seekg(originalPos);
            return "IGNORE";
        }
        if (auto structType = std::dynamic_pointer_cast<StructType>(contentType_)) {
            file.seekg(originalPos);
            return "struct:" + structType->getName();
        }
        else {
            file.seekg(secondaryOffset);
            std::string result = contentType_->read(file, secondaryOffset, secondaryOffset);

            file.seekg(originalPos);
            return result;
        }
    }

    ArrayType::ArrayType() : SpecialType("array", 4) {}

    std::string ArrayType::read(std::ifstream& file, size_t& primaryOffset, size_t& secondaryOffset) const {
        uint32_t count = 0;
        file.seekg(primaryOffset);
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        primaryOffset += sizeof(count);
        return std::to_string(count);
    }
    Catalog::Catalog() {
        setupDefaultTypes();
    }

    void Catalog::setupDefaultTypes() {
        types_["bool"] = createBoolType();
        types_["char"] = createCharType();
        types_["int8_t"] = createIntType("int8_t", 1);
        types_["int16_t"] = createIntType("int16_t", 2);
        types_["int64_t"] = createIntType("int64_t", 8);
        types_["uint8_t"] = createIntType("uint8_t", 1);
        types_["uint16_t"] = createIntType("uint16_t", 2);
        types_["uint32_t"] = createIntType("uint32_t", 4);
        types_["uint64_t"] = std::make_shared<Uint64Type>();
        types_["float"] = createFloatType();
        types_["double"] = createDoubleType();
        types_["char*"] = createStringType();
        types_["asset"] = createAssetType();
        types_["nullable"] = createNullableType();
        types_["array"] = createArrayType();
        types_["enum"] = createEnumType();
        types_["cSPVector2"] = createVectorType("cSPVector2", 2);
        types_["cSPVector3"] = createVectorType("cSPVector3", 3);
        types_["cSPQuaternion"] = createQuaternionType();
    }

    TypePtr Catalog::add(const std::string& name, size_t size) {
        auto type = std::make_shared<PrimitiveType>(name, size);
        types_[name] = type;
        return type;
    }

    StructTypePtr Catalog::addStruct(const std::string& name, size_t fixedSize) {
        auto structType = std::make_shared<StructType>(name, fixedSize, this);
        types_["struct:" + name] = structType;
        return structType;
    }

    TypePtr Catalog::createBoolType() {
        return std::make_shared<BoolType>();
    }

    TypePtr Catalog::createCharType() {
        return std::make_shared<CharType>();
    }

    TypePtr Catalog::createIntType(const std::string& name, size_t size) {
        return std::make_shared<IntegerType>(name, size);
    }

    TypePtr Catalog::createFloatType() {
        return std::make_shared<FloatType>();
    }

    TypePtr Catalog::createDoubleType() {
        return std::make_shared<DoubleType>();
    }

    TypePtr Catalog::createStringType() {
        return std::make_shared<StringType>();
    }

    TypePtr Catalog::createVectorType(const std::string& name, size_t dimensions) {
        return std::make_shared<VectorType>(name, dimensions);
    }

    TypePtr Catalog::createQuaternionType() {
        return std::make_shared<QuaternionType>();
    }

    TypePtr Catalog::createKeyType() {
        return std::make_shared<KeyType>();
    }

    TypePtr Catalog::createAssetType() {
        return std::make_shared<AssetType>();
    }

    TypePtr Catalog::createNullableType() {
        return std::make_shared<NullableType>();
    }

    TypePtr Catalog::createArrayType() {
        return std::make_shared<ArrayType>();
    }

    TypePtr Catalog::createEnumType() {
        return std::make_shared<EnumType>();
    }

    void Catalog::registerFileType(const std::string& extension, const std::vector<std::string>& rootStructs, size_t secondaryOffset) {
        fileTypes_[extension] = std::make_shared<FileType>(extension, rootStructs, secondaryOffset);
    }

    TypePtr Catalog::getType(const std::string& name) const {
        auto it = types_.find(name);
        return (it != types_.end()) ? it->second : nullptr;
    }

    FileTypePtr Catalog::getFileType(const std::string& extension) const {
        auto it = fileTypes_.find(extension);
        return (it != fileTypes_.end()) ? it->second : nullptr;
    }

} // namespace recap
