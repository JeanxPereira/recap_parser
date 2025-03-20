#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <sstream>
#include "catalog.h"
#include <pugixml.hpp>
#include <stack>

namespace fs = std::filesystem;

class Logger {
public:
    Logger() : indentLevel(0), showPointers(false) {}

    void enablePointerDisplay() { showPointers = true; }
    void enableXmlMode() { xmlMode = true; }
    void increaseIndent() { indentLevel++; }
    void decreaseIndent() { indentLevel--; }

    void initXmlDocument() {
        xmlDoc.reset();
        pugi::xml_node declaration = xmlDoc.append_child(pugi::node_declaration);
        declaration.append_attribute("version") = "1.0";
        declaration.append_attribute("encoding") = "utf-8";
        currentNode = xmlDoc;
    }

    bool saveXmlDocument(const std::string& filename) {
        return xmlDoc.save_file(filename.c_str());
    }

    std::string getXmlString() {
        std::stringstream ss;
        xmlDoc.save(ss);
        return ss.str();
    }
    void logVec3(const std::string& fieldName, const std::string& vecString,
        size_t mainPtr = 0, size_t secPtr = 0) {
        if (xmlMode) {
            pugi::xml_node member = currentNode.append_child(fieldName.c_str());

            // Parse the vector string in format {x, y, z}
            std::string vecStr = vecString;
            vecStr.erase(std::remove(vecStr.begin(), vecStr.end(), '{'), vecStr.end());
            vecStr.erase(std::remove(vecStr.begin(), vecStr.end(), '}'), vecStr.end());

            std::stringstream ss(vecStr);
            std::string xVal, yVal, zVal;
            std::getline(ss, xVal, ',');
            std::getline(ss, yVal, ',');
            std::getline(ss, zVal, ',');

            // Trim whitespace
            xVal.erase(0, xVal.find_first_not_of(" \t"));
            yVal.erase(0, yVal.find_first_not_of(" \t"));
            zVal.erase(0, zVal.find_first_not_of(" \t"));

            // Create child elements for each component
            pugi::xml_node xNode = member.append_child("X");
            xNode.text().set(xVal.c_str());

            pugi::xml_node yNode = member.append_child("Y");
            yNode.text().set(yVal.c_str());

            pugi::xml_node zNode = member.append_child("Z");
            zNode.text().set(zVal.c_str());
        }
        else {
            indent();
            if (showPointers) {
                std::cout << "(" << mainPtr << ", " << secPtr << ") ";
            }
            std::cout << "parse_member_vec3(" << fieldName << ", " << vecString << ")" << std::endl;
        }
    }
    void logVec2(const std::string& fieldName, const std::string& vecString,
        size_t mainPtr = 0, size_t secPtr = 0) {
        if (xmlMode) {
            pugi::xml_node member = currentNode.append_child(fieldName.c_str());

            // Parse the vector string in format {x, y, z}
            std::string vecStr = vecString;
            vecStr.erase(std::remove(vecStr.begin(), vecStr.end(), '{'), vecStr.end());
            vecStr.erase(std::remove(vecStr.begin(), vecStr.end(), '}'), vecStr.end());

            std::stringstream ss(vecStr);
            std::string xVal, yVal;
            std::getline(ss, xVal, ',');
            std::getline(ss, yVal, ',');

            // Trim whitespace
            xVal.erase(0, xVal.find_first_not_of(" \t"));
            yVal.erase(0, yVal.find_first_not_of(" \t"));

            // Create child elements for each component
            pugi::xml_node xNode = member.append_child("X");
            xNode.text().set(xVal.c_str());

            pugi::xml_node yNode = member.append_child("Y");
            yNode.text().set(yVal.c_str());
        }
        else {
            indent();
            if (showPointers) {
                std::cout << "(" << mainPtr << ", " << secPtr << ") ";
            }
            std::cout << "parse_member_vec3(" << fieldName << ", " << vecString << ")" << std::endl;
        }
    }

    template<typename... Args>
    void logStruct(const std::string& structName, size_t mainPtr = 0, size_t secPtr = 0) {
        if (xmlMode) {
            std::string cleanName = structName;
            size_t parenPos = cleanName.find(" (");
            if (parenPos != std::string::npos) {
                cleanName = cleanName.substr(0, parenPos);
            }

            if (xmlNodeStack.empty() && cleanName == "Noun") {
                cleanName = "noun";
            }

            if (!xmlNodeStack.empty()) {
                currentNode = currentNode.append_child(cleanName.c_str());
            }
            else {
                currentNode = xmlDoc.append_child(cleanName.c_str());
            }
            xmlNodeStack.push(currentNode);
        }
        else {
            indent();
            if (showPointers) {
                std::cout << "(" << mainPtr << ", " << secPtr << ") ";
            }
            std::cout << "parse_struct(" << structName << ")" << std::endl;
        }
    }

