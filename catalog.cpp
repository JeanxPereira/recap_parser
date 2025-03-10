#define _CRT_SECURE_NO_WARNINGS
#include "catalog.h"
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <filesystem>

TypeDatabase TypeDatabase::sInstance;

FileStream::FileStream(const std::string& filename, const char* mode)
    : mFile(std::fopen(filename.c_str(), mode)) {
}

FileStream::~FileStream() {
    if (mFile) {
        std::fclose(mFile);
    }
}

FileStream::operator bool() const {
    return mFile != nullptr;
}

bool FileStream::eof() const {
    return std::feof(mFile) != 0;
}

void FileStream::skip(size_t bytes) {
    std::fseek(mFile, static_cast<long>(bytes), SEEK_CUR);
}

void FileStream::seek(size_t position, int origin) {
    std::fseek(mFile, static_cast<long>(position), origin);
}

template<>
uint32_t FileStream::read() {
    uint32_t value;
    std::fread(&value, sizeof(value), 1, mFile);
    return value;
}

template<>
int32_t FileStream::read() {
    int32_t value;
    std::fread(&value, sizeof(value), 1, mFile);
    return value;
}

template<>
int64_t FileStream::read() {
    int64_t value;
    std::fread(&value, sizeof(value), 1, mFile);
    return value;
}

template<>
float FileStream::read() {
    float value;
    std::fread(&value, sizeof(value), 1, mFile);
    return value;
}

template<>
double FileStream::read() {
    double value;
    std::fread(&value, sizeof(value), 1, mFile);
    return value;
}

std::string FileStream::readString() {
    char buffer[1024];
    size_t i = 0;

    do {
        buffer[i] = std::fgetc(mFile);
    } while (buffer[i++] != '\0' && i < sizeof(buffer) - 1);

    buffer[i] = '\0';
    return std::string(buffer);
}

void CatalogItem::read(FileStream& stream) {
    if (stream.eof()) {
        return;
    }

    stream.skip(8); // Skip magic number and header data

    uint32_t assetNameOffset = stream.read<uint32_t>();
    compileTime = stream.read<int64_t>();
    dataCrc = stream.read<uint32_t>();
    typeCrc = stream.read<uint32_t>();
    uint32_t sourceFileOffset = stream.read<uint32_t>();
    version = stream.read<int32_t>();
    uint32_t tagsOffset = stream.read<uint32_t>();
    uint32_t tagsCount = stream.read<uint32_t>();

    // We'll need to read the strings later, after we've processed all the fixed-size data
}

void CatalogItem::write(FileStream& stream) const {
    // Implement writing functionality
}

bool Catalog::read(const std::string& filename) {
    FileStream stream(filename, "rb");
    if (!stream) {
        return false;
    }

    uint32_t magicNumber = stream.read<uint32_t>();
    uint32_t items = stream.read<uint32_t>();

    mItems.reserve(items);

    stream.seek(0, SEEK_SET);
    for (uint32_t id = 0; id < items; ++id) {
        auto& item = mItems.emplace_back();
        item.read(stream);
    }

    stream.seek((items * 0x28) + 8, SEEK_SET);
    for (auto& item : mItems) {
        // Read strings and other dynamic data here
        // This would depend on the exact file format
    }

    return true;
}

void Catalog::write(const std::string& filename) const {
    // Implement writing functionality
}

Type::Type(std::string name, uint32_t size)
    : mName(std::move(name)), mSize(size) {
}

TypeDatabase::TypeDatabase() {
    // Constructor
}

TypeDatabase& TypeDatabase::instance() {
    return sInstance;
}

std::shared_ptr<Type> TypeDatabase::add(std::string name, uint32_t size) {
    auto type = std::make_shared<Type>(std::move(name), size);
    mTypes[type->getName()] = type;
    mTypesByHash[HashFunction(type->getName().c_str(), -2128831035, 1)] = type;
    return type;
}

std::shared_ptr<Type> TypeDatabase::addStruct(std::string name) {
    return add(std::move(name), 0); // Struct size will be calculated based on fields
}

std::shared_ptr<Type> TypeDatabase::getType(const std::string& name) {
    auto it = mTypes.find(name);
    return (it != mTypes.end()) ? it->second : nullptr;
}

