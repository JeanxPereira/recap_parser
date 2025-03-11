#include "catalog.h"
#include <iostream>
#include <string>
#include <functional>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <stack>
#include <stdexcept>

namespace fs = std::filesystem;

class XmlWriter {
public:
    explicit XmlWriter(std::ostream& out) : mOut(out), mIndent(0) {
        mOut << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    }

    void startElement(const std::string& name) {
        writeIndent();
        mOut << "<" << name << ">\n";
        mElementStack.push(name);
        mIndent += 2;
    }

    void endElement() {
        if (mElementStack.empty()) return;

        mIndent -= 2;
        writeIndent();
        mOut << "</" << mElementStack.top() << ">\n";
        mElementStack.pop();
    }

    void writeElement(const std::string& name, const std::string& value) {
        writeIndent();
        mOut << "<" << name << ">" << escape(value) << "</" << name << ">\n";
    }

    void writeElement(const std::string& name, int value) {
        writeIndent();
        mOut << "<" << name << ">" << value << "</" << name << ">\n";
    }

    void writeElement(const std::string& name, long long value) {
        writeIndent();
        mOut << "<" << name << ">" << value << "</" << name << ">\n";
    }

    void writeElement(const std::string& name, unsigned int value) {
        writeIndent();
        mOut << "<" << name << ">" << value << "</" << name << ">\n";
    }

    void writeElement(const std::string& name, float value) {
        writeIndent();
        mOut << "<" << name << ">" << value << "</" << name << ">\n";
    }

    void writeElement(const std::string& name, double value) {
        writeIndent();
        mOut << "<" << name << ">" << value << "</" << name << ">\n";
    }

    void writeAttribute(const std::string& name, const std::string& value) {
        mOut.seekp(-2, std::ios_base::cur);
        mOut << " " << name << "=\"" << escape(value) << "\">\n";
    }

    void writeComment(const std::string& comment) {
        writeIndent();
        mOut << "<!-- " << comment << " -->\n";
    }

private:
    void writeIndent() {
        for (int i = 0; i < mIndent; ++i) {
            mOut << " ";
        }
    }

    std::string escape(const std::string& input) {
        std::string result;
        result.reserve(input.size());

        for (char c : input) {
            switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '\"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default: result += c; break;
            }
        }

        return result;
    }

    std::ostream& mOut;
    int mIndent;
    std::stack<std::string> mElementStack;
};

class TextWriter {
public:
    explicit TextWriter(std::ostream& out) : mOut(out), mIndent(0) {}

    void startObject(const std::string& name) {
        writeIndent();
        mOut << name << ":\n";
        mIndent += 2;
    }

    void endObject() {
        mIndent -= 2;
    }

    void writeProperty(const std::string& name, const std::string& value) {
        writeIndent();
        mOut << name << ": " << value << "\n";
    }

    void writeProperty(const std::string& name, int value) {
        writeIndent();
        mOut << name << ": " << value << "\n";
    }

    void writeProperty(const std::string& name, long long value) {
        writeIndent();
        mOut << name << ": " << value << "\n";
    }

    void writeProperty(const std::string& name, unsigned int value) {
        writeIndent();
        mOut << name << ": " << value << "\n";
    }

    void writeProperty(const std::string& name, float value) {
        writeIndent();
        mOut << name << ": " << value << "\n";
    }

    void writeProperty(const std::string& name, double value) {
        writeIndent();
        mOut << name << ": " << value << "\n";
    }

    void writeComment(const std::string& comment) {
        writeIndent();
        mOut << "# " << comment << "\n";
    }

private:
    void writeIndent() {
        for (int i = 0; i < mIndent; ++i) {
            mOut << " ";
        }
    }

    std::ostream& mOut;
    int mIndent;
};

class BinaryParser {
public:
    BinaryParser(std::vector<char> data)
        : mData(std::move(data)), mPosition(0), mKeyDataOffset(0) {
    }
    char read() {
        if (mPosition + sizeof(char) > mData.size()) {
            throw std::runtime_error("End of buffer reached");
        }

        char value = mData[mPosition];
        mPosition += sizeof(char);
        return value;
    }
    template<typename T>
    T read() {
        if (mPosition + sizeof(T) > mData.size()) {
            throw std::runtime_error("End of buffer reached");
        }

        T value = *reinterpret_cast<const T*>(mData.data() + mPosition);
        mPosition += sizeof(T);
        return value;
    }

