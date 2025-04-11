#include "catalog.h"
#include <iostream>
#include <cstring>
#include <filesystem>


bool Parser::parse() {
    fileStream.open(filename, std::ios::binary);
    if (!fileStream) {
        return false;
    }
    std::string extension = std::filesystem::path(filename).extension().string();

    const FileTypeInfo* fileType = catalog.getFileType(extension);
    if (!fileType) {
        if (!extension.empty() && extension[0] == '.') {
            extension = extension.substr(1);
        }
        else {
            extension = "." + extension;
        }
        fileType = catalog.getFileType(extension);

        if (!fileType) {
            std::cerr << "Unsupported file type: " << std::filesystem::path(filename).extension().string() << std::endl;
            std::cerr << "Registered file types: ";
            for (const auto& type : { "Phase, .Noun" }) {
                std::cerr << "." << type << " ";
            }
            std::cerr << std::endl;
            return false;
        }
    }

    offsetManager.setPrimaryOffset(0);
    offsetManager.setSecondaryOffset(fileType->secondaryOffsetStart);

    if (xmlMode) {
        xmlDoc = pugi::xml_document();
        currentXmlNode = xmlDoc;
    }

    for (const auto& structType : fileType->structTypes) {
        parseStruct(structType);
    }

    fileStream.close();
    return true;
}