    template<typename T>
    void logMember(const std::string& typeName, const std::string& fieldName, const T& value,
        size_t mainPtr = 0, size_t secPtr = 0) {
        if (xmlMode) {
            pugi::xml_node member = currentNode.append_child(fieldName.c_str());
            member.text().set(toString(value).c_str());
        }
        else {
            indent();
            if (showPointers) {
                std::cout << "(" << mainPtr << ", " << secPtr << ") ";
            }
            std::cout << "parse_member_" << typeName << "(" << fieldName << ", " << toString(value) << ")" << std::endl;
        }
    }

    template<typename T, typename U>
    void logMemberWithSecondary(const std::string& typeName, const std::string& fieldName,
        const T& mainValue, const U& secondaryValue,
        size_t mainPtr = 0, size_t secPtr = 0) {
        if (xmlMode) {
            pugi::xml_node member = currentNode.append_child(fieldName.c_str());
            member.text().set(toString(secondaryValue).c_str());
        }
        else {
            indent();
            if (showPointers) {
                std::cout << "(" << mainPtr << ", " << secPtr << ") ";
            }
            std::cout << "parse_member_" << typeName << "(" << fieldName << ", "
                << toString(mainValue) << ", " << toString(secondaryValue) << ")" << std::endl;
        }
    }

    void printLineBreak() {
        if (!xmlMode) {
            std::cout << std::endl;
        }
    }

    void finishStructNode() {
        if (xmlMode && !xmlNodeStack.empty()) {
            xmlNodeStack.pop();
            if (!xmlNodeStack.empty()) {
                currentNode = xmlNodeStack.top();
            }
            else {
                currentNode = xmlDoc;
            }
        }
    }

    template<typename T>
    std::string toString(const T& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }

private:
    int indentLevel;
    bool showPointers;
    bool xmlMode;
    pugi::xml_document xmlDoc;
    pugi::xml_node currentNode;
    std::stack<pugi::xml_node> xmlNodeStack;

    void indent() {
        for (int i = 0; i < indentLevel; i++) {
            std::cout << "    ";
        }
    }
};

template<>
std::string Logger::toString(const bool& value) {
    return value ? "true" : "false";
}

class Parser {
public:
    Parser(const std::string& filePath, bool showPointers = false, bool xmlMode = false, const std::string& xmlOutputPath = "")
        : filePath(filePath), xmlMode(xmlMode), xmlOutputPath(xmlOutputPath) {
        fs::path path(filePath);
        fileExtension = path.extension().string();

        if (showPointers) {
            logger.enablePointerDisplay();
        }

        if (xmlMode) {
            logger.enableXmlMode();
            logger.initXmlDocument();
        }

        file.open(filePath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }

        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        fileData.resize(fileSize);
        file.read(reinterpret_cast<char*>(fileData.data()), fileSize);

        mainPointer = 0;

        auto& catalog = parser::Catalog::getInstance();
        const auto& config = catalog.getFileTypeConfig(fileExtension);

        secondaryPointer = config.secondaryPointerOffset;
    }

    ~Parser() {
        if (file.is_open()) {
            file.close();
        }

        if (xmlMode && !xmlOutputPath.empty()) {
            fs::path outputPath(xmlOutputPath);
            if (fs::is_directory(outputPath)) {
                fs::path inputPath(filePath);
                std::string filename = inputPath.filename().string() + ".xml";
                outputPath /= filename;
            }
            logger.saveXmlDocument(outputPath.string());
        }
    }

