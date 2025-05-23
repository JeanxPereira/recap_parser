﻿#include "catalog.h"
#include "exporter.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cctype>

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

bool Parser::parse() {
    fileStream.open(filename, std::ios::binary);
    if (!fileStream) {
        return false;
    }

    std::string extension = fs::path(filename).extension().string();
    const FileTypeInfo* fileType = catalog.getFileType(extension);

    if (!fileType) {
        fileType = catalog.getFileTypeByName(filename);
    }

    if (!fileType) {
        return false;
    }

    offsetManager.setPrimaryOffset(0);
    offsetManager.setSecondaryOffset(fileType->secondaryOffsetStart);

    if (exportMode && exporter) {
        exporter->beginDocument();
    }

    for (const auto& structType : fileType->structTypes) {
        isProcessingRootTag = true;
        parseStruct(structType);
        isProcessingRootTag = false;
    }

    if (exportMode && exporter) {
        exporter->endDocument();
    }

    fileStream.close();
    return true;
}

void Parser::parseStruct(const std::string& structName, int arrayIndex) {
    try {
        auto structDef = catalog.getStruct(structName);
        if (!structDef) {
            std::cerr << "Unknown struct: " << structName << std::endl;
            return;
        }

        if (arrayIndex >= 0) {
            logParse(fmt::format("parse_struct({}, [{}])", structName, arrayIndex));
        }
        else {
            logParse(fmt::format("parse_struct({})", structName));
        }

        bool shouldEndNode = false;
        if (exportMode && exporter) {
            if (isProcessingRootTag) {
                std::string lowerStructName = structName;
                std::transform(lowerStructName.begin(), lowerStructName.end(),
                    lowerStructName.begin(), [](unsigned char c) { return std::tolower(c); });
                exporter->beginNode(lowerStructName);
                isProcessingRootTag = false;
                shouldEndNode = true;
            }
            else if (arrayIndex < 0 && !isInsideNullable) {
                exporter->beginNode(structName);
                shouldEndNode = true;
            }
        }

        size_t previousStructBaseOffset = currentStructBaseOffset;
        size_t structBaseOffset = offsetManager.getPrimaryOffset();

        if (secOffsetStruct) {
            structBaseOffsetStack.push(previousStructBaseOffset);
            currentStructBaseOffset = offsetManager.getSecondaryOffset();

            if (!processingArrayElement) {
                offsetManager.setSecondaryOffset(offsetManager.getSecondaryOffset() + structDef->getFixedSize());
            }
        }

        indentLevel++;

        size_t structStartOffset = offsetManager.getPrimaryOffset();
        for (const auto& member : structDef->getMembers()) {
            if (processingArrayElement) {
                offsetManager.setPrimaryOffset(structStartOffset);
            }
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

        if (exportMode && exporter && shouldEndNode) {
            exporter->endNode();
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

void Parser::parseMember(const StructMember& member, const std::shared_ptr<StructDefinition>& parentStruct, size_t arraySize) {
    const TypeDefinition* typeDef = catalog.getType(member.typeName);
    if (!typeDef) {
        std::cerr << "Unknown type: " << member.typeName << std::endl;
        return;
    }
    size_t originalSecondaryOffset = offsetManager.getRealSecondaryOffset();
    size_t arrayStructOffset = offsetManager.getPrimaryOffset();
    if (member.typeName == "array") {
        size_t structStartOffset = currentStructBaseOffset;

        size_t arrayStartOffset;
        if (secOffsetStruct) {
            if (processingArrayElement) {
                arrayStartOffset = arrayStructOffset + member.offset;
            }
            else if (isInsideNullable) {
                arrayStartOffset = startNullableOffset + member.offset;
            }
            else {
                arrayStartOffset = structStartOffset + member.offset;
            }
        }
        else if (member.useSecondaryOffset) {
            arrayStartOffset = member.offset;
        }
        else {
            arrayStartOffset = currentStructBaseOffset + member.offset;
        }

        offsetManager.setPrimaryOffset(arrayStartOffset);
        uint32_t hasValue = offsetManager.readPrimary<uint32_t>();

        bool useSecondaryForElements = secOffsetStruct || (!secOffsetStruct && !processingArrayElement);
        size_t arrayDataOffset = useSecondaryForElements ?
            offsetManager.getSecondaryOffset() :
            offsetManager.getPrimaryOffset();

        if (hasValue != 0) {
            uint32_t count;
            if (member.countOffset > 0) {
                size_t countOffset = startNullableOffset + member.offset + member.countOffset;
                count = offsetManager.readAt<uint32_t>(countOffset);
            }
            else {
                count = offsetManager.readPrimary<uint32_t>();
            }

            const TypeDefinition* typeDef = catalog.getType(member.elementType);
            auto structDef = catalog.getStruct(member.elementType);
            logParse(fmt::format("parse_member_array({}, {})", member.name, count));
            indentLevel++;

            if (exportMode && exporter) {
                exporter->beginArray(member.name);
            }

            if (structDef) {
                size_t elementSize = structDef->getFixedSize();
                size_t elementBaseOffset = offsetManager.getPrimaryOffset();

                totalArraySize = structDef->getFixedSize() * count;
                offsetManager.setSecondaryOffset(originalSecondaryOffset + totalArraySize);

                for (uint32_t i = 0; i < count; i++) {
                    if (exportMode && exporter) {
                        exporter->beginArrayEntry();
                    }

                    if (useSecondaryForElements) {
                        offsetManager.setPrimaryOffset(arrayDataOffset);
                        bool oldSecOffsetStruct = secOffsetStruct;
                        secOffsetStruct = true;
                        processingArrayElement = true;
                        parseStruct(member.elementType, i);

                        secOffsetStruct = oldSecOffsetStruct;
                        arrayDataOffset += structDef->getFixedSize();
                    }
                    else {
                        offsetManager.setPrimaryOffset(elementBaseOffset);
                        processingArrayElement = true;
                        parseStruct(member.elementType, i);
                        elementBaseOffset += elementSize;
                    }

                    if (exportMode && exporter) {
                        exporter->endArrayEntry();
                    }
                }
            }
            else {
                bool useSecondaryForElements = !secOffsetStruct && !processingArrayElement;
                size_t elementBaseOffset = useSecondaryForElements ?
                    offsetManager.getSecondaryOffset() :
                    offsetManager.getPrimaryOffset();

                size_t totalElementSize = count * (typeDef ? typeDef->size : 0);
                if (useSecondaryForElements) {
                    offsetManager.setSecondaryOffset(originalSecondaryOffset + totalElementSize);
                }

                for (size_t i = 0; i < count; i++) {
                    size_t elementSize = 0;
                    if (typeDef) {
                        elementSize = typeDef->size;

                        if (useSecondaryForElements) {
                            offsetManager.setPrimaryOffset(elementBaseOffset);
                            elementBaseOffset += elementSize;
                        }

                        StructMember elementMember("entry", member.elementType, offsetManager.getPrimaryOffset(), useSecondaryForElements);

                        if (exportMode && exporter) {
                            exporter->beginArrayEntry();
                        }

                        parseMember(elementMember, parentStruct);

                        if (exportMode && exporter) {
                            exporter->endArrayEntry();
                        }

                        if (!useSecondaryForElements) {
                            offsetManager.advancePrimary(elementSize);
                        }
                    }
                }
            }

            if (exportMode && exporter) {
                exporter->endArray();
            }

            processingArrayElement = false;
            indentLevel--;
        }

        return;
    }

    if (secOffsetStruct) {
        if (processingArrayElement) {
            offsetManager.setPrimaryOffset(arrayStructOffset + member.offset);
        }
        else {
            offsetManager.setPrimaryOffset(currentStructBaseOffset + member.offset);
        }
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

        if (exportMode && exporter) {
            exporter->exportBool(member.name, value);
        }
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

        if (exportMode && exporter) {
            exporter->exportInt(member.name, value);
        }
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

        if (exportMode && exporter) {
            exporter->exportFloat(member.name, value);
        }
        break;
    }
    case DataType::GUID: {
        uint32_t data1;
        uint16_t data2;
        uint16_t data3;
        uint64_t data4;

        if (secOffsetStruct) {
            data1 = offsetManager.readPrimary<uint32_t>();
            data2 = offsetManager.readPrimary<uint16_t>();
            data3 = offsetManager.readPrimary<uint16_t>();
            data4 = offsetManager.readPrimary<uint64_t>();
        }
        else {
            data1 = offsetManager.readPrimary<uint32_t>();
            data2 = offsetManager.readPrimary<uint16_t>();
            data3 = offsetManager.readPrimary<uint16_t>();
            data4 = offsetManager.readPrimary<uint64_t>();
        }

        valueStr = fmt::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
            data1,
            data2,
            data3,
            (data4 >> 48) & 0xFFFF,
            data4 & 0xFFFFFFFFFFFFULL
        );

        logMessage = fmt::format("parse_member_guid({}, {})", member.name, valueStr);

        if (exportMode && exporter) {
            exporter->exportGuid(member.name, valueStr);
        }
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
        logMessage = fmt::format("parse_member_cSPVector2({}, {})", member.name, valueStr);

        if (exportMode && exporter) {
            exporter->exportVector2(member.name, x, y);
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
        valueStr = fmt::format("x: {:.5f}, y: {:.5f}, z: {:.5f}", x, y, z);
        logMessage = fmt::format("parse_member_cSPVector3({}, {})", member.name, valueStr);

        if (exportMode && exporter) {
            exporter->exportVector3(member.name, x, y, z);
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
        logMessage = fmt::format("parse_member_cSPVector4({}, {})", member.name, valueStr);

        if (exportMode && exporter) {
            exporter->exportQuaternion(member.name, w, x, y, z);
        }
        break;
    }
    case DataType::KEY: {
        uint32_t offset = offsetManager.readPrimary<uint32_t>();
        if (offset != 0) {
            size_t currentSecondary = offsetManager.getSecondaryOffset();
            std::string key = offsetManager.readString(true);
            valueStr = key;
            logMessage = fmt::format("parse_member_key({}, {})", member.name, valueStr);

            if (exportMode && exporter) {
                exporter->exportString(member.name, valueStr);
            }
        }
        else {
            return;
        }
        break;
    }
    case DataType::CKEYASSET: {
        uint32_t offset = offsetManager.readPrimary<uint32_t>();
        if (offset != 0) {
            size_t currentSecondary = offsetManager.getSecondaryOffset();
            std::string key = offsetManager.readString(true);
            valueStr = key;
            logMessage = fmt::format("parse_member_cKeyAsset({}, {})", member.name, valueStr);

            if (exportMode && exporter) {
                exporter->exportString(member.name, valueStr);
            }
        }
        else {
            return;
        }
        break;
    }
    case DataType::LOCALIZEDASSETSTRING: {
        uint32_t offset = offsetManager.readPrimary<uint32_t>();
        uint32_t assetString = offsetManager.readPrimary<uint32_t>();
        if (offset != 0) {
            size_t currentSecondary = offsetManager.getSecondaryOffset();
            std::string str = offsetManager.readString(true);
            if (assetString != 0) {
                std::string id = offsetManager.readString(true);
                logMessage = fmt::format("parse_member_cLocalizedAssetString({}, {}, {})", member.name, str, id);

                if (exportMode && exporter) {
                    exporter->beginNode(member.name);
                    exporter->exportString("text", str);
                    exporter->exportString("id", id);
                    exporter->endNode();
                }
            }
            else {
                logMessage = fmt::format("parse_member_cLocalizedAssetString({}, {})", member.name, str);

                if (exportMode && exporter) {
                    exporter->exportString(member.name, str);
                }
            }
        }
        else {
            return;
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

            if (exportMode && exporter) {
                exporter->exportString(member.name, valueStr);
            }
        }
        else {
            return;
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

            if (exportMode && exporter) {
                exporter->exportString(member.name, valueStr);
            }
        }
        else {
            return;
        }
        break;
    }
    case DataType::CHAR: {
        std::string string = offsetManager.readString();
        valueStr = string;
        if (!string.empty() && string != "0") {
            logMessage = fmt::format("parse_member_char({}, {})", member.name, valueStr);

            if (exportMode && exporter) {
                exporter->exportString(member.name, valueStr);
            }
        }
        else {
            return;
        }
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

        if (exportMode && exporter) {
            exporter->exportUInt32(member.name, value);
        }
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

        if (exportMode && exporter) {
            exporter->exportUInt8(member.name, value);
        }
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

        if (exportMode && exporter) {
            exporter->exportUInt16(member.name, value);
        }
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

        if (exportMode && exporter) {
            exporter->exportUInt32(member.name, value);
        }
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

        if (exportMode && exporter) {
            exporter->exportUInt64(member.name, value);
        }
        break;
    }
    case DataType::INT64: {
        int64_t value;
        if (secOffsetStruct) {
            value = offsetManager.readPrimary<int64_t>();
        }
        else {
            value = offsetManager.readPrimary<int64_t>();
        }
        valueStr = fmt::format("0x{:X}", value);
        logMessage = fmt::format("parse_member_int64_t({}, {})", member.name, valueStr);

        if (exportMode && exporter) {
            exporter->exportInt64(member.name, value);
        }
        break;
    }
    case DataType::NULLABLE: {
        size_t startOffset = offsetManager.getPrimaryOffset();

        uint32_t hasValue = offsetManager.readPrimary<uint32_t>();
        if (hasValue > 0 && !typeDef->targetType.empty()) {
            auto targetStruct = catalog.getStruct(typeDef->targetType);
            if (targetStruct) {
                if (member.hasCustomName) {
                    logParse(fmt::format("parse_member_nullable({}, {})", member.name, typeDef->targetType));
                }
                else {
                    logParse(fmt::format("parse_member_nullable({})", typeDef->targetType));
                }

                startNullableOffset = offsetManager.getRealSecondaryOffset();

                bool oldSecOffsetStruct = secOffsetStruct;
                size_t oldStructBaseOffset = currentStructBaseOffset;
                bool oldProcessingArrayElement = processingArrayElement;

                secOffsetStruct = true;
                processingArrayElement = true;
                isInsideNullable = true;

                offsetManager.setPrimaryOffset(offsetManager.getSecondaryOffset());
                offsetManager.setSecondaryOffset(originalSecondaryOffset + targetStruct->getFixedSize());

                if (exportMode && exporter) {
                    exporter->beginNode(member.name);
                    parseStruct(typeDef->targetType);
                    exporter->endNode();
                }
                else {
                    parseStruct(typeDef->targetType);
                }

                processingArrayElement = oldProcessingArrayElement;
                secOffsetStruct = oldSecOffsetStruct;
                currentStructBaseOffset = oldStructBaseOffset;
                isInsideNullable = false;

                offsetManager.setPrimaryOffset(startOffset + 4);

                return;
            }
        }
        else {
            offsetManager.setPrimaryOffset(startOffset + 4);
            return;
        }
        break;
    }
    case DataType::STRUCT: {
        if (member.hasCustomName) {
            logMessage = fmt::format("parse_member_struct({}, {})", member.name, typeDef->targetType);
        }
        else {
            logMessage = fmt::format("parse_member_struct({})", typeDef->targetType);
        }
        logParse(logMessage);
        size_t currentOffset = offsetManager.getPrimaryOffset();
        size_t previousBaseOffset = currentStructBaseOffset;
        currentStructBaseOffset = currentOffset;

        if (exportMode && exporter && member.hasCustomName) {
            exporter->beginNode(member.name);
            parseStruct(typeDef->targetType);
            exporter->endNode();
        }
        else {
            parseStruct(typeDef->targetType);
        }

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
}

void Parser::exportToFile(const std::string& outputFile) {
    if (exportMode && exporter) {
        exporter->saveToFile(outputFile);
    }
}