#pragma once
#pragma warning(disable: 4251 4275)

#include <string>
#include <memory>
#include <stack>
#include <vector>
#include <fstream>
#include <iostream>
#include <pugixml.hpp>
#include <fmt/format.h>
#include <yaml-cpp/yaml.h>

class FormatExporter {
public:
    virtual ~FormatExporter() = default;

    virtual void beginDocument() = 0;
    virtual void endDocument() = 0;

    virtual void beginNode(const std::string& name) = 0;
    virtual void endNode() = 0;

    virtual void exportBool(const std::string& name, bool value) = 0;
    virtual void exportInt(const std::string& name, int value) = 0;
    virtual void exportUInt8(const std::string& name, uint8_t value) = 0;
    virtual void exportUInt16(const std::string& name, uint16_t value) = 0;
    virtual void exportUInt32(const std::string& name, uint32_t value) = 0;
    virtual void exportUInt64(const std::string& name, uint64_t value) = 0;
    virtual void exportInt64(const std::string& name, int64_t value) = 0;
    virtual void exportFloat(const std::string& name, float value) = 0;
    virtual void exportString(const std::string& name, const std::string& value) = 0;

    virtual void exportGuid(const std::string& name, const std::string& value) = 0;
    virtual void exportVector2(const std::string& name, float x, float y) = 0;
    virtual void exportVector3(const std::string& name, float x, float y, float z) = 0;
    virtual void exportQuaternion(const std::string& name, float w, float x, float y, float z) = 0;

    virtual void beginArray(const std::string& name) = 0;
    virtual void beginArrayEntry() = 0;
    virtual void endArrayEntry() = 0;
    virtual void endArray() = 0;

    virtual bool saveToFile(const std::string& filepath) = 0;
};

class XmlExporter : public FormatExporter {
private:
    pugi::xml_document xmlDoc;
    std::stack<pugi::xml_node> nodeStack;

public:
    XmlExporter() = default;
    ~XmlExporter() override = default;

    void beginDocument() override {
        while (!nodeStack.empty()) nodeStack.pop();
        nodeStack.push(xmlDoc);
    }

    void endDocument() override {
        while (nodeStack.size() > 1) {
            nodeStack.pop();
        }
    }

    void beginNode(const std::string& name) override {
        pugi::xml_node newNode = nodeStack.top().append_child(name.c_str());
        nodeStack.push(newNode);
    }

    void endNode() override {
        if (nodeStack.size() > 1) {
            nodeStack.pop();
        }
    }

    void exportBool(const std::string& name, bool value) override {
        nodeStack.top().append_child(name.c_str()).text().set(value ? "true" : "false");
    }

    void exportInt(const std::string& name, int value) override {
        nodeStack.top().append_child(name.c_str()).text().set(std::to_string(value).c_str());
    }

    void exportUInt8(const std::string& name, uint8_t value) override {
        nodeStack.top().append_child(name.c_str()).text().set(std::to_string(value).c_str());
    }

    void exportUInt16(const std::string& name, uint16_t value) override {
        nodeStack.top().append_child(name.c_str()).text().set(std::to_string(value).c_str());
    }

    void exportUInt32(const std::string& name, uint32_t value) override {
        nodeStack.top().append_child(name.c_str()).text().set(std::to_string(value).c_str());
    }

    void exportUInt64(const std::string& name, uint64_t value) override {
        nodeStack.top().append_child(name.c_str()).text().set(fmt::format("0x{:X}", value).c_str());
    }

    void exportInt64(const std::string& name, int64_t value) override {
        nodeStack.top().append_child(name.c_str()).text().set(fmt::format("0x{:X}", value).c_str());
    }

    void exportFloat(const std::string& name, float value) override {
        nodeStack.top().append_child(name.c_str()).text().set(fmt::format("{:.5f}", value).c_str());
    }

    void exportString(const std::string& name, const std::string& value) override {
        nodeStack.top().append_child(name.c_str()).text().set(value.c_str());
    }