    void parse() {
        auto& catalog = parser::Catalog::getInstance();
        const auto& config = catalog.getFileTypeConfig(fileExtension);

        if (config.rootStructs.empty()) {
            std::cout << "Unknown file type: " << fileExtension << std::endl;
            return;
        }

        if (config.hasRepeatingStructs) {
            auto headerDef = catalog.getStruct(config.headerStruct);
            if (headerDef) {
                parseStruct(headerDef);
            }

            uint32_t entryCount = 0;
            if (config.entryCountOffset + sizeof(uint32_t) <= fileSize) {
                memcpy(&entryCount, fileData.data() + config.entryCountOffset, sizeof(uint32_t));
            }

            secondaryPointer = config.headerSize + (entryCount * config.entrySize);

            auto entryDef = catalog.getStruct(config.entryStruct);
            if (entryDef) {
                parseStructArray(entryDef, entryCount, config.headerSize);
            }
        }
        else {
            for (const auto& rootStruct : config.rootStructs) {
                auto structDef = catalog.getStruct(rootStruct);
                if (structDef) {
                    parseStruct(structDef);
                }
            }
        }

        if (xmlMode && xmlOutputPath.empty()) {
            std::cout << logger.getXmlString() << std::endl;
        }
    }

    void parseStructArray(std::shared_ptr<parser::StructDefinition> structDef, size_t count, size_t startOffset) {
        size_t structSize = structDef->getFixedSize();
        for (size_t i = 0; i < count; i++) {
            size_t savedMainPointer = mainPointer;
            mainPointer = startOffset + (i * structSize);
            logger.printLineBreak();
            logger.logStruct(structDef->getName() + "[" + std::to_string(i) + "]", mainPointer, secondaryPointer);
            logger.increaseIndent();

            for (const auto& field : structDef->getFields()) {
                parseMember(field);
            }

            logger.decreaseIndent();
            logger.finishStructNode();
            logger.printLineBreak();
            mainPointer = savedMainPointer;
        }
    }

private:
    std::string filePath;
    std::string fileExtension;
    std::ifstream file;
    std::vector<uint8_t> fileData;
    size_t fileSize;
    size_t mainPointer;
    size_t secondaryPointer;
    Logger logger;
    bool xmlMode;
    std::string xmlOutputPath;

    void applyPadding(size_t paddingBytes) {
        if (paddingBytes > 0) {
            secondaryPointer += paddingBytes;
            //logger.logMember("padding", "__padding", std::to_string(paddingBytes), mainPointer, secondaryPointer);
        }
    }

    void parseStruct(std::shared_ptr<parser::StructDefinition> structDef) {
        logger.logStruct(structDef->getName(), mainPointer, secondaryPointer);
        logger.increaseIndent();

        const auto& fields = structDef->getFields();
        const auto& paddingInfos = structDef->getPaddingInfos();

        size_t fieldIndex = 0;
        size_t paddingIndex = 0;

        while (fieldIndex < fields.size() || paddingIndex < paddingInfos.size()) {
            if (fieldIndex < fields.size()) {
                parseMember(fields[fieldIndex]);
                fieldIndex++;

                if (paddingIndex < paddingInfos.size() && paddingIndex == fieldIndex - 1) {
                    const auto& padding = paddingInfos[paddingIndex];
                    if (padding.usesSecondaryPointer) {
                        secondaryPointer += padding.bytes;
                        //logger.logMember("padding", "__padding", std::to_string(padding.bytes), mainPointer, secondaryPointer);
                    }
                    paddingIndex++;
                }
            }
        }

        logger.decreaseIndent();
        logger.finishStructNode();
    }