std::shared_ptr<Type> TypeDatabase::getTypeByHash(uint32_t hash) {
    auto it = mTypesByHash.find(hash);
    return (it != mTypesByHash.end()) ? it->second : nullptr;
}

void TypeDatabase::initialize() {
    // Define basic types
    auto int_type = add("int", sizeof(int32_t));
    auto int64_t_type = add("int64_t", sizeof(int64_t));
    auto uint32_t_type = add("uint32_t", sizeof(uint32_t));
    auto uint64_t_type = add("uint64_t", sizeof(uint64_t));
    auto bool_type = add("bool", sizeof(bool));

    // Define floating point types
    auto float_type = add("float", sizeof(float));
    auto double_type = add("double", sizeof(double));
    auto vec3_type = add("vec3", sizeof(vec3));

    // Define special types
    auto char_ptr_type = add("char*", sizeof(char*));
    auto asset_type = add("asset", sizeof(uint32_t));
    auto nullable_type = add("nullable", 4);
    auto key_type = add("key", 4);
    auto array_type = add("array", 4);
    auto enum_type = add("enum", 4);

    add("cLocalizedAssetString", 4);

    //  CatalogEntry
    {
        auto catalogEntry = addStruct("CatalogEntry");
        catalogEntry->addField("assetNameWType", char_ptr_type, 0);
        catalogEntry->addField("compileTime", int64_t_type, 8);
        catalogEntry->addField("dataCrc", uint32_t_type, 16);
        catalogEntry->addField("typeCrc", uint32_t_type, 20);
        catalogEntry->addField("sourceFileNameWType", char_ptr_type, 24);
        catalogEntry->addField("version", int_type, 28);
        catalogEntry->addField("tags", array_type, 32);
    }

    //  Catalog
    {
        auto catalog = addStruct("Catalog");
        catalog->addField("entries", array_type, 0);
    }

    //  Noun
    {
        auto noun = addStruct("Noun");
        noun->addField("nounType", enum_type, 0);
        noun->addField("clientOnly", bool_type, 4);
        noun->addField("isFixed", bool_type, 5);
        noun->addField("isSelfPowered", bool_type, 6);
        noun->addField("lifetime", float_type, 12);
        noun->addField("gfxPickMethod", enum_type, 8);
        noun->addField("graphicsScale", float_type, 20);
        noun->addField("modelKey", key_type, 36);

        auto bbox = addStruct("cSPBoundingBox");
        bbox->addField("min", vec3_type, 56);
        bbox->addField("max", vec3_type, 68);

        noun->addField("cSPBoundingBox", bbox, 24);
        noun->addField("presetExtents", enum_type, 80);
        noun->addField("voice", key_type, 96);
        noun->addField("foot", key_type, 112);
        noun->addField("flightSound", key_type, 128);
        noun->addField("assetId", uint64_t_type, 152);
        noun->addField("playerClassData", asset_type, 164);
        noun->addField("characterAnimationData", asset_type, 168);
        noun->addField("creatureThumbnailData", nullable_type, 172);

        auto creatureThumbnailData = addStruct("creatureThumbnailData");
        creatureThumbnailData->addField("fovY", vec3_type, true);
        creatureThumbnailData->addField("nearPlane", vec3_type, true);
        creatureThumbnailData->addField("farPlane", vec3_type, true);

        noun->addField("creatureThumbnailData", creatureThumbnailData, true);

        noun->addField("physicsType", enum_type, 184);
        noun->addField("density", float_type, 188);
        noun->addField("physicsKey", key_type, 204);
        noun->addField("affectsNavMesh", bool_type, 208);
        noun->addField("dynamicWall", bool_type, 209);
        noun->addField("hasLocomotion", bool_type, 219);
        noun->addField("locomotionType", enum_type, 220);
        noun->addField("hasNetworkComponent", bool_type, 216);
        noun->addField("hasCombatantComponent", bool_type, 218);
        noun->addField("hasCameraComponent", bool_type, 217);
        noun->addField("spawnTeamId", enum_type, 224);
        noun->addField("isIslandMarker", bool_type, 228);
        noun->addField("locomotionTuning", nullable_type, 304);

        auto shared = addStruct("SharedComponentData");

        noun->addField("toonType", key_type, 324);
        noun->addField("isFlora", bool_type, 328);
        noun->addField("isMineral", bool_type, 329);
        noun->addField("isCreature", bool_type, 330);
        noun->addField("isPlayer", bool_type, 331);
        noun->addField("isSpawned", bool_type, 332);
        noun->addField("modelEffect", key_type, 348);
        noun->addField("removalEffect", key_type, 364);
        noun->addField("meleeDeathEffect", key_type, 396);
        noun->addField("meleeCritEffect", key_type, 412);
        noun->addField("energyDeathEffect", key_type, 428);
        noun->addField("energyCritEffect", key_type, 444);
        noun->addField("plasmaDeathEffect", key_type, 460);
        noun->addField("plasmaCritEffect", key_type, 476);
    }

    //  ClassAttributes
    {
        auto classAttributes = addStruct("ClassAttributes");
        classAttributes->addField("baseHealth", float_type, 0);
        classAttributes->addField("baseMana", float_type, 4);
        classAttributes->addField("baseStrength", float_type, 8);
        classAttributes->addField("baseDexterity", float_type, 12);
        classAttributes->addField("baseMind", float_type, 16);
        classAttributes->addField("basePhysicalDefense", float_type, 20);
        classAttributes->addField("baseMagicalDefense", float_type, 24);
        classAttributes->addField("baseEnergyDefense", float_type, 28);
        classAttributes->addField("baseCritical", float_type, 32);
        classAttributes->addField("baseCombatSpeed", float_type, 36);
        classAttributes->addField("baseNonCombatSpeed", float_type, 40);
        classAttributes->addField("baseStealthDetection", float_type, 44);
        classAttributes->addField("baseMovementSpeedBuff", float_type, 48);
        classAttributes->addField("maxHealth", float_type, 52);
        classAttributes->addField("maxMana", float_type, 56);
        classAttributes->addField("maxStrength", float_type, 60);
        classAttributes->addField("maxDexterity", float_type, 64);
        classAttributes->addField("maxMind", float_type, 68);
        classAttributes->addField("maxPhysicalDefense", float_type, 72);
        classAttributes->addField("maxMagicalDefense", float_type, 76);
        classAttributes->addField("maxEnergyDefense", float_type, 80);
        classAttributes->addField("maxCritical", float_type, 84);
    }
    //  PlayerClass
    {
        auto playerClass = addStruct("PlayerClass");
        playerClass->addField("testingOnly", bool_type, 0);
        playerClass->addField("speciesName", char_ptr_type, 16);
        playerClass->addField("nameLocaleKey", key_type, 32);
        playerClass->addField("shortNameLocaleKey", key_type, 48);
        playerClass->addField("creatureType", enum_type, 4);
        playerClass->addField("localeTableID", key_type, 64);
        playerClass->addField("homeworld", enum_type, 68);
        playerClass->addField("creatureClass", enum_type, 72);
        playerClass->addField("primaryAttribute", enum_type, 76);
        playerClass->addField("unlockLevel", enum_type, 80);
        playerClass->addField("basicAbility", key_type, 80);
        playerClass->addField("basicAbility", key_type, 80);
        playerClass->addField("specialAbility1", key_type, 112);
        playerClass->addField("specialAbility2", key_type, 128);
        playerClass->addField("specialAbility3", key_type, 144);
        playerClass->addField("passiveAbility", key_type, 160);

        auto vec3 = addStruct("cSPVector3");
        vec3->addField("cSPVector3", vec3_type, 172);

        playerClass->addField("sharedAbilityOffset", vec3, 172);
        playerClass->addField("mpClassAttributes", asset_type, 12);
        playerClass->addField("mpClassEffect", asset_type, 8);
        playerClass->addField("originalHandBlock", key_type, 196);
        playerClass->addField("originalFootBlock", key_type, 212);
        playerClass->addField("originalWeaponBlock", key_type, 228);
        playerClass->addField("weaponMinDamage", float_type, 232);
        playerClass->addField("weaponMaxDamage", float_type, 236);
        playerClass->addField("noHands", bool_type, 248);
        playerClass->addField("noFeet", bool_type, 249);
        playerClass->addField("descriptionTag", array_type, 164);
        playerClass->addField("editableCharacterPart", array_type, 240);
    }
}

uint32_t HashFunction(const char* str, uint32_t seed, uint32_t len) {
    if (!str) return 0;

    uint32_t hash = seed;
    while (*str && (len == 0 || len--)) {
        hash = ((hash << 5) + hash) + *str++;
    }
    return hash;
}