    void exportGuid(const std::string& name, const std::string& value) override {
        nodeStack.top().append_child(name.c_str()).text().set(value.c_str());
    }

    void exportVector2(const std::string& name, float x, float y) override {
        auto vecNode = nodeStack.top().append_child(name.c_str());
        vecNode.append_child("x").text().set(fmt::format("{:.5f}", x).c_str());
        vecNode.append_child("y").text().set(fmt::format("{:.5f}", y).c_str());
    }

    void exportVector3(const std::string& name, float x, float y, float z) override {
        auto vecNode = nodeStack.top().append_child(name.c_str());
        vecNode.append_child("x").text().set(fmt::format("{:.5f}", x).c_str());
        vecNode.append_child("y").text().set(fmt::format("{:.5f}", y).c_str());
        vecNode.append_child("z").text().set(fmt::format("{:.5f}", z).c_str());
    }

    void exportQuaternion(const std::string& name, float w, float x, float y, float z) override {
        auto vecNode = nodeStack.top().append_child(name.c_str());
        vecNode.append_child("w").text().set(fmt::format("{:.5f}", w).c_str());
        vecNode.append_child("x").text().set(fmt::format("{:.5f}", x).c_str());
        vecNode.append_child("y").text().set(fmt::format("{:.5f}", y).c_str());
        vecNode.append_child("z").text().set(fmt::format("{:.5f}", z).c_str());
    }

    void beginArray(const std::string& name) override {
        pugi::xml_node arrayNode = nodeStack.top().append_child(name.c_str());
        nodeStack.push(arrayNode);
    }

    void beginArrayEntry() override {
        pugi::xml_node entryNode = nodeStack.top().append_child("entry");
        nodeStack.push(entryNode);
    }

    void endArrayEntry() override {
        if (nodeStack.size() > 1) {
            nodeStack.pop();
        }
    }

    void endArray() override {
        if (nodeStack.size() > 1) {
            nodeStack.pop();
        }
    }

    bool saveToFile(const std::string& filepath) override {
        return xmlDoc.save_file(filepath.c_str());
    }

    pugi::xml_document& getDocument() {
        return xmlDoc;
    }

    pugi::xml_node getCurrentNode() {
        return nodeStack.top();
    }
};

class YamlExporter : public FormatExporter {
private:
    YAML::Node rootNode;
    std::vector<YAML::Node*> nodeStack;
    std::map<std::string, YAML::Node> namedNodes;
    std::vector<YAML::Node> sequenceEntries;

public:
    YamlExporter() = default;
    ~YamlExporter() override = default;

    void beginDocument() override {
        rootNode = YAML::Node(YAML::NodeType::Map);
        nodeStack.clear();
        nodeStack.push_back(&rootNode);
        namedNodes.clear();
        sequenceEntries.clear();
    }

    void endDocument() override {
        while (nodeStack.size() > 1) {
            nodeStack.pop_back();
        }
    }

    void beginNode(const std::string& name) override {
        YAML::Node* currentNode = nodeStack.back();

        namedNodes[name] = YAML::Node(YAML::NodeType::Map);
        (*currentNode)[name] = namedNodes[name];

        nodeStack.push_back(&namedNodes[name]);
    }

    void endNode() override {
        if (nodeStack.size() > 1) {
            nodeStack.pop_back();
        }
    }

    void exportBool(const std::string& name, bool value) override {
        YAML::Node* currentNode = nodeStack.back();
        (*currentNode)[name] = value;
    }

    void exportInt(const std::string& name, int value) override {
        YAML::Node* currentNode = nodeStack.back();
        (*currentNode)[name] = value;
    }

    void exportUInt8(const std::string& name, uint8_t value) override {
        YAML::Node* currentNode = nodeStack.back();
        (*currentNode)[name] = static_cast<int>(value);
    }

    void exportUInt16(const std::string& name, uint16_t value) override {
        YAML::Node* currentNode = nodeStack.back();
        (*currentNode)[name] = static_cast<int>(value);
    }

