#include "catalog.h"

namespace parser {

    void StructDefinition::addField(const std::string& name,
        std::shared_ptr<DataType> type,
        std::shared_ptr<DataType> containedType,
        size_t offset,
        bool usesSecondaryPointer) {
        if (type->getName() == "nullable") {
            auto nullableType = parser::Catalog::getInstance().addNullableType(containedType);
            fields.push_back(std::make_shared<Field>(name, nullableType, offset, usesSecondaryPointer));
        } else if (type->getName() == "dNullable") {
            auto dnullableType = parser::Catalog::getInstance().addDirectNullableType(containedType);
            fields.push_back(std::make_shared<Field>(name, dnullableType, offset, usesSecondaryPointer));
        }
        else {
            fields.push_back(std::make_shared<Field>(name, type, offset, usesSecondaryPointer));
        }
    }

    void StructDefinition::addStructField(const std::string& name, const std::string& structName, size_t offset, bool usesSecondaryPointer) {
        auto& catalog = Catalog::getInstance();
        auto structType = std::dynamic_pointer_cast<StructType>(catalog.getType("struct:" + structName));
        if (!structType) {
            structType = std::dynamic_pointer_cast<StructType>(catalog.addStructType(structName));
        }
        addField(name, structType, offset, usesSecondaryPointer);
    }