void Parser::parseStruct(const std::string& structName) {
    try {
        auto structDef = catalog.getStruct(structName);
        if (!structDef) {
            std::cerr << "Unknown struct: " << structName << std::endl;
            return;
        }

        logParse(fmt::format("parse_struct({})", structName));

        pugi::xml_node prevXmlNode;
        if (xmlMode) {
            prevXmlNode = currentXmlNode;
            currentXmlNode = currentXmlNode.append_child(structName.c_str());
        }

        size_t previousStructBaseOffset = currentStructBaseOffset;

        if (secOffsetStruct) {
            structBaseOffsetStack.push(previousStructBaseOffset);
            currentStructBaseOffset = offsetManager.getSecondaryOffset();

            offsetManager.setSecondaryOffset(offsetManager.getSecondaryOffset() + structDef->getFixedSize());
        }

        indentLevel++;

        for (const auto& member : structDef->getMembers()) {
            parseMember(member, structDef);
        }

        indentLevel--;
        if (secOffsetStruct) {
            if (!structBaseOffsetStack.empty()) {
                currentStructBaseOffset = structBaseOffsetStack.top();
                structBaseOffsetStack.pop();
            }
            else {
                currentStructBaseOffset = 0;
            }
        }
        else {
            currentStructBaseOffset = previousStructBaseOffset;
        }

        if (xmlMode) {
            currentXmlNode = prevXmlNode;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in parse_struct(" << structName << "): "
            << e.what() << " at position ("
            << offsetManager.getPrimaryOffset() << ", "
            << offsetManager.getSecondaryOffset() << ")" << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown error in parse_struct(" << structName << ") at position ("
            << offsetManager.getPrimaryOffset() << ", "
            << offsetManager.getSecondaryOffset() << ")" << std::endl;
    }
}

void Parser::parseMember(const StructMember& member, const std::shared_ptr<StructDefinition>& parentStruct) {
    const TypeDefinition* typeDef = catalog.getType(member.typeName);
    if (!typeDef) {
        std::cerr << "Unknown type: " << member.typeName << std::endl;
        return;
    }

    if (member.typeName == "array") {
        size_t originalOffset = offsetManager.getPrimaryOffset();
        offsetManager.setPrimaryOffset(currentStructBaseOffset + member.offset);
        uint32_t hasValue = offsetManager.readPrimary<uint32_t>();
        if (hasValue != 0) {
            offsetManager.advancePrimary(sizeof(uint32_t));
            uint32_t count = offsetManager.readPrimary<uint32_t>();

            logParse(fmt::format("parse_member_array({}, {})", member.name, count));
            indentLevel++;

            pugi::xml_node arrayNode;
            pugi::xml_node originalNode = currentXmlNode;
            if (xmlMode) {
                arrayNode = currentXmlNode.append_child(member.name.c_str());
            }

            for (size_t i = 0; i < count; i++) {
                size_t elementSize = 0;
                const TypeDefinition* typeDef = catalog.getType(member.elementType);
                if (typeDef) {
                    elementSize = typeDef->size;
                }

                StructMember elementMember("entry", member.elementType, offsetManager.getSecondaryOffset(), true);

                if (xmlMode) {
                    currentXmlNode = arrayNode.append_child("entry");
                }
                parseMember(elementMember, parentStruct);
                offsetManager.advanceSecondary(elementSize);

                if (xmlMode) {
                    currentXmlNode = originalNode;
                }
            }

            indentLevel--;
        }

        offsetManager.setPrimaryOffset(originalOffset);
        return;
    }

    if (secOffsetStruct) {
        offsetManager.setPrimaryOffset(currentStructBaseOffset + member.offset);
    }
    else if (member.useSecondaryOffset) {
        offsetManager.setPrimaryOffset(member.offset);
    }
    else {
        offsetManager.setPrimaryOffset(currentStructBaseOffset + member.offset);
    }
    bool isSpecialType = (typeDef->type == DataType::KEY ||
        typeDef->type == DataType::ASSET ||
        typeDef->type == DataType::NULLABLE ||
        typeDef->type == DataType::CHAR_PTR);

    std::string valueStr;
    std::string logMessage;

    switch (typeDef->type) {
    case DataType::BOOL: {
        bool value;
        if (secOffsetStruct) {
            value = offsetManager.readPrimary<bool>();
        }
        else {
            value = offsetManager.readPrimary<bool>();
        }
        valueStr = value ? "true" : "false";
        logMessage = fmt::format("parse_member_bool({}, {})", member.name, valueStr);
        break;
    }
    case DataType::INT: {
        int value;
        if (secOffsetStruct) {
            value = offsetManager.readPrimary<int>();
        }
        else {
            value = offsetManager.readPrimary<int>();
        }
        valueStr = std::to_string(value);
        logMessage = fmt::format("parse_member_int({}, {})", member.name, valueStr);
        break;
    }
    case DataType::FLOAT: {
        float value;
        if (secOffsetStruct) {
            value = offsetManager.readPrimary<float>();
        }
        else {
            value = offsetManager.readPrimary<float>();
        }
        valueStr = fmt::format("{:.5f}", value);
        logMessage = fmt::format("parse_member_float({}, {})", member.name, valueStr);
        break;
    }
    case DataType::VECTOR2: {
        float x;
        float y;
        if (secOffsetStruct) {
            x = offsetManager.readPrimary<float>();
            y = offsetManager.readPrimary<float>();
        }
        else {
            x = offsetManager.readPrimary<float>();
            y = offsetManager.readPrimary<float>();
        }
        valueStr = fmt::format("x: {:.5f}, y: {:.5f}", x, y);
        logMessage = fmt::format("parse_member_vector2({}, {})", member.name, valueStr);

        if (xmlMode) {
            auto vecNode = currentXmlNode.append_child(member.name.c_str());
            vecNode.append_child("x").text().set(fmt::format("{:.5f}", x).c_str());
            vecNode.append_child("y").text().set(fmt::format("{:.5f}", y).c_str());
        }
        break;
    }
    case DataType::VECTOR3: {
        float x;
        float y;
        float z;
        if (secOffsetStruct) {
            x = offsetManager.readPrimary<float>();
            y = offsetManager.readPrimary<float>();
            z = offsetManager.readPrimary<float>();
        }
        else {
            x = offsetManager.readPrimary<float>();
            y = offsetManager.readPrimary<float>();
            z = offsetManager.readPrimary<float>();
        }
        valueStr = fmt::format("x: {:.f}, y: {:.5f}, z: {:.5f}", x, y, z);
        logMessage = fmt::format("parse_member_vec3({}, {})", member.name, valueStr);

        if (xmlMode) {
            auto vecNode = currentXmlNode.append_child(member.name.c_str());
            vecNode.append_child("x").text().set(fmt::format("{:.5f}", x).c_str());
            vecNode.append_child("y").text().set(fmt::format("{:.5f}", y).c_str());
            vecNode.append_child("z").text().set(fmt::format("{:.5f}", z).c_str());
        }
        break;
    }
    case DataType::QUATERNION: {
        float w;
        float x;
        float y;
        float z;
        if (secOffsetStruct) {
            w = offsetManager.readPrimary<float>();
            x = offsetManager.readPrimary<float>();
            y = offsetManager.readPrimary<float>();
            z = offsetManager.readPrimary<float>();
        }
        else {
            w = offsetManager.readPrimary<float>();
            x = offsetManager.readPrimary<float>();
            y = offsetManager.readPrimary<float>();
            z = offsetManager.readPrimary<float>();
        }
        valueStr = fmt::format("w: {:.5f}, x: {:.5f}, y: {:.5f}, z: {:.5f}", w, x, y, z);
        logMessage = fmt::format("parse_member_quaternion({}, {})", member.name, valueStr);

        if (xmlMode) {
            auto vecNode = currentXmlNode.append_child(member.name.c_str());
            vecNode.append_child("w").text().set(fmt::format("{:.5f}", w).c_str());
            vecNode.append_child("x").text().set(fmt::format("{:.5f}", x).c_str());
            vecNode.append_child("y").text().set(fmt::format("{:.5f}", y).c_str());
            vecNode.append_child("z").text().set(fmt::format("{:.5f}", z).c_str());
        }
        break;
    }
    case DataType::KEY: {
        uint32_t offset = offsetManager.readPrimary<uint32_t>();
        if (offset > 0) {
            size_t currentSecondary = offsetManager.getSecondaryOffset();
            std::string key = offsetManager.readString(true);
            valueStr = key;
            logMessage = fmt::format("parse_member_key({}, {})", member.name, valueStr);
        }
        else {
            return;
            //valueStr = "null";
            //logMessage = fmt::format("parse_member_key({}, {})", member.name, valueStr);
        }
        break;
    }
    case DataType::ASSET: {
        uint32_t offset = offsetManager.readPrimary<uint32_t>();
        if (offset > 0) {
            size_t currentSecondary = offsetManager.getSecondaryOffset();
            std::string asset = offsetManager.readString(true);
            valueStr = asset;
            logMessage = fmt::format("parse_member_asset({}, {})", member.name, valueStr);
        }
        else {
            return;
            //valueStr = "null";
            //logMessage = fmt::format("parse_member_asset({}, {})", member.name, valueStr);
        }
        break;
    }
    case DataType::CHAR_PTR: {
        uint32_t offset = offsetManager.readPrimary<uint32_t>();
        if (offset > 0) {
            size_t currentSecondary = offsetManager.getSecondaryOffset();
            std::string char_ptr = offsetManager.readString(true);
            valueStr = char_ptr;
            logMessage = fmt::format("parse_member_char*({}, {})", member.name, valueStr);
        }
        else {
            return;
            //valueStr = "null";
            //logMessage = fmt::format("parse_member_asset({}, {})", member.name, valueStr);
        }
        break;
    }
    case DataType::CHAR: {
        std::string string = offsetManager.readString(true);
        valueStr = string;
        logMessage = fmt::format("parse_member_asset({}, {})", member.name, valueStr);
        break;
    }
    case DataType::ENUM: {
        uint32_t value;
        if (secOffsetStruct) {
            value = offsetManager.readPrimary<uint32_t>();
        }
        else {
            value = offsetManager.readPrimary<uint32_t>();
        }
        valueStr = std::to_string(value);
        logMessage = fmt::format("parse_member_enum({}, {})", member.name, valueStr);
        break;
    }
    case DataType::UINT8: {
        uint8_t value;
        if (secOffsetStruct) {
            value = offsetManager.readPrimary<uint8_t>();
        }
        else {
            value = offsetManager.readPrimary<uint8_t>();
        }
        valueStr = std::to_string(value);
        logMessage = fmt::format("parse_member_uint8_t({}, {})", member.name, valueStr);
        break;
    }
    case DataType::UINT16: {
        uint16_t value;
        if (secOffsetStruct) {
            value = offsetManager.readPrimary<uint16_t>();
        }
        else {
            value = offsetManager.readPrimary<uint16_t>();
        }
        valueStr = std::to_string(value);
        logMessage = fmt::format("parse_member_uint16_t({}, {})", member.name, valueStr);
        break;
    }
    case DataType::UINT32: {
        uint32_t value;
        if (secOffsetStruct) {
            value = offsetManager.readPrimary<uint32_t>();
        }
        else {
            value = offsetManager.readPrimary<uint32_t>();
        }
        valueStr = std::to_string(value);
        logMessage = fmt::format("parse_member_uint32_t({}, {})", member.name, valueStr);
        break;
    }
    case DataType::UINT64: {
        uint64_t value;
        if (secOffsetStruct) {
            value = offsetManager.readPrimary<uint64_t>();
        }
        else {
            value = offsetManager.readPrimary<uint64_t>();
        }
        valueStr = fmt::format("0x{:X}", value);
        logMessage = fmt::format("parse_member_uint64_t({}, {})", member.name, valueStr);
        break;
    }
    case DataType::NULLABLE: {
        uint32_t hasValue = offsetManager.readPrimary<uint32_t>();
        if (hasValue > 0 && !typeDef->targetType.empty()) {
            auto targetStruct = catalog.getStruct(typeDef->targetType);
            if (targetStruct) {
                bool previousSecOffsetStruct = secOffsetStruct;
                size_t previousStructBaseOffset = currentStructBaseOffset;
                secOffsetStruct = true;
                currentStructBaseOffset = offsetManager.getPrimaryOffset() - sizeof(uint32_t);
                parseStruct(typeDef->targetType);
                secOffsetStruct = previousSecOffsetStruct;
                currentStructBaseOffset = previousStructBaseOffset;

                if (typeDef->type == DataType::NULLABLE) {
                    offsetManager.setSecondaryOffset(offsetManager.getSecondaryOffset()); // + targetStruct->getFixedSize()
                }
                return;
            }
        }
        else {
            return;
            //valueStr = hasValue > 0 ? "true" : "false";
            //logMessage = fmt::format("parse_member_nullable({}, {})", member.name, valueStr);
        }
        break;
    }
    case DataType::STRUCT: {
        //logMessage = fmt::format("parse_member_struct({}, {})", member.name, typeDef->targetType);
        //logParse(logMessage);
        //indentLevel++;
        size_t currentOffset = offsetManager.getPrimaryOffset();
        size_t previousBaseOffset = currentStructBaseOffset;
        currentStructBaseOffset = currentOffset;

        parseStruct(typeDef->targetType);

        currentStructBaseOffset = previousBaseOffset;
        return;
    }
    default: {
        valueStr = "unknown";
        logMessage = fmt::format("parse_member_unknown({}, {})", member.name, valueStr);
        break;
    }
    }

    logParse(logMessage);

    if (xmlMode && typeDef->type != DataType::VECTOR2 &&
        typeDef->type != DataType::VECTOR3 && typeDef->type != DataType::QUATERNION) {
        currentXmlNode.append_child(member.name.c_str()).text().set(valueStr.c_str());
    }
}

void Parser::exportXml(const std::string& outputFile) {
    if (!xmlMode) return;

    xmlDoc.save_file(outputFile.c_str());
}