    std::string readString() {
        std::string result;
        while (mPosition < mData.size() && mData[mPosition] != '\0') {
            result.push_back(mData[mPosition++]);
        }

        // Skip the null terminator
        if (mPosition < mData.size()) {
            mPosition++;
        }

        return result;
    }

    void seek(size_t position) {
        if (position >= mData.size()) {
            throw std::runtime_error("Seek beyond end of buffer");
        }
        mPosition = position;
    }

    size_t position() const {
        return mPosition;
    }

    size_t size() const {
        return mData.size();
    }

    bool eof() const {
        return mPosition >= mData.size();
    }

    size_t mKeyDataOffset;

    void setKeyDataOffset(size_t offset) {
        mKeyDataOffset = offset;
    }

    std::string readKeyString(uint32_t keyId = 1) {
        if (keyId == 0) {
            return "";
        }

        size_t currentPos = mPosition;
        seek(mKeyDataOffset);
        std::string result = readString();
        mKeyDataOffset += result.length() + 1;  // +1 for null terminator
        seek(currentPos);

        return result;
    }


    uint32_t readKeyValue(uint32_t keyId = 1) {
        if (keyId == 0) {
            return 0;
        }

        size_t currentPos = mPosition;
        seek(mKeyDataOffset);
        uint32_t value = read<uint32_t>();
        mKeyDataOffset += sizeof(uint32_t);

        seek(currentPos);

        return value;
    }

private:
    std::vector<char> mData;
    size_t mPosition;
};

class DarksporeParser {
public:
    DarksporeParser() {
        TypeDatabase::instance().initialize();

        // Register file type parsers
        registerParser("Phase", std::bind(&DarksporeParser::parseGeneric, this, std::placeholders::_1, std::placeholders::_2, "Phase"));
        registerParser("CharacterAnimation", std::bind(&DarksporeParser::parseGeneric, this, std::placeholders::_1, std::placeholders::_2, "CharacterAnimation"));
        registerParser("AiDefinition", std::bind(&DarksporeParser::parseGeneric, this, std::placeholders::_1, std::placeholders::_2, "AiDefinition"));
        registerParser("ClassAttributes", std::bind(&DarksporeParser::parseGeneric, this, std::placeholders::_1, std::placeholders::_2, "ClassAttributes"));
        registerParser("npcAffix", std::bind(&DarksporeParser::parseGeneric, this, std::placeholders::_1, std::placeholders::_2, "NpcAffix"));
        registerParser("PlayerClass", std::bind(&DarksporeParser::parseGeneric, this, std::placeholders::_1, std::placeholders::_2, "PlayerClass"));
        registerParser("NonPlayerClass", std::bind(&DarksporeParser::parseGeneric, this, std::placeholders::_1, std::placeholders::_2, "NonPlayerClass"));
        registerParser("Noun", std::bind(&DarksporeParser::parseGeneric, this, std::placeholders::_1, std::placeholders::_2, "Noun"));
        registerParser("markerSet", std::bind(&DarksporeParser::parseGeneric, this, std::placeholders::_1, std::placeholders::_2, "MarkerSet"));
        registerParser("catalog", std::bind(&DarksporeParser::parseGeneric, this, std::placeholders::_1, std::placeholders::_2, "Catalog"));
    }

    bool parse(const std::string& path, const std::string& filetype, bool recursive, bool exportXml) {
        //mExportXml = exportXml;

        auto parserIt = mParsers.find(filetype);
        if (parserIt == mParsers.end()) {
            std::cerr << "Unknown file type: " << filetype << std::endl;
            return false;
        }

        if (fs::is_directory(path)) {
            if (recursive) {
                return parseDirectory(path, filetype);
            }
            else {
                std::cerr << "Path is a directory but recursive option is not set" << std::endl;
                return false;
            }
        }
        else {
            return parseFile(path, parserIt->second);
        }
    }

private:
    using ParserFunc = std::function<bool(const std::string&, std::ostream&)>;