    void initializeCatalog() {
        auto& catalog = Catalog::getInstance();

        // Register basic types
        auto bool_type = catalog.addType("bool", sizeof(bool));
        auto float_type = catalog.addType("float", sizeof(float));
        auto vec3_type = catalog.addType("vec3", sizeof(float) * 3);
        auto vec2_type = catalog.addType("vec2", sizeof(float) * 2);
        auto char_type = catalog.addType("char", sizeof(char));
        auto string_type = catalog.addType("char*", sizeof(char*));
        auto int_type = catalog.addType("int", sizeof(int));
        auto uint32_type = catalog.addType("uint32_t", sizeof(uint32_t));
        auto uint64_type = catalog.addType("uint64_t", sizeof(uint64_t));
        auto int64_type = catalog.addType("int64_t", sizeof(int64_t));

        // Special types
        auto key_type = catalog.addType("key", 4);
        auto asset_type = catalog.addType("asset", 4);
        auto nullable_type = catalog.addType("nullable", 4);
        auto array_type = catalog.addType("array", 8);  // Size + offset
        auto enum_type = catalog.addType("enum", 4);
        auto dNullable_type = catalog.addType("dNullable", 4);

        //  Phase
        {
            auto phase = catalog.addStruct("Phase");
            phase->addField("gambit", array_type, 0);
            phase->addField("phaseType", enum_type, 4);
            phase->addField("cGambitDefinition", key_type, 52, true);
            phase->addField("startNode", bool_type, 12);
        }

        //  Noun
        {
            auto noun = catalog.addStruct("Noun");
            noun->addField("nounType", array_type, 0);
            noun->addField("clientOnly", bool_type, 4);
            noun->addField("isFixed", bool_type, 5);
            noun->addField("isSelfPowered", bool_type, 6);
            noun->addField("lifetime", float_type, 12);
            noun->addField("gfxPickMethod", float_type, 8);
            noun->addField("graphicsScale", float_type, 20);
            noun->addField("modelKey", key_type, 36, true);

            auto bbox = catalog.addStruct("cSPBoundingBox");
            bbox->addField("min", vec3_type, 0);
            bbox->addField("max", vec3_type, 12);
            noun->addStructField("boundingBox", "cSPBoundingBox", 56);

            noun->addField("presetExtents", enum_type, 80);
            noun->addField("voice", nullable_type, catalog.getType("char*"), 96, true);
            noun->addField("foot", nullable_type, catalog.getType("char*"), 112, true);
            noun->addField("flightSound", nullable_type, catalog.getType("char*"), 128, true);
            noun->addField("gfxStates", nullable_type, catalog.getType("char*"), 132, true);
            noun->addField("doorDef", nullable_type, 136, true);
            noun->addField("switchDef", nullable_type, 140, true);
            noun->addField("pressureSwitchDef", nullable_type, 144, true);
            noun->addField("crystalDef", nullable_type, 148, true);
            noun->addField("assetId", uint64_type, 152);
            noun->addField("npcClassData", nullable_type, catalog.getType("asset"), 160, true);
            noun->addField("playerClassData", nullable_type, catalog.getType("char*"), 164, true);
            noun->addField("characterAnimationData", nullable_type, catalog.getType("char*"), 168, true);

            auto creatureThumbnailData = catalog.addStruct("creatureThumbnailData");
            creatureThumbnailData->addField("fovY", float_type, 0, true);
            creatureThumbnailData->addField("nearPlane", float_type, 4, true);
            creatureThumbnailData->addField("farPlane", float_type, 8, true);
            creatureThumbnailData->addField("cameraPosition", vec3_type, 12, true);
            creatureThumbnailData->addField("cameraScale", float_type, 24, true);
            creatureThumbnailData->addField("cameraRotation_0", vec3_type, 28, true);
            creatureThumbnailData->addField("cameraRotation_1", vec3_type, 40, true);
            creatureThumbnailData->addField("cameraRotation_2", vec3_type, 52, true);
            creatureThumbnailData->addField("mouseCameraDataValid", uint32_type, 64, true);
            creatureThumbnailData->addField("mouseCameraOffset", vec3_type, 68, true);
            creatureThumbnailData->addField("mouseCameraSubjectPosition", vec3_type, 80, true);
            creatureThumbnailData->addField("mouseCameraTheta", float_type, 92, true);
            creatureThumbnailData->addField("mouseCameraPhi", float_type, 96, true);
            creatureThumbnailData->addField("mouseCameraRoll", float_type, 100, true);
            creatureThumbnailData->addField("poseAnimID", uint32_type, 104, true);
            //creatureThumbnailData->addField("mouseCameraSubjectPosition", bool, 32);
            noun->addField("creatureThumbnailData", nullable_type, catalog.getType("struct:creatureThumbnailData"), 172, true);

            noun->addField("eliteAssetIds", nullable_type, catalog.getType("uint64_type"), 176, false);
            noun->addField("physicsType", enum_type, 184);
            noun->addField("density", float_type, 188);
            noun->addField("physicsKey", key_type, 204, true);

            auto locomotionTuning = catalog.addStruct("locomotionTuning");
            locomotionTuning->addField("acceleration", float_type, 0, true);
            locomotionTuning->addField("deceleration", float_type, 4, true);
            locomotionTuning->addField("turnRate", float_type, 8, true);
            noun->addField("locomotionTuning", nullable_type, catalog.getType("struct:locomotionTuning"), 304, true);

            auto sharedComponentData = catalog.addStruct("sharedComponentData");
            sharedComponentData->addField("toonType", key_type, 0, true);
            sharedComponentData->addField("modelEffect", key_type, 0, true);
            sharedComponentData->addField("removalEffect", key_type, 0, true);
            sharedComponentData->addField("meleeDeathEffect", key_type, 0, true);
            sharedComponentData->addField("meleeCritEffect", key_type, 0, true);
            sharedComponentData->addField("energyDeathEffect", key_type, 0, true);
            sharedComponentData->addField("energyCritEffect", key_type, 0, true);
            sharedComponentData->addField("plasmaDeathEffect", key_type, 0, true);
            sharedComponentData->addField("plasmaCritEffect", key_type, 0, true);
            noun->addField("sharedComponentData", nullable_type, catalog.getType("struct:sharedComponentData"), 312, true);
        }

        //  ClassAttributes
        {
            auto classAttributes = catalog.addStruct("ClassAttributes");
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
            auto playerClass = catalog.addStruct("PlayerClass");
            playerClass->addField("testingOnly", bool_type, 0);
            playerClass->addField("creatureClass", enum_type, 72);
            playerClass->addField("primaryAttribute", enum_type, 76);
            playerClass->addField("unlockLevel", enum_type, 80);
            playerClass->addField("basicAbility", key_type, 96, true);
            playerClass->addField("basicAbility", key_type, 96, true);
            playerClass->addField("specialAbility1", key_type, 112, true);
            playerClass->addField("specialAbility2", key_type, 128, true);
            playerClass->addField("specialAbility3", key_type, 144, true);
            playerClass->addField("passiveAbility", key_type, 160, true);
            playerClass->addField("sharedAbilityOffset", vec3_type, 172);
            playerClass->addField("mpClassAttributes", asset_type, 12, true);
            playerClass->addField("mpClassEffect", asset_type, 8, true);
            playerClass->addField("originalHandBlock", key_type, 196, true);
            playerClass->addField("originalFootBlock", key_type, 212, true);
            playerClass->addField("originalWeaponBlock", key_type, 228, true);
            playerClass->addField("weaponMinDamage", float_type, 232);
            playerClass->addField("weaponMaxDamage", float_type, 236);
            playerClass->addField("noHands", bool_type, 248);
            playerClass->addField("noFeet", bool_type, 249);
            playerClass->addField("descriptionTag", key_type, 164, true);
            playerClass->addField("editableCharacterPart", key_type, 240, true);
        }

        //  NonPlayerClass
        {
            auto nonPlayerClass = catalog.addStruct("NonPlayerClass");
            nonPlayerClass->addField("testingOnly", bool_type, 0);
            auto name = catalog.addStruct("name");
            name->addField("str", key_type, 0, true);
            name->addField("id", key_type, 4, true);
            nonPlayerClass->addStructField("name", "name", 16);
            nonPlayerClass->addField("creatureType", enum_type, 4);
            nonPlayerClass->addField("aggroRange", float_type, 56);
            nonPlayerClass->addField("alertRange", float_type, 60);
            nonPlayerClass->addField("dropAggroRange", float_type, 64);
            nonPlayerClass->addField("challengeValue", float_type, 36);
            nonPlayerClass->addField("mNPCType", enum_type, 68);
            nonPlayerClass->addField("npcRank", enum_type, 72);
            nonPlayerClass->addField("mpClassAttributes", key_type, 12, true);
            nonPlayerClass->addField("mpClassEffect", nullable_type, catalog.getType("char*"), 8, true);
            auto description = catalog.addStruct("description");
            description->addField("str", key_type, 0, true);
            description->addField("id", key_type, 4, true);
            nonPlayerClass->addStructField("description", "description", 80);
            nonPlayerClass->addField("dropType", array_type, 40);
            nonPlayerClass->addField("dropDelay", float_type, 52);
            nonPlayerClass->addField("targetable", bool_type, 76);
            nonPlayerClass->addField("playerCountHealthScale", float_type, 100);
            nonPlayerClass->addField("longDescription", array_type, 104);
            nonPlayerClass->addField("eliteAffix", array_type, 112);
            nonPlayerClass->addField("playerPet", bool_type, 120);
        }

        //  CharacterAnimation
        {
            auto characterAnimation = catalog.addStruct("CharacterAnimation");
            characterAnimation->addField("gaitOverlay", uint32_type, 0);
            characterAnimation->addField("overrideGait", uint32_type, 80);
            characterAnimation->addField("ignoreGait", bool_type, 84);
            characterAnimation->addField("morphology", nullable_type, catalog.getType("char*"), 100, true);
            characterAnimation->addField("preAggroIdleAnimState", nullable_type, catalog.getType("char*"), 116, true);
            characterAnimation->addField("idleAnimState", nullable_type, catalog.getType("char*"), 132, true);
            characterAnimation->addField("lobbyIdleAnimState", nullable_type, catalog.getType("char*"), 148, true);
            characterAnimation->addField("specialIdleAnimState", nullable_type, catalog.getType("char*"), 164, true);
            characterAnimation->addField("walkStopState", nullable_type, catalog.getType("char*"), 180, true);
            characterAnimation->addField("victoryIdleAnimState", nullable_type, catalog.getType("char*"), 196, true);
            characterAnimation->addField("combatIdleAnimState", nullable_type, catalog.getType("char*"), 212, true);
            characterAnimation->addField("moveAnimState", nullable_type, catalog.getType("char*"), 228, true);
            characterAnimation->addField("combatMoveAnimState", nullable_type, catalog.getType("char*"), 244, true);
            characterAnimation->addField("deathAnimState", nullable_type, catalog.getType("char*"), 260, true);
            characterAnimation->addField("aggroAnimState", nullable_type, catalog.getType("char*"), 276, true);
            characterAnimation->addField("aggroAnimDuration", float_type, 280);
            characterAnimation->addField("subsequentAggroAnimState", nullable_type, catalog.getType("char*"), 296, true);
            characterAnimation->addField("subsequentAggroAnimDuration", float_type, 300);
            characterAnimation->addField("enterPassiveIdleAnimState", nullable_type, catalog.getType("char*"), 316, true);
            characterAnimation->addField("enterPassiveIdleAnimDuration", float_type, 320);
            characterAnimation->addField("danceEmoteAnimState", nullable_type, catalog.getType("char*"), 336, true);
            characterAnimation->addField("meleeDeathAnimState", nullable_type, catalog.getType("char*"), 352, true);
            characterAnimation->addField("meleeCritDeathAnimState", nullable_type, catalog.getType("char*"), 368, true);
            characterAnimation->addField("meleeCritKnockbackDeathAnimState", nullable_type, catalog.getType("char*"), 384, true);
            characterAnimation->addField("cyberCritDeathAnimState", nullable_type, catalog.getType("char*"), 400, true);
            characterAnimation->addField("cyberCritKnockbackDeathAnimState", nullable_type, catalog.getType("char*"), 416, true);
            characterAnimation->addField("plasmaCritDeathAnimState", nullable_type, catalog.getType("char*"), 432, true);
            characterAnimation->addField("plasmaCritKnockbackDeathAnimState", nullable_type, catalog.getType("char*"), 448, true);
            characterAnimation->addField("bioCritDeathAnimState", nullable_type, catalog.getType("char*"), 464, true);
            characterAnimation->addField("bioCritKnockbackDeathAnimState", nullable_type, catalog.getType("char*"), 480, true);
            characterAnimation->addField("necroCritDeathAnimState", nullable_type, catalog.getType("char*"), 496, true);
            characterAnimation->addField("necroCritKnockbackDeathAnimState", nullable_type, catalog.getType("char*"), 512, true);
            characterAnimation->addField("spacetimeCritDeathAnimState", nullable_type, catalog.getType("char*"), 528, true);
            characterAnimation->addField("spacetimeCritKnockbackDeathAnimState", nullable_type, catalog.getType("char*"), 544, true);
            characterAnimation->addField("bodyFadeAnimState", nullable_type, catalog.getType("char*"), 560, true);
            characterAnimation->addField("randomAbility1AnimState", nullable_type, catalog.getType("char*"), 576, true);
            characterAnimation->addField("randomAbility2AnimState", nullable_type, catalog.getType("char*"), 592, true);
            characterAnimation->addField("randomAbility3AnimState", nullable_type, catalog.getType("char*"), 608, true);
            characterAnimation->addField("overlay1AnimState", nullable_type, catalog.getType("char*"), 624, true);
            characterAnimation->addField("overlay2AnimState", nullable_type, catalog.getType("char*"), 640, true);
            characterAnimation->addField("overlay3AnimState", nullable_type, catalog.getType("char*"), 656, true);
        }

        //  MarkerSet
        {
            auto markerSetHeader = catalog.addStruct("MarkerSetHeader", 40);
            markerSetHeader->addField("version", uint32_type, 0);
            markerSetHeader->addField("entryCount", uint32_type, 4);
        }
        {
            auto decalData = catalog.addStruct("decalData", 268);
            decalData->addField("size", vec3_type, 0, true);
            decalData->addField("material", key_type, 12, true);
            decalData->addField("layer", enum_type, 76, true);
            decalData->addField("diffuse", key_type, 80, true);
            decalData->addField("normal", key_type, 144, true);
            decalData->addField("diffuseTint", vec3_type, 208, true);
            decalData->addField("opacity", float_type, 220, true);
            decalData->addField("specularTint", vec3_type, 224, true);
            decalData->addField("tile", vec2_type, 236, true);
            decalData->addField("normalLevel", float_type, 240, true);
            decalData->addField("glowLevel", float_type, 248, true);
            decalData->addField("emissiveLevel", float_type, 252, true);
            decalData->addField("specExponent", float_type, 256, true);
            decalData->addField("specExponent", float_type, 260, true);
            decalData->addField("enable", bool_type, 264, true);
            decalData->addField("display_volume", bool_type, 265, true);
        }
        {
            auto markerSetEntry = catalog.addStruct("MarkerSetEntry", 200);
            markerSetEntry->addField("markerName", key_type, 0, true);
            markerSetEntry->addField("markerId", uint32_type, 4);
            markerSetEntry->addField("nounDef", key_type, 8, true);
            markerSetEntry->addField("pos", vec3_type, 28);
            markerSetEntry->addField("rotDegrees", vec3_type, 40);
            markerSetEntry->addField("scale", float_type, 52);
            markerSetEntry->addField("dimensions", vec3_type, 56);
            markerSetEntry->addField("visible", bool_type, 68);
            markerSetEntry->addField("castShadows", bool_type, 69);
            markerSetEntry->addField("backFaceShadows", bool_type, 70);
            markerSetEntry->addField("onlyShadows", bool_type, 71);
            markerSetEntry->addField("createWithCollision", bool_type, 72);
            markerSetEntry->addField("debugDisplayKDTree", bool_type, 73);
            markerSetEntry->addField("debugDisplayNormals", bool_type, 74);
            markerSetEntry->addField("navMeshSetting", enum_type, 76);
            markerSetEntry->addField("assetOverrideId", uint64_type, 80);
            markerSetEntry->addField("ignoreOnXBox", bool_type, 160);
            markerSetEntry->addField("ignoreOnMinSpec", bool_type, 161);
            markerSetEntry->addField("ignoreOnPC", bool_type, 162);
            markerSetEntry->addField("highSpecOnly", bool_type, 163);
            markerSetEntry->addField("targetMarkerId", uint32_type, 164);

            markerSetEntry->addField("pointLightData", nullable_type, catalog.getType("uint32_t"), 92, false);
            markerSetEntry->addField("spotLightData", nullable_type, catalog.getType("uint32_t"), 96, false);
            markerSetEntry->addField("lineLightData", nullable_type, catalog.getType("uint32_t"), 100, false);
            markerSetEntry->addField("parallelLightData", nullable_type, catalog.getType("uint32_t"), 104, false);
            markerSetEntry->addField("graphicsData", nullable_type, catalog.getType("uint32_t"), 112, false);

            auto animatedData = catalog.addStruct("animatedData");
            animatedData->addField("animatorData", key_type, 0, true);

            markerSetEntry->addField("animatedData", dNullable_type, catalog.getType("struct:animatedData"), 124, true);

            markerSetEntry->addField("decalData", nullable_type, catalog.getType("struct:decalData"), 128, true);
        }

        catalog.registerFileType(".Phase", { "Phase" }, 68);
        catalog.registerFileType(".Noun", { "Noun" }, 480);
        catalog.registerFileType(".ClassAttributes", { "ClassAttributes" }, 0);
        catalog.registerFileType(".PlayerClass", { "PlayerClass" }, 256);
        catalog.registerFileType(".NonPlayerClass", { "NonPlayerClass" }, 124);
        catalog.registerFileType(".CharacterAnimation", { "CharacterAnimation" }, 660);
        catalog.registerFileType(".MarkerSet", { "MarkerSetHeader" }, 0, true, "MarkerSetHeader", "MarkerSetEntry", 40, 200, 4);
    }

} // namespace parser