    void parseMember(std::shared_ptr<parser::Field> field)  {
        const auto& typeName = field->getType()->getName();
        const auto& fieldName = field->getName();
        size_t offset = field->getOffset();
        bool usesSecondaryPointer = field->getUsesSecondaryPointer();

        size_t currentPointer;

        currentPointer = mainPointer + offset;
        if (fieldName == "diffuse" && "normal") {
            currentPointer = secondaryPointer + offset;
        }
        /*
        if (usesSecondaryPointer && offset != 0) {
            if (fieldName == "decalData") {
               currentPointer = mainPointer + offset;
               //return;
            }
            else {
                currentPointer = secondaryPointer + offset;
            }
        }
        else {
            currentPointer = mainPointer + offset;
        }*/

        if (field->isPadding()) {
            if (field->getUsesSecondaryPointer()) {
                size_t paddingSize = field->getPaddingSize();
                secondaryPointer += paddingSize;
                //logger.logMember("padding", field->getName(), paddingSize, mainPointer, secondaryPointer);
            }
            return;
        }

        if (field->hasPadding() && field->getUsesSecondaryPointer()) {
            size_t paddingBytes = field->getPaddingBytes();
            secondaryPointer += paddingBytes;
            logger.logMember("padding", field->getName() + "_padding", std::to_string(paddingBytes),
                currentPointer, secondaryPointer);
        }

        if (field->getType()->isStructType()) {
            std::string structName = field->getType()->getStructName();
            auto structDef = parser::Catalog::getInstance().getStruct(structName);
            if (structDef) {
                logger.printLineBreak();

                logger.logStruct(fieldName + " (" + structName + ")", currentPointer, secondaryPointer);

                size_t savedMainPointer = mainPointer;
                mainPointer = currentPointer;

                size_t savedSecondaryPointer = secondaryPointer;

                logger.increaseIndent();

                for (const auto& nestedField : structDef->getFields()) {
                    parseMember(nestedField);
                }

                logger.decreaseIndent();
                logger.finishStructNode();
                logger.printLineBreak();

                mainPointer = savedMainPointer;
                return;
            }
        }

        if (typeName == "bool") {
            bool value = false;
            if (currentPointer + sizeof(bool) <= fileSize) {
                memcpy(&value, fileData.data() + currentPointer, sizeof(bool));
            }

            if (usesSecondaryPointer) {
                secondaryPointer += sizeof(bool);            }
            logger.logMember(typeName, fieldName, value, currentPointer, secondaryPointer);
        }
        else if (typeName == "float") {
            float value = 0.0f;
            if (currentPointer + sizeof(float) <= fileSize) {
                memcpy(&value, fileData.data() + currentPointer, sizeof(float));
            }
            logger.logMember(typeName, fieldName, value, currentPointer, secondaryPointer);

            if (usesSecondaryPointer) {
                secondaryPointer += sizeof(float);
            }
        }
        else if (typeName == "int") {
            int value = 0;
            if (currentPointer + sizeof(int) <= fileSize) {
                memcpy(&value, fileData.data() + currentPointer, sizeof(int));
            }
            logger.logMember(typeName, fieldName, value, currentPointer, secondaryPointer);

            if (usesSecondaryPointer) {
                secondaryPointer += sizeof(int);
            }
        }
        else if (typeName == "uint32_t") {
            uint32_t value = 0;
            if (currentPointer + sizeof(uint32_t) <= fileSize) {
                memcpy(&value, fileData.data() + currentPointer, sizeof(uint32_t));
            }
            logger.logMember(typeName, fieldName, value, currentPointer, secondaryPointer);

            if (usesSecondaryPointer) {
                secondaryPointer += sizeof(int);
            }
        }
        else if (typeName == "uint64_t") {
            uint64_t value = 0;
            if (currentPointer + sizeof(uint64_t) <= fileSize) {
                memcpy(&value, fileData.data() + currentPointer, sizeof(uint64_t));
            }
            logger.logMember(typeName, fieldName, value, currentPointer, secondaryPointer);

            if (usesSecondaryPointer) {
                secondaryPointer += sizeof(int);
            }
        }
        else if (typeName == "enum") {
            uint32_t value = 0;
            if (currentPointer + sizeof(uint32_t) <= fileSize) {
                memcpy(&value, fileData.data() + currentPointer, sizeof(uint32_t));
            }
            logger.logMember(typeName, fieldName, value, currentPointer, secondaryPointer);

            if (usesSecondaryPointer) {
                secondaryPointer += sizeof(int);
            }
        }

        else if (typeName == "key" || typeName == "asset" || typeName == "nullable") {
            uint32_t mainValue = 0;
            if (currentPointer + sizeof(uint32_t) <= fileSize) {
                memcpy(&mainValue, fileData.data() + currentPointer, sizeof(uint32_t));
            }

            if (usesSecondaryPointer) {
                if (typeName == "key") {
                    std::string value = "<empty>";
                    size_t strPointer = secondaryPointer;

                    if (strPointer < fileSize) {
                        size_t end = strPointer;
                        while (end < fileSize && fileData[end] != 0) {
                            end++;
                        }

                        if (end > strPointer) {
                            value = std::string(reinterpret_cast<const char*>(fileData.data() + strPointer), end - strPointer);
                            secondaryPointer = end + 1;
                            if (fieldName == "nounDef") {
                                //secondaryPointer += 4;
                                //logger.logMember("padding", "nounDef_padding", std::to_string(5), currentPointer, secondaryPointer);
                            }
                        }
                    }

                    logger.logMemberWithSecondary(typeName, fieldName, mainValue, value, currentPointer, secondaryPointer);
                }
                else if (typeName == "asset") {
                    std::string value = "<empty>";
                    size_t strPointer = secondaryPointer;

                    if (strPointer < fileSize) {
                        size_t end = strPointer;
                        while (end < fileSize && fileData[end] != 0) {
                            end++;
                        }

                        if (end > strPointer) {
                            value = std::string(reinterpret_cast<const char*>(fileData.data() + strPointer), end - strPointer);
                            secondaryPointer = end + 1;
                        }
                    }

                    logger.logMemberWithSecondary(typeName, fieldName, mainValue, value, currentPointer, secondaryPointer);
                }
                else if (typeName == "nullable") {
                    uint32_t mainValue = 0;
                    if (currentPointer + sizeof(uint32_t) <= fileSize) {
                        memcpy(&mainValue, fileData.data() + currentPointer, sizeof(uint32_t));
                    }

                    // Log the nullable field itself
                    logger.logMember(typeName, fieldName, mainValue, currentPointer, secondaryPointer);

                    // Only proceed with reading the contained data if mainValue != 0
                    if (mainValue == 0) {
                        return; // Skip this field if it's null
                    }

                    if (usesSecondaryPointer) {
                        auto nullable = std::dynamic_pointer_cast<parser::NullableType>(field->getType());
                        if (nullable) {
                            std::shared_ptr<parser::DataType> containedType = nullable->getContainedType();

                            if (containedType && containedType->isStructType()) {
                                std::string structName = containedType->getStructName();
                                auto structDef = parser::Catalog::getInstance().getStruct(structName);

                                if (structDef) {
                                    // Get the base pointer for the struct
                                    size_t structBasePointer = secondaryPointer;

                                    logger.printLineBreak();
                                    logger.logStruct(fieldName + " (" + structName + ")", currentPointer, structBasePointer);

                                    size_t savedMainPointer = mainPointer;
                                    mainPointer = currentPointer; // Keep track of the original pointer

                                    logger.increaseIndent();

                                    // Process each field using absolute offsets from the struct base
                                    for (const auto& nestedField : structDef->getFields()) {
                                        // Calculate field pointer based on struct base + field offset
                                        size_t fieldPointer = structBasePointer + nestedField->getOffset();

                                        auto modifiedField = std::make_shared<parser::Field>(
                                            nestedField->getName(),
                                            nestedField->getType(),
                                            nestedField->getOffset(),
                                            true  // Force use of secondary pointer
                                        );

                                        // Save current secondaryPointer
                                        size_t savedSecondaryPointer = secondaryPointer;
                                        // Use the struct base pointer for parsing
                                        secondaryPointer = structBasePointer;

                                        // Parse the member
                                        parseMember(modifiedField);

                                        // Restore secondaryPointer to prevent accumulating offsets
                                        secondaryPointer = savedSecondaryPointer;
                                    }

                                    logger.decreaseIndent();
                                    logger.finishStructNode();
                                    logger.printLineBreak();

                                    // After processing the entire struct, advance the secondary pointer past it
                                    secondaryPointer = structBasePointer + structDef->getFixedSize();
                                    mainPointer = savedMainPointer;
                                }
                            }
                            else if (containedType->getName() == "char*") {
                                size_t originalSecondaryPointer = secondaryPointer;
                                std::string value = "<empty>";
                                size_t strPointer = secondaryPointer;
                                if (strPointer < fileSize) {
                                    size_t end = strPointer;
                                    while (end < fileSize && fileData[end] != 0) {
                                        end++;
                                    }
                                    if (end > strPointer) {
                                        value = std::string(reinterpret_cast<const char*>(fileData.data() + strPointer), end - strPointer);
                                        secondaryPointer = end + 1;
                                    }
                                }
                                logger.logMemberWithSecondary(typeName, fieldName, mainValue, value, currentPointer, originalSecondaryPointer);
                            }
                            else {
                                uint32_t secondaryValue = 0;
                                if (secondaryPointer + sizeof(uint32_t) <= fileSize) {
                                    memcpy(&secondaryValue, fileData.data() + secondaryPointer, sizeof(uint32_t));
                                    secondaryPointer += sizeof(uint32_t);
                                }
                                logger.logMemberWithSecondary(typeName, fieldName, mainValue, secondaryValue);
                            }
                        }
                    }
                }
                else {
                    uint32_t secondaryValue = 0;
                    if (secondaryPointer + sizeof(uint32_t) <= fileSize) {
                        memcpy(&secondaryValue, fileData.data() + secondaryPointer, sizeof(uint32_t));
                        secondaryPointer += sizeof(uint32_t);
                    }
                    logger.logMemberWithSecondary(typeName, fieldName, mainValue, secondaryValue);
                }
            }
            else {
                logger.logMember(typeName, fieldName, mainValue, currentPointer, secondaryPointer);
            }
        }
        else if (typeName == "dNullable") {
            uint32_t nullableFlag = 0;
            size_t origSecondaryPointer = secondaryPointer;

            if (secondaryPointer + sizeof(uint32_t) <= fileSize) {
                memcpy(&nullableFlag, fileData.data() + secondaryPointer, sizeof(uint32_t));
            }

            secondaryPointer += sizeof(uint32_t);

            logger.logMember("dNullable", fieldName, nullableFlag, currentPointer, origSecondaryPointer);
            if (nullableFlag == 0) {
                return;
            }

            auto containedType = parser::Catalog::getInstance().getType("struct:animatedData");
            if (containedType && containedType->isStructType()) {
                std::string structName = containedType->getStructName();
                auto structDef = parser::Catalog::getInstance().getStruct(structName);

                if (structDef) {
                    logger.printLineBreak();
                    logger.logStruct(fieldName + " (" + structName + ")", currentPointer, secondaryPointer);

                    size_t savedMainPointer = mainPointer;

                    logger.increaseIndent();

                    for (const auto& nestedField : structDef->getFields()) {
                        auto modifiedField = std::make_shared<parser::Field>(
                            nestedField->getName(),
                            nestedField->getType(),
                            nestedField->getOffset(),
                            true
                        );
                        parseMember(modifiedField);
                    }

                    logger.decreaseIndent();
                    logger.finishStructNode();
                    logger.printLineBreak();

                    mainPointer = savedMainPointer;
                }
            }
        }
        else if (typeName == "array") {
            uint32_t size = 0;
            uint32_t arrayOffset = 0;

            if (currentPointer + sizeof(uint32_t) <= fileSize) {
                memcpy(&size, fileData.data() + currentPointer, sizeof(uint32_t));
            }

            if (currentPointer + sizeof(uint32_t) * 2 <= fileSize) {
                memcpy(&arrayOffset, fileData.data() + currentPointer + sizeof(uint32_t), sizeof(uint32_t));
            }

            //std::string arrayInfo = "size=" + std::to_string(size) + ", offset=" + std::to_string(arrayOffset);
            std::string arrayInfo =std::to_string(size);
            logger.logMember(typeName, fieldName, arrayInfo);

            if (usesSecondaryPointer) {
                secondaryPointer += sizeof(int);
            }
        }
        else if (typeName == "vec3") {
            float values[3] = { 0.0f, 0.0f, 0.0f };
            if (currentPointer + sizeof(float) * 3 <= fileSize) {
                memcpy(values, fileData.data() + currentPointer, sizeof(float) * 3);
            }
            std::string vecInfo = "{" + std::to_string(values[0]) + ", " +
                std::to_string(values[1]) + ", " +
                std::to_string(values[2]) + "}";

            if (xmlMode) {
                logger.logVec3(fieldName, vecInfo, currentPointer, secondaryPointer);
            }
            else {
                logger.logMember(typeName, fieldName, vecInfo, currentPointer, secondaryPointer);
            }

            if (usesSecondaryPointer) {
                secondaryPointer += sizeof(float) * 3;
            }
        }
        else if (typeName == "char*" && usesSecondaryPointer) {
            std::string value = "<empty>";
            size_t strPointer = secondaryPointer;

            if (strPointer < fileSize) {
                size_t end = strPointer;
                while (end < fileSize && fileData[end] != 0) {
                    end++;
                }

                if (end > strPointer) {
                    value = std::string(reinterpret_cast<const char*>(fileData.data() + strPointer), end - strPointer);
                    secondaryPointer = end + 1;
                }
            }

            logger.logMemberWithSecondary(typeName, fieldName, secondaryPointer, value);
        }
        else {
            logger.logMember(typeName, fieldName, secondaryPointer);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path> [-p] [--xml [output_path]]" << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    bool showPointers = false;
    bool xmlMode = false;
    std::string xmlOutputPath = "";

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-p") {
            showPointers = true;
        }
        else if (arg == "--xml") {
            xmlMode = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                xmlOutputPath = argv[i + 1];
                i++;
            }
        }
    }

    try {
        parser::initializeCatalog();
        Parser parser(filePath, showPointers, xmlMode, xmlOutputPath);
        parser.parse();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}