    bool parseDirectory(const fs::path& directory, const std::string& filetype) {
        bool success = true;

        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == "." + filetype) {
                auto parserIt = mParsers.find(filetype);
                if (parserIt != mParsers.end()) {
                    if (!parseFile(entry.path().string(), parserIt->second)) {
                        success = false;
                    }
                }
            }
        }

        return success;
    }

    bool parseFile(const std::string& filename, const ParserFunc& parser) {
        try {
            size_t pos = filename.find_last_of("/\\");
            std::string onlyFilename = (pos == std::string::npos) ? filename : filename.substr(pos + 1);
            std::cout << "parsing \"" << onlyFilename << "\"\n";

            if (mExportXml) {
                fs::path outPath = fs::path(filename).replace_extension(".xml");
                std::ofstream outFile(outPath);

                if (!outFile) {
                    std::cerr << "Failed to open output file: " << outPath << std::endl;
                    return false;
                }

                return parser(filename, outFile);
            }
            else {
                return parser(filename, std::cout);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing " << filename << ": " << e.what() << std::endl;
            return false;
        }
    }

    void registerParser(const std::string& filetype, ParserFunc parser) {
        mParsers[filetype] = std::move(parser);
    }

    void outputStructText(
        const std::string& structName,
        const std::string& data,
        std::ostream& out
    ) {
        TextWriter writer(out);
        parseStructToText(structName, data, writer);
    }

    void parseType(const std::shared_ptr<Type>& type, BinaryParser& parser, const std::string& fieldName, int indentLevel = 0) {
        std::string indent(indentLevel * 4, ' ');

        // If the type has fields, treat it as a struct
        if (!type->getFields().empty()) {
            std::cout << indent << "parse_struct(" << fieldName << ")\n";

            // Iterate through the fields in the order they were added
            for (const auto& field : type->getFields()) {
                // Calculate the position to read from based on whether the field uses key data
                size_t readPosition;
                if (field.useKeyData) {
                    readPosition = parser.mKeyDataOffset + field.offset;
                }
                else {
                    readPosition = field.offset;
                }

                parser.seek(readPosition);

                // If the field is a nested struct
                if (!field.type->getFields().empty()) {
                    std::cout << "\n";
                    parseType(field.type, parser, field.name, indentLevel + 1);
                    std::cout << "\n";
                }
                else {
                    // Process primitive fields
                    if (field.type->getName() == "enum") {
                        uint32_t value = parser.read<uint32_t>();
                        std::cout << indent << "    parse_member_enum(" << field.name << ", " << value << ")\n";
                    }
                    else if (field.type->getName() == "array") {
                        uint32_t value = parser.read<uint32_t>();
                        std::cout << indent << "    parse_member_array(" << field.name << ", " << value << ")\n";
                    }
                    else if (field.type->getName() == "bool") {
                        bool value = parser.read<uint8_t>() != 0;
                        std::cout << indent << "    parse_member_bool(" << field.name << ", " << (value ? "true" : "false") << ")\n";
                    }
                    else if (field.type->getName() == "float") {
                        float value = parser.read<float>();
                        std::cout << indent << "    parse_member_float(" << field.name << ", " << value << ")\n";
                    }
                    else if (field.type->getName() == "padding") {
                        if (field.useKeyData) {
                            parser.mKeyDataOffset += field.offset;
                        }
                        //std::cout << indent << "    parse_member_padding(" << field.name << ", " << field.offset << " bytes)\n";
                    }
                    else if (field.type->getName() == "uint64_t") {
                        uint64_t value = parser.read<uint64_t>();
                        std::cout << indent << "    parse_member_u64(" << field.name << ", " << value << ")\n";
                    }
                    else if (field.type->getName() == "uint32_t") {
                        uint32_t value = parser.read<uint32_t>();
                        std::cout << indent << "    parse_member_u32(" << field.name << ", " << value << ")\n";
                    }
                    else if (field.type->getName() == "int") {
                        int32_t value = parser.read<int32_t>();
                        std::cout << indent << "    parse_member_int(" << field.name << ", " << value << ")\n";
                    }
                    else if (field.type->getName() == "key") {
                        uint32_t keyId = parser.read<uint32_t>();
                        if (keyId != 0) {
                            std::string keyValue = parser.readKeyString();
                            std::cout << indent << "    parse_member_key(" << field.name << ", " << keyId << ", " << keyValue << ")\n";
                        }
                        else {
                            //std::cout << indent << "    parse_member_key(" << field.name << ", " << keyId << ", NULL)\n";
                        }
                    }
                    else if (field.type->getName() == "char") {
                        std::string string = parser.readString();
                        std::cout << indent << "    parse_member_char(" << field.name << ", " << string << ")\n";
                    }
                    else if (field.type->getName() == "char*") {
                        uint32_t stringId = parser.read<uint32_t>();
                        if (stringId != 0) {
                            std::string stringValue = parser.readKeyString();
                            std::cout << indent << "    parse_member_char*(" << field.name << ", " << stringId << ", " << stringValue << ")\n";
                        }
                    }
                    else if (field.type->getName() == "cLocalizedAssetString") {
                        uint32_t assetString = parser.read<uint32_t>();
                        if (assetString != 0) {
                            std::string assetValue = parser.readKeyString();
                            std::cout << indent << "    parse_member_cLocalizedAssetString(" << field.name << ", " << assetString << ", " << assetValue << ")\n";
                        }
                    }
                    else if (field.type->getName() == "asset") {
                        uint32_t assetId = parser.read<uint32_t>();
                        if (assetId != 0) {
                            std::string assetValue = parser.readKeyString();
                            std::cout << indent << "    parse_member_asset(" << field.name << ", " << assetId << ", " << assetValue << ")\n";
                        }
                    }
                    else if (field.type->getName() == "nullable") {
                        uint32_t value = parser.read<uint32_t>();
                        std::cout << indent << "    parse_member_nullable(" << field.name << ", " << value << ")\n";
                    }
                    else if (field.type->getName() == "asset") {
                        uint32_t value = parser.read<uint32_t>();
                        std::cout << indent << "    parse_member_asset(" << field.name << ", " << value << ")\n";
                    }
                    else if (field.type->getName() == "vec3") {
                        vec3 value;
                        value.x = parser.read<float>();
                        value.y = parser.read<float>();
                        value.z = parser.read<float>();
                        std::cout << indent << "    parse_member_vec3(" << field.name
                            << ", x: " << value.x
                            << ", y: " << value.y
                            << ", z: " << value.z << ")\n";
                    }
                    else if (field.type->getName() != "padding") {
                        std::cout << indent << "    parse_member_unk(" << field.name << ")\n";
                    }
                    if (field.useKeyData) {
                        // Update keyDataOffset based on the field type
                        if (field.type->getName() == "bool") {
                            parser.mKeyDataOffset += sizeof(uint8_t);
                        }
                        else if (field.type->getName() == "float") {
                            parser.mKeyDataOffset += sizeof(float);
                        }
                        else if (field.type->getName() == "uint64_t") {
                            parser.mKeyDataOffset += sizeof(uint64_t);
                        }
                        else if (field.type->getName() == "vec3") {
                            parser.mKeyDataOffset += sizeof(vec3);
                        }
                        else if (field.type->getName() == "enum" ||
                            field.type->getName() == "key" ||
                            field.type->getName() == "char*" ||
                            field.type->getName() == "asset" ||
                            field.type->getName() == "nullable" ||
                            field.type->getName() == "array") {
                            parser.mKeyDataOffset += sizeof(uint32_t);
                        }
                        // For complex types, we need to calculate the size differently
                        else if (!field.type->getFields().empty()) {
                            parser.mKeyDataOffset += field.type->getSize();
                        }
                    }
                }
            }
        }
        else {
            // If it is a primitive type
            if (type->getName() == "enum") {
                uint32_t value = parser.read<uint32_t>();
                std::cout << indent << "parse_member_enum(" << fieldName << ", " << value << ")\n";
            }
            else if (type->getName() == "bool") {
                bool value = parser.read<uint8_t>() != 0;
                std::cout << indent << "parse_member_bool(" << fieldName << ", " << (value ? "true" : "false") << ")\n";
            }
            else if (type->getName() == "float") {
                float value = parser.read<float>();
                std::cout << indent << "parse_member_float(" << fieldName << ", " << value << ")\n";
            }
            else if (type->getName() == "key") {
                uint32_t keyId = parser.read<uint32_t>();
                if (keyId != 0) {
                    std::string keyValue = parser.readKeyString(keyId);
                    std::cout << indent << "parse_member_key(" << fieldName << ", " << keyId << ", " << keyValue << ")\n";
                }
                else {
                    //std::cout << indent << "parse_member_key(" << fieldName << ", " << keyId << ", NULL)\n";
                }
            }
            else if (type->getName() == "char*") {
                uint32_t stringId = parser.read<uint32_t>();
                if (stringId != 0) {
                    std::string stringValue = parser.readKeyString(stringId);
                    std::cout << indent << "parse_member_char*(" << fieldName << ", " << stringId << ", " << stringValue << ")\n";
                }
                else {
                    //std::cout << indent << "parse_member_char*(" << fieldName << ", " << stringId << ", NULL)\n";
                }
            }
            else if (type->getName() == "asset") {
                uint32_t assetId = parser.read<uint32_t>();
                if (assetId != 0) {
                    std::string assetValue = parser.readKeyString(assetId);
                    std::cout << indent << "parse_member_asset(" << fieldName << ", " << assetId << ", " << assetValue << ")\n";
                }
                else {
                    //std::cout << indent << "parse_member_asset(" << fieldName << ", " << assetId << ", NULL)\n";
                }
            }
            else if (type->getName() == "vec3") {
                vec3 value;
                value.x = parser.read<float>();
                value.y = parser.read<float>();
                value.z = parser.read<float>();
                std::cout << indent << "parse_member_vec3(" << fieldName << ", x: "
                    << value.x << ", y: " << value.y << ", z: " << value.z << ")\n";
            }
        }
    }

    void parseStructToText(const std::string& structName, const std::string& data, TextWriter& writer) {
        auto structType = TypeDatabase::instance().getType(structName);
        if (!structType) {
            std::cerr << "  # Unknown structure type\n";
            return;
        }

        BinaryParser parser(std::vector<char>(data.begin(), data.end()));
        // Não ordena os campos para manter a ordem de declaração
        parseType(structType, parser, structName, 0);

        writer.endObject();
    }

    // Generic parser for most file types
    bool parseGeneric(const std::string& filename, std::ostream& out, const std::string& typeName) {
        // Read file content
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }

        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> data(fileSize);
        file.read(data.data(), fileSize);

        BinaryParser parser(std::vector<char>(data.data(), data.data() + fileSize));

        if (typeName == "PlayerClass") {
            parser.setKeyDataOffset(0x100);
        }
        else if (typeName == "NonPlayerClass") {
            parser.setKeyDataOffset(0x7C);
        }
        else if (typeName == "Noun") {
            parser.setKeyDataOffset(0x1E0);
        }
        else if (typeName == "CharacterAnimation") {
            parser.setKeyDataOffset(0x294);
        }
        else if (typeName == "AiDefinition") {
            parser.setKeyDataOffset(0x90);   // wrong
        }
        else if (typeName == "ClassAttributes") {
            parser.setKeyDataOffset(0x0);
        }
        else if (typeName == "NpcAffix") {
            parser.setKeyDataOffset(0xB0);   // wrong
        }
        else if (typeName == "MarkerSet") {
            parser.setKeyDataOffset(0xC0);   // wrong
        }
        else if (typeName == "Phase") {
            parser.setKeyDataOffset(0x78); // basic (wrong)
        }
        else if (typeName == "Catalog") {
            parser.setKeyDataOffset(0xE0);   // wrong
        }
        else {
            parser.setKeyDataOffset(0x100);  // default for other files
        }

        auto structType = TypeDatabase::instance().getType(typeName);
        if (!structType) {
            std::cerr << "Unknown structure type: " << typeName << std::endl;
            return false;
        }

        TextWriter writer(out);
        //writer.startObject(typeName);
        parseType(structType, parser, typeName);
        writer.endObject();

        return true;
    }

    std::unordered_map<std::string, ParserFunc> mParsers;
    bool mExportXml = false;
};


void printUsage() {
    std::cout << "Usage: darkspore_parser path filetype [recursive] [xml]" << std::endl;
    std::cout << "  path: path to file or directory" << std::endl;
    std::cout << "  filetype: One of: phase, CharacterAnimation, AIDefinition, ClassAttributes, npcAffix," << std::endl;
    std::cout << "            PlayerClass, nonPlayerClass, noun, markerSet, catalog" << std::endl;
    std::cout << "  recursive: parse all files in path (filetype as extension)" << std::endl;
    std::cout << "  xml: export parsed files as xml" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage();
        return 1;
    }

    std::string path = argv[1];
    std::string filetype = argv[2];
    bool recursive = (argc > 3 && std::string(argv[3]) == "recursive");
    bool exportXml = (argc > 4 && std::string(argv[4]) == "xml") ||
        (argc > 3 && std::string(argv[3]) == "xml");

    DarksporeParser parser;
    if (!parser.parse(path, filetype, recursive, exportXml)) {
        return 1;
    }

    return 0;
}