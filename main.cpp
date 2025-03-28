#include "catalog.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <pugixml.hpp>

namespace fs = std::filesystem;

#include <boost/interprocess/offset_ptr.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>

using namespace boost::interprocess;
using namespace recap;

class OffsetManager {
public:
    OffsetManager(size_t primaryStart, size_t secondaryStart) {
        // Inicializa a base de memória
        memBase = new char[1];
        primaryOffset = memBase + primaryStart;
        secondaryOffset = memBase + secondaryStart;
    }

    ~OffsetManager() {
        delete[] memBase;
    }

    // Obtém o offset primário atual
    size_t getPrimaryOffset() const {
        return primaryOffset.get() - memBase;
    }

    // Obtém o offset secundário atual
    size_t getSecondaryOffset() const {
        return secondaryOffset.get() - memBase;
    }

    // Ajusta o offset primário
    void setPrimaryOffset(size_t offset) {
        primaryOffset = memBase + offset;
    }

    // Ajusta o offset secundário
    void setSecondaryOffset(size_t offset) {
        secondaryOffset = memBase + offset;
    }

    // Avança o offset primário
    void advancePrimary(size_t amount) {
        primaryOffset += amount;
    }

    // Avança o offset secundário
    void advanceSecondary(size_t amount) {
        secondaryOffset += amount;
    }

    // Cria um offset baseado no primário
    size_t getAbsolutePrimaryOffset(size_t relativeOffset) const {
        return getPrimaryOffset() + relativeOffset;
    }

    // Cria um offset baseado no secundário
    size_t getAbsoluteSecondaryOffset(size_t relativeOffset) const {
        return getSecondaryOffset() + relativeOffset;
    }

private:
    char* memBase;  // Base para calcular offsets relativos
    offset_ptr<char> primaryOffset;
    offset_ptr<char> secondaryOffset;
};

// Versão modificada do Parser usando OffsetManager
class Parser {
public:
    Parser(Catalog& catalog) : catalog_(catalog), outputMode_(OutputMode::LOG) {}

    void setOutputMode(OutputMode mode) {
        outputMode_ = mode;
    }

    bool parseFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return false;
        }

        fs::path filePath(filename);
        std::string extension = filePath.extension().string();

        auto fileType = catalog_.getFileType(extension);
        if (!fileType) {
            std::cerr << "Unsupported file type: " << extension << std::endl;
            return false;
        }

        if (outputMode_ == OutputMode::LOG || outputMode_ == OutputMode::DEBUG_LOG) {
            std::cout << "parsing \"" << filePath.filename().string() << "\"" << std::endl;
        }

        pugi::xml_document doc;
        pugi::xml_node rootNode;

        if (outputMode_ == OutputMode::XML) {
            pugi::xml_node decl = doc.append_child(pugi::node_declaration);
            decl.append_attribute("version") = "1.0";
            std::string rootStructName = fileType->getRootStructs()[0];
            std::transform(rootStructName.begin(), rootStructName.end(), rootStructName.begin(),
                [](unsigned char c) { return std::tolower(c); });

            rootNode = doc.append_child(rootStructName.c_str());
        }

        // Inicializa o gerenciador de offsets
        OffsetManager offsets(0, fileType->getSecondaryOffset());

        for (const auto& rootStructName : fileType->getRootStructs()) {
            auto structType = std::dynamic_pointer_cast<StructType>(catalog_.getType("struct:" + rootStructName));
            if (!structType) {
                std::cerr << "Structure not found: " << rootStructName << std::endl;
                continue;
            }

            if (outputMode_ == OutputMode::LOG) {
                std::cout << structType->formatLog(rootStructName, "") << std::endl;
                processStruct(file, structType, offsets, 1);
            }
            else if (outputMode_ == OutputMode::DEBUG_LOG) {
                std::cout << "(" << offsets.getPrimaryOffset() << ", " << offsets.getSecondaryOffset() << ") "
                    << structType->formatLog(rootStructName, "") << std::endl;
                processStruct(file, structType, offsets, 1, true);
            }
            else if (outputMode_ == OutputMode::XML) {
                processStructXml(file, structType, offsets, rootNode);
            }
        }

        if (outputMode_ == OutputMode::XML) {
            std::string xmlFilename = filePath.stem().string() + ".xml";
            doc.save_file(xmlFilename.c_str());
            std::cout << "XML saved to: " << xmlFilename << std::endl;
        }

        return true;
    }