    void exportUInt32(const std::string& name, uint32_t value) override {
        YAML::Node* currentNode = nodeStack.back();
        (*currentNode)[name] = static_cast<int64_t>(value);
    }

    void exportUInt64(const std::string& name, uint64_t value) override {
        YAML::Node* currentNode = nodeStack.back();
        (*currentNode)[name] = fmt::format("0x{:X}", value);
    }

    void exportInt64(const std::string& name, int64_t value) override {
        YAML::Node* currentNode = nodeStack.back();
        (*currentNode)[name] = fmt::format("0x{:X}", value);
    }

    void exportFloat(const std::string& name, float value) override {
        YAML::Node* currentNode = nodeStack.back();
        (*currentNode)[name] = fmt::format("{:.5f}", value);
    }

    void exportString(const std::string& name, const std::string& value) override {
        YAML::Node* currentNode = nodeStack.back();
        (*currentNode)[name] = value;
    }

    void exportGuid(const std::string& name, const std::string& value) override {
        YAML::Node* currentNode = nodeStack.back();
        (*currentNode)[name] = value;
    }

    void exportVector2(const std::string& name, float x, float y) override {
        YAML::Node* currentNode = nodeStack.back();
        YAML::Node vecNode;
        vecNode["x"] = fmt::format("{:.5f}", x);
        vecNode["y"] = fmt::format("{:.5f}", y);
        (*currentNode)[name] = vecNode;
    }

    void exportVector3(const std::string& name, float x, float y, float z) override {
        YAML::Node* currentNode = nodeStack.back();
        YAML::Node vecNode;
        vecNode["x"] = fmt::format("{:.5f}", x);
        vecNode["y"] = fmt::format("{:.5f}", y);
        vecNode["z"] = fmt::format("{:.5f}", z);
        (*currentNode)[name] = vecNode;
    }

    void exportQuaternion(const std::string& name, float w, float x, float y, float z) override {
        YAML::Node* currentNode = nodeStack.back();
        YAML::Node vecNode;
        vecNode["w"] = fmt::format("{:.5f}", w);
        vecNode["x"] = fmt::format("{:.5f}", x);
        vecNode["y"] = fmt::format("{:.5f}", y);
        vecNode["z"] = fmt::format("{:.5f}", z);
        (*currentNode)[name] = vecNode;
    }

    void beginArray(const std::string& name) override {
        YAML::Node* currentNode = nodeStack.back();

        namedNodes[name] = YAML::Node(YAML::NodeType::Sequence);
        (*currentNode)[name] = namedNodes[name];

        nodeStack.push_back(&namedNodes[name]);
    }

    void beginArrayEntry() override {
        YAML::Node* currentNode = nodeStack.back();

        sequenceEntries.push_back(YAML::Node(YAML::NodeType::Map));
        currentNode->push_back(sequenceEntries.back());

        nodeStack.push_back(&sequenceEntries.back());
    }

    void endArrayEntry() override {
        if (nodeStack.size() > 1) {
            nodeStack.pop_back();
        }
    }

    void endArray() override {
        if (nodeStack.size() > 1) {
            nodeStack.pop_back();
        }
    }

    bool saveToFile(const std::string& filepath) override {
        try {
            std::ofstream outFile(filepath);
            if (!outFile.is_open()) {
                return false;
            }

            YAML::Emitter emitter;
            emitter.SetIndent(2);
            emitter.SetMapFormat(YAML::Block);
            emitter.SetSeqFormat(YAML::Block);
            emitter << rootNode;

            outFile << emitter.c_str();
            outFile.close();
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Error saving YAML file: " << e.what() << std::endl;
            return false;
        }
    }
};

class ExporterFactory {
public:
    static std::unique_ptr<FormatExporter> createExporter(const std::string& format) {
        if (format == "xml") {
            return std::make_unique<XmlExporter>();
        }
        else if (format == "yaml" || format == "yml") {
            return std::make_unique<YamlExporter>();
        }
        else if (format != "none") {
            throw std::runtime_error("Unsupported export format: " + format);
        }

        return nullptr;
    }
};