private:
    void processStruct(std::ifstream& file, StructTypePtr structType, OffsetManager& offsets,
        int indentLevel, bool showOffsets = false) {

        std::string indent(indentLevel * 4, ' ');

        for (const auto& field : structType->getFields()) {
            // Calcula o offset de leitura com base no campo
            size_t readOffset;
            if (field->usesSecondaryOffset()) {
                readOffset = offsets.getAbsoluteSecondaryOffset(field->getOffset());
            }
            else {
                readOffset = offsets.getAbsolutePrimaryOffset(field->getOffset());
            }

            // Cria uma cópia local do offset secundário
            size_t localSecondaryOffset = offsets.getSecondaryOffset();

            // Lê o valor do campo
            std::string value = field->getType()->read(file, readOffset, localSecondaryOffset);

            // Atualiza o offset secundário no gerenciador
            offsets.setSecondaryOffset(localSecondaryOffset);

            if (value == "IGNORE") {
                continue;
            }

            // Processa structs aninhadas
            if (auto nestedStructType = std::dynamic_pointer_cast<StructType>(field->getType())) {
                if (showOffsets) {
                    std::cout << indent << "(" << readOffset << ", " << offsets.getSecondaryOffset() << ") "
                        << nestedStructType->formatLog(field->getName(), "") << std::endl;
                }
                else {
                    std::cout << indent << nestedStructType->formatLog(field->getName(), "") << std::endl;
                }

                // Cria um gerenciador de offset temporário para a struct aninhada
                OffsetManager nestedOffsets = offsets;
                if (field->usesSecondaryOffset()) {
                    nestedOffsets.setPrimaryOffset(offsets.getSecondaryOffset() + field->getOffset());
                }
                else {
                    nestedOffsets.setPrimaryOffset(offsets.getPrimaryOffset() + field->getOffset());
                }

                processStruct(file, nestedStructType, nestedOffsets, indentLevel + 1, showOffsets);

                // Restaura apenas o offset secundário do gerenciador principal
                offsets.setSecondaryOffset(nestedOffsets.getSecondaryOffset());
            }
            // Processa campos "nullable" que podem conter structs
            else if (auto nullableType = std::dynamic_pointer_cast<NullableType>(field->getType())) {
                if (value.substr(0, 7) == "struct:") {
                    std::string structName = value.substr(7);
                    auto structContentType = std::dynamic_pointer_cast<StructType>(catalog_.getType("struct:" + structName));

                    if (structContentType) {
                        uint32_t indicator = 0;
                        file.seekg(readOffset);
                        file.read(reinterpret_cast<char*>(&indicator), sizeof(indicator));

                        if (indicator != 0) {
                            if (showOffsets) {
                                std::cout << indent << "(" << readOffset << ", " << offsets.getSecondaryOffset() << ") "
                                    << structContentType->formatLog(field->getName(), "") << std::endl;
                            }
                            else {
                                std::cout << indent << structContentType->formatLog(field->getName(), "") << std::endl;
                            }

                            // Para structs nullable, criamos um contexto especial
                            processNullableStruct(file, structContentType, readOffset + 4, offsets, indentLevel + 1, showOffsets);
                        }
                    }
                }
                else {
                    if (showOffsets) {
                        std::cout << indent << "(" << readOffset << ", " << offsets.getSecondaryOffset() << ") "
                            << field->getType()->formatLog(field->getName(), value) << std::endl;
                    }
                    else {
                        std::cout << indent << field->getType()->formatLog(field->getName(), value) << std::endl;
                    }
                }
            }
            // Processa campos regulares
            else {
                if (showOffsets) {
                    std::cout << indent << "(" << readOffset << ", " << offsets.getSecondaryOffset() << ") "
                        << field->getType()->formatLog(field->getName(), value) << std::endl;
                }
                else {
                    std::cout << indent << field->getType()->formatLog(field->getName(), value) << std::endl;
                }
            }
        }
    }

    // Função especial para processar structs dentro de campos nullable
    void processNullableStruct(std::ifstream& file, StructTypePtr structType, size_t fixedPrimaryDisplay,
        OffsetManager& offsets, int indentLevel, bool showOffsets = false) {

        std::string indent(indentLevel * 4, ' ');

        for (const auto& field : structType->getFields()) {
            // Para campos em uma struct nullable, sempre lemos do offset secundário
            size_t readOffset = offsets.getAbsoluteSecondaryOffset(field->getOffset());

            // Lê o valor do campo
            size_t localSecondaryOffset = offsets.getSecondaryOffset();
            std::string value = field->getType()->read(file, readOffset, localSecondaryOffset);
            offsets.setSecondaryOffset(localSecondaryOffset);

            if (value == "IGNORE") {
                continue;
            }

            // Exibe o offset primário fixo, mas o offset secundário real
            if (showOffsets) {
                std::cout << indent << "(" << fixedPrimaryDisplay << ", " << offsets.getSecondaryOffset() << ") "
                    << field->getType()->formatLog(field->getName(), value) << std::endl;
            }
            else {
                std::cout << indent << field->getType()->formatLog(field->getName(), value) << std::endl;
            }

            // Processa structs aninhadas
            if (auto nestedStructType = std::dynamic_pointer_cast<StructType>(field->getType())) {
                processNullableStruct(file, nestedStructType, fixedPrimaryDisplay, offsets, indentLevel + 1, showOffsets);
            }
            else if (auto nestedNullable = std::dynamic_pointer_cast<NullableType>(field->getType())) {
                if (value.substr(0, 7) == "struct:") {
                    std::string structName = value.substr(7);
                    auto nestedStructType = std::dynamic_pointer_cast<StructType>(catalog_.getType("struct:" + structName));

                    if (nestedStructType) {
                        uint32_t indicator = 0;
                        file.seekg(readOffset);
                        file.read(reinterpret_cast<char*>(&indicator), sizeof(indicator));

                        if (indicator != 0) {
                            // Para structs nullable aninhadas, incrementamos o offset fixo em +4
                            processNullableStruct(file, nestedStructType, fixedPrimaryDisplay + 4, offsets, indentLevel + 1, showOffsets);
                        }
                    }
                }
            }
        }
    }

    // Processamento XML
    void processStructXml(std::ifstream& file, StructTypePtr structType, OffsetManager& offsets,
        pugi::xml_node& parentNode) {

        for (const auto& field : structType->getFields()) {
            // Calcula o offset de leitura
            size_t fieldOffset;
            if (field->usesSecondaryOffset()) {
                fieldOffset = offsets.getAbsoluteSecondaryOffset(field->getOffset());
            }
            else {
                fieldOffset = offsets.getAbsolutePrimaryOffset(field->getOffset());
            }

            // Processa structs aninhadas
            if (auto nestedStructType = std::dynamic_pointer_cast<StructType>(field->getType())) {
                pugi::xml_node childNode = parentNode.append_child(field->getName().c_str());

                // Cria um gerenciador de offset temporário para a struct aninhada
                OffsetManager nestedOffsets = offsets;
                if (field->usesSecondaryOffset()) {
                    nestedOffsets.setPrimaryOffset(offsets.getSecondaryOffset() + field->getOffset());
                }
                else {
                    nestedOffsets.setPrimaryOffset(offsets.getPrimaryOffset() + field->getOffset());
                }

                processStructXml(file, nestedStructType, nestedOffsets, childNode);

                // Restaura apenas o offset secundário
                offsets.setSecondaryOffset(nestedOffsets.getSecondaryOffset());
            }
            // Processa campos nullable
            else if (auto nullableType = std::dynamic_pointer_cast<NullableType>(field->getType())) {
                uint32_t indicator = 0;
                file.seekg(fieldOffset);
                file.read(reinterpret_cast<char*>(&indicator), sizeof(indicator));

                if (indicator != 0) {
                    auto contentType = nullableType->getContentType();

                    if (auto structContentType = std::dynamic_pointer_cast<StructType>(contentType)) {
                        pugi::xml_node childNode = parentNode.append_child(field->getName().c_str());

                        // Para structs nullable, usamos o offset secundário para ambos
                        OffsetManager nullableOffsets = offsets;
                        nullableOffsets.setPrimaryOffset(offsets.getSecondaryOffset());

                        processStructXml(file, structContentType, nullableOffsets, childNode);

                        // Restaura apenas o offset secundário
                        offsets.setSecondaryOffset(nullableOffsets.getSecondaryOffset());
                    }
                    else if (contentType) {
                        size_t localSecondaryOffset = offsets.getSecondaryOffset();
                        std::string value = contentType->read(file, fieldOffset, localSecondaryOffset);
                        offsets.setSecondaryOffset(localSecondaryOffset);
                        contentType->formatXml(parentNode, field->getName(), value);
                    }
                }
            }
            // Processa campos regulares
            else {
                size_t localSecondaryOffset = offsets.getSecondaryOffset();
                std::string value = field->getType()->read(file, fieldOffset, localSecondaryOffset);
                offsets.setSecondaryOffset(localSecondaryOffset);

                if (value != "IGNORE") {
                    field->getType()->formatXml(parentNode, field->getName(), value);
                }
            }
        }
    }

    Catalog& catalog_;
    OutputMode outputMode_;
};

void setupCatalog(Catalog& catalog) {
    auto bool_type = catalog.getType("bool");
    auto char_type = catalog.getType("char");
    auto char_ptr_type = catalog.getType("char*");
    auto uint32_type = catalog.getType("uint32_t");
    auto uint64_type = catalog.getType("uint64_t");
    auto float_type = catalog.getType("float");
    auto vec3_type = catalog.getType("cSPVector3");
    auto key_type = catalog.createKeyType();
    auto asset_type = catalog.getType("asset");
    auto array_type = catalog.getType("array");
    auto enum_type = catalog.getType("enum");

    catalog.registerFileType(".Phase", { "Phase" }, 68);
    catalog.registerFileType(".Noun", { "Noun" }, 480);
    catalog.registerFileType(".ClassAttributes", { "ClassAttributes" });
    catalog.registerFileType(".PlayerClass", { "PlayerClass" }, 256);
    catalog.registerFileType(".NonPlayerClass", { "NonPlayerClass" }, 124);
    catalog.registerFileType(".CharacterAnimation", { "CharacterAnimation" }, 660);
    {
        auto phase = catalog.addStruct("Phase");
        phase->addField("gambit", array_type, 0);
        phase->addField("phaseType", enum_type, 4);
        phase->addField("cGambitDefinition", key_type, 52, true);
        phase->addField("startNode", bool_type, 12);
    }
    {
        auto noun = catalog.addStruct("Noun");
        noun->addField("nounType", enum_type, 0);
        noun->addField("clientOnly", bool_type, 4);
        noun->addField("isFixed", bool_type, 5);
        noun->addField("isSelfPowered", bool_type, 6);
        noun->addField("lifetime", float_type, 12);
        noun->addField("gfxPickMethod", enum_type, 8);
        noun->addField("graphicsScale", float_type, 20);
        noun->addField("modelKey", key_type, 36);
        noun->addField("prefab", key_type, 16);
        noun->addField("levelEditorModelKey", key_type, 52);

        auto boundingBox = catalog.addStruct("cSPBoundingBox");
        boundingBox->addField("min", catalog.getType("cSPVector3"), 0);
        boundingBox->addField("max", catalog.getType("cSPVector3"), 12);
        noun->addField("boundingBox", catalog.getType("struct:cSPBoundingBox"), 56);

        noun->addField("presetExtents", enum_type, 80);
        noun->addField("voice", key_type, 96);
        noun->addField("foot", key_type, 112);
        noun->addField("flightSound", key_type, 128);

        auto gfxStates = catalog.addStruct("cGameObjectGfxStateData", 56);
        gfxStates->addField("name", key_type, 12, true);
        gfxStates->addField("model", key_type, 28, true);
        gfxStates->addField("prefab", asset_type, 48, true);
        gfxStates->addField("animation", key_type, 44, true);
        gfxStates->addField("animationLoops", bool_type, 52, true);
        noun->addNullableField("gfxStates", catalog.getType("struct:cGameObjectGfxStateData"), 132);

        auto cNewGfxState = catalog.addStruct("cNewGfxState", 40);
		cNewGfxState->addField("prefab", asset_type, 0, true);
		cNewGfxState->addField("model", key_type, 16, true);
		cNewGfxState->addField("animation", key_type, 32, true);

        auto doorDef = catalog.addStruct("cDoorDef", 24);
        doorDef->addNullableField("graphicsState_open", catalog.getType("struct:cNewGfxState"), 0);
        doorDef->addNullableField("graphicsState_opening", catalog.getType("struct:cNewGfxState"), 4);
        doorDef->addNullableField("graphicsState_closed", catalog.getType("struct:cNewGfxState"), 8);
        doorDef->addNullableField("graphicsState_closing", catalog.getType("struct:cNewGfxState"), 12);
        doorDef->addField("clickToOpen", bool_type, 16, true);
        doorDef->addField("clickToClose", bool_type, 17, true);
        doorDef->addField("initialState", bool_type, 20, true);
        noun->addNullableField("doorDef", catalog.getType("struct:cDoorDef"), 136);

        auto switchDef = catalog.addStruct("cSwitchDef", 12);
        switchDef->addNullableField("graphicsState_unpressed", catalog.getType("struct:cNewGfxState"), 0);
        switchDef->addNullableField("graphicsState_pressing", catalog.getType("struct:cNewGfxState"), 4);
        switchDef->addNullableField("graphicsState_pressed", catalog.getType("struct:cNewGfxState"), 8);
        noun->addNullableField("switchDef", catalog.getType("struct:cSwitchDef"), 140);

        auto CrystalDef = catalog.addStruct("CrystalDef", 12);
        CrystalDef->addField("clickToOpen", uint32_type, 0, true);
        CrystalDef->addField("clickToClose", enum_type, 4, true);
        CrystalDef->addField("initialState", enum_type, 8, true);
        noun->addNullableField("crystalDef", catalog.getType("struct:CrystalDef"), 140);

        noun->addField("assetId", uint64_type, 152);
        noun->addField("npcClassData", asset_type, 160);
        noun->addField("playerClassData", asset_type, 164);
        noun->addField("characterAnimationData", asset_type, 168);

        auto cThumbnailCaptureParameters = catalog.addStruct("creatureThumbnailData", 108);
        cThumbnailCaptureParameters->addField("fovY", float_type, 0, true);
        cThumbnailCaptureParameters->addField("nearPlane", float_type, 4, true);
        cThumbnailCaptureParameters->addField("farPlane", float_type, 8, true);
        cThumbnailCaptureParameters->addField("cameraPosition", vec3_type, 12, true);
        cThumbnailCaptureParameters->addField("cameraScale", float_type, 24, true);
        cThumbnailCaptureParameters->addField("cameraRotation_0", vec3_type, 28, true);
        cThumbnailCaptureParameters->addField("cameraRotation_1", vec3_type, 40, true);
        cThumbnailCaptureParameters->addField("cameraRotation_2", vec3_type, 52, true);
        cThumbnailCaptureParameters->addField("mouseCameraDataValid", bool_type, 64, true);
        cThumbnailCaptureParameters->addField("mouseCameraOffset", vec3_type, 68, true);
        cThumbnailCaptureParameters->addField("mouseCameraSubjectPosition", vec3_type, 80, true);
        cThumbnailCaptureParameters->addField("mouseCameraTheta", float_type, 92, true);
        cThumbnailCaptureParameters->addField("mouseCameraPhi", float_type, 96, true);
        cThumbnailCaptureParameters->addField("mouseCameraRoll", float_type, 100, true);
        cThumbnailCaptureParameters->addField("poseAnimID", uint32_type, 104, true);
        noun->addNullableField("creatureThumbnailData", catalog.getType("struct:creatureThumbnailData"), 172);

        noun->addField("eliteAssetIds", uint64_type, 176);
        noun->addField("eliteAssetIds", enum_type, 184);
        noun->addField("density", float_type, 188);
        noun->addField("physicsKey", key_type, 204);
        noun->addField("affectsNavMesh", bool_type, 208);
        noun->addField("dynamicWall", bool_type, 209);
        noun->addField("hasLocomotion", bool_type, 219);
        noun->addField("hasLocomotion", enum_type, 220);
        noun->addField("hasNetworkComponent", bool_type, 216);
        noun->addField("hasCombatantComponent", bool_type, 218);
        noun->addField("aiDefinition", asset_type, 218);
        noun->addField("hasCameraComponent", bool_type, 217);
        noun->addField("spawnTeamId", key_type, 224);
        noun->addField("isIslandMarker", bool_type, 217);
        noun->addField("activateFnNamespace", char_ptr_type, 217);

    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Uso: recap_parser <arquivo> [--xml] [-d]" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    bool useXml = false;
    bool debugMode = false;
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--xml") {
            useXml = true;
        }
        else if (arg == "-d") {
            debugMode = true;
        }
    }
    Catalog catalog;
    setupCatalog(catalog);
    Parser parser(catalog);
    if (useXml) {
        parser.setOutputMode(OutputMode::XML);
    }
    else if (debugMode) {
        parser.setOutputMode(OutputMode::DEBUG_LOG);
    }
    if (!parser.parseFile(filename)) {
        return 1;
    }

    return 0;
}