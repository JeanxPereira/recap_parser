#include "catalog.h"

Catalog::Catalog() {
    initialize();
}

void Catalog::initialize() {
    // Register basic types
    addType("bool", DataType::BOOL, 1);
    addType("int", DataType::INT, 4);
    addType("int16_t", DataType::INT16, 2);
    addType("int64_t", DataType::INT64, 8);
    addType("uint8_t", DataType::UINT8, 1);
    addType("uint16_t", DataType::UINT16, 2);
    addType("uint32_t", DataType::UINT32, 4);
    addType("uint64_t", DataType::UINT64, 8);
    addType("float", DataType::FLOAT, 4);
    addType("guid", DataType::GUID, 16);
    addType("cSPVector2", DataType::VECTOR2, 8);
    addType("cSPVector3", DataType::VECTOR3, 12);
    addType("cSPVector4", DataType::QUATERNION, 16);
    addType("cLocalizedAssetString", DataType::LOCALIZEDASSETSTRING, 4);
    addType("char", DataType::CHAR, 1);
    addType("char*", DataType::CHAR_PTR, 4);
    addType("key", DataType::KEY, 4);
    addType("asset", DataType::ASSET, 4);
    addType("cKeyAsset", DataType::CKEYASSET, 16);
    addType("nullable", DataType::NULLABLE, 4);
    addType("array", DataType::ARRAY, 4);
    addType("enum", DataType::ENUM, 4);

    // Register filetypes
    registerFileType(".AffixTuning", { "AffixTuning" }, 24);
    registerFileType(".AIDefinition", { "AIDefinition" }, 640);
    registerFileName("catalog_131", { "Catalog" }, 8);
    registerFileType(".ChainLevels", { "ChainLevels" }, 12);
    registerFileType(".CharacterAnimation", { "CharacterAnimation" }, 660);
    registerFileType(".CharacterType", { "CharacterType" }, 0);
    registerFileType(".ClassAttributes", { "ClassAttributes" }, 0);
    registerFileType(".Condition", { "Condition" }, 36);
    registerFileType(".CrystalTuning", { "CrystalTuning" }, 20);
    registerFileType(".DifficultyTuning", { "DifficultyTuning" }, 72);
    registerFileType(".DirectorTuning", { "DirectorTuning" }, 16);
    registerFileType(".EliteNPCGlobals", { "EliteNPCGlobals" }, 24);
    registerFileType(".Level", { "Level" }, 152);
    registerFileType(".LevelConfig", { "LevelConfig" }, 40);
    registerFileType(".LevelObjectives", { "LevelObjectives" }, 40);
    registerFileType(".LootPreferences", { "LootPreferences" }, 0);
    registerFileType(".LootRigblock", { "LootRigblock" }, 140);
    registerFileType(".LootPrefix", { "LootPrefix" }, 564);
    registerFileType(".LootSuffix", { "LootSuffix" }, 584);
    registerFileType(".MagicNumbers", { "MagicNumbers" }, 0);
    registerFileType(".MarkerSet", { "MarkerSet" }, 40);
    registerFileType(".NavPowerTuning", { "NavPowerTuning" }, 12);
    registerFileType(".NonPlayerClass", { "NonPlayerClass" }, 124);
    registerFileType(".Noun", { "Noun" }, 480);
    registerFileType(".NPCAffix", { "NPCAffix" }, 44);
    registerFileType(".ObjectExtents", { "ObjectExtents" }, 0);
    registerFileType(".Phase", { "Phase" }, 68);
    registerFileType(".PlayerClass", { "PlayerClass" }, 256);
    registerFileType(".PVPLevels", { "PVPLevels" }, 8);
    registerFileType(".SectionConfig", { "SectionConfig" }, 8);
    registerFileType(".ServerEventDef", { "ServerEventDef" }, 16);
    registerFileType(".SpaceshipTuning", { "SpaceshipTuning" }, 668);
    registerFileType(".TestAsset", { "TestAsset" }, 0);
    registerFileType(".UnlocksTuning", { "UnlocksTuning" }, 40);
    registerFileType(".WeaponTuning", { "WeaponTuning" }, 8);

    //  Phase
    auto phase = add_struct("Phase");
    phase->add("gambit", "array", 0);
    phase->add("phaseType", "enum", 4);
    phase->add("cGambitDefinition", "key", 52);
    phase->add("startNode", "bool", 12);

    //  Noun
    auto noun = add_struct("Noun");
    noun->add("nounType", "enum", 0);
    noun->add("clientOnly", "bool", 4);
    noun->add("isFixed", "bool", 5);
    noun->add("isSelfPowered", "bool", 6);
    noun->add("lifetime", "float", 12);
    noun->add("gfxPickMethod", "enum", 8);
    noun->add("graphicsScale", "float", 20);
    noun->add("modelKey", "key", 36);
    noun->add("prefab", "key", 16);
    noun->add("levelEditorModelKey", "key", 52);

    auto cSPBoundingBox = add_struct("cSPBoundingBox");
    cSPBoundingBox->add("min", "cSPVector3", 32);
    cSPBoundingBox->add("max", "cSPVector3", 44);
    noun->add(cSPBoundingBox, 24);

    noun->add("presetExtents", "enum", 80);
    noun->add("voice", "key", 96);
    noun->add("foot", "key", 112);
    noun->add("flightSound", "key", 128);

    auto cGameObjectGfxStates = add_struct("gfxStates", 8);
            auto cGameObjectGfxStateData = add_struct("state", 56);
                 cGameObjectGfxStateData->add("name", "key", 12);
                 cGameObjectGfxStateData->add("model", "key", 28);
                 cGameObjectGfxStateData->add("prefab", "asset", 48);
                 cGameObjectGfxStateData->add("animation", "key", 44);
                 cGameObjectGfxStateData->add("animationLoops", "bool", 52);
    cGameObjectGfxStates->addStructArray("state", cGameObjectGfxStateData, 0);
    registerNullableType("gfxStates");

    noun->add("gfxStates", "nullable", cGameObjectGfxStates, 132);

    auto cNewGfxState = add_struct("cNewGfxState", 40);
            cNewGfxState->add("prefab", "asset", 0);
            cNewGfxState->add("model", "key", 16);
            cNewGfxState->add("animation", "key", 32);
    registerNullableType("cNewGfxState");

    auto cDoorDef = add_struct("doorDef", 24);
        cDoorDef->add("graphicsState_open", "nullable", cNewGfxState, 0);
        cDoorDef->add("graphicsState_opening", "nullable", cNewGfxState, 4);
        cDoorDef->add("graphicsState_closed", "nullable", cNewGfxState, 8);
        cDoorDef->add("graphicsState_closing", "nullable", cNewGfxState, 12);
        cDoorDef->add("clickToOpen", "bool", 16);
        cDoorDef->add("clickToClose", "bool", 17);
        cDoorDef->add("initialState", "enum", 20);
    registerNullableType("doorDef");
    noun->add("doorDef", "nullable", cDoorDef, 136);

    auto cSwitchDef = add_struct("switchDef", 12);
         cSwitchDef->add("graphicsState_unpressed", "nullable", cNewGfxState, 0);
         cSwitchDef->add("graphicsState_pressing", "nullable", cNewGfxState, 4);
         cSwitchDef->add("graphicsState_pressed", "nullable", cNewGfxState, 8);
    registerNullableType("switchDef");
    noun->add("switchDef", "nullable", cSwitchDef, 140);

    auto cPressureSwitchDef = add_struct("pressureSwitchDef", 40);
         cPressureSwitchDef->add("graphicsState_unpressed", "nullable", cNewGfxState, 0);
         cPressureSwitchDef->add("graphicsState_pressing", "nullable", cNewGfxState, 4);
         cPressureSwitchDef->add("graphicsState_pressed", "nullable", cNewGfxState, 8);
         auto cVolumeDef = add_struct("cVolumeDef");
              cVolumeDef->add("shape", "enum", 0);
              cVolumeDef->add("boxWidth", "float", 4);
              cVolumeDef->add("boxLength", "float", 8);
              cVolumeDef->add("boxHeight", "float", 12);
              cVolumeDef->add("sphereRadius", "float", 16);
              cVolumeDef->add("capsuleHeight", "float", 20);
              cVolumeDef->add("capsuleRadius", "float", 24);
         cPressureSwitchDef->add(cVolumeDef, 28);
    registerNullableType("pressureSwitchDef");
    noun->add("pressureSwitchDef", "nullable", cPressureSwitchDef, 144);

    auto CrystalDef = add_struct("crystalDef", 24);
         CrystalDef->add("modifier", "key", 0);
         CrystalDef->add("type", "enum", 4);
         CrystalDef->add("rarity", "enum", 16);
    registerNullableType("crystalDef");
    noun->add("crystalDef", "nullable", CrystalDef, 148);

    noun->add("assetId", "uint64_t", 152);
    noun->add("npcClassData", "asset", 160);
    noun->add("playerClassData", "asset", 164);
    noun->add("characterAnimationData", "asset", 168);

    auto cThumbnailCaptureParameters = add_struct("creatureThumbnailData", 108);
         cThumbnailCaptureParameters->add("fovY", "float", 0);
         cThumbnailCaptureParameters->add("nearPlane", "float", 4);
         cThumbnailCaptureParameters->add("farPlane", "float", 8);
         cThumbnailCaptureParameters->add("cameraPosition", "cSPVector3", 12);
         cThumbnailCaptureParameters->add("cameraScale", "float", 24);
         cThumbnailCaptureParameters->add("cameraRotation_0", "cSPVector3", 28);
         cThumbnailCaptureParameters->add("cameraRotation_1", "cSPVector3", 40);
         cThumbnailCaptureParameters->add("cameraRotation_2", "cSPVector3", 52);
         cThumbnailCaptureParameters->add("mouseCameraDataValid", "bool", 64);
         cThumbnailCaptureParameters->add("mouseCameraOffset", "cSPVector3", 68);
         cThumbnailCaptureParameters->add("mouseCameraSubjectPosition", "cSPVector3", 80);
         cThumbnailCaptureParameters->add("mouseCameraTheta", "float", 92);
         cThumbnailCaptureParameters->add("mouseCameraPhi", "float", 96);
         cThumbnailCaptureParameters->add("mouseCameraRoll", "float", 100);
         cThumbnailCaptureParameters->add("poseAnimID", "uint32_t", 104);
    registerNullableType("creatureThumbnailData");
    noun->add("creatureThumbnailData", "nullable", cThumbnailCaptureParameters, 172);

    noun->addArray("eliteAssetIds", "uint64_t", 172);
    noun->add("physicsType", "enum", 184);
    noun->add("density", "float", 188);
    noun->add("physicsKey", "key", 204);
    noun->add("affectsNavMesh", "bool", 208);
    noun->add("dynamicWall", "bool", 209);
    noun->add("hasLocomotion", "bool", 219);
    noun->add("locomotionType", "enum", 220);
    noun->add("hasNetworkComponent", "bool", 216);
    noun->add("hasCombatantComponent", "bool", 218);
    noun->add("aiDefinition", "asset", 212);
    noun->add("hasCameraComponent", "bool", 212);
    noun->add("spawnTeamId", "enum", 224);
    noun->add("isIslandMarker", "bool", 228);
    noun->add("activateFnNamespace", "char*", 232);
    noun->add("tickFnNamespace", "char*", 236);
    noun->add("deactivateFnNamespace", "char*", 240);
    noun->add("startFnNamespace", "char*", 244);
    noun->add("endFnNamespace", "char*", 248);

    auto TriggerVolumeEvents = add_struct("events", 32);
    TriggerVolumeEvents->add("onEnterEvent", "key", 12);
    TriggerVolumeEvents->add("onExitEvent", "key", 28);
    registerNullableType("events");

    auto TriggerVolumeDef = add_struct("triggerVolume", 136);
         TriggerVolumeDef->add("onEnter", "key", 12);
         TriggerVolumeDef->add("onExit", "key", 28);
         TriggerVolumeDef->add("onStay", "key", 44);
         TriggerVolumeDef->add("events", "nullable", TriggerVolumeEvents, 48);
         TriggerVolumeDef->add("useGameObjectDimensions", "bool", 52);
         TriggerVolumeDef->add("isKinematic", "bool", 53);
         TriggerVolumeDef->add("shape", "enum", 56);
         TriggerVolumeDef->add("offset", "cSPVector3", 60);
         TriggerVolumeDef->add("timeToActivate", "float", 72);
         TriggerVolumeDef->add("persistentTimer", "bool", 76);
         TriggerVolumeDef->add("triggerOnceOnly", "bool", 77);
         TriggerVolumeDef->add("triggerIfNotBeaten", "bool", 78);
         TriggerVolumeDef->add("triggerActivationType", "enum", 80);
         TriggerVolumeDef->add("luaCallbackOnEnter", "char*", 84);
         TriggerVolumeDef->add("luaCallbackOnExit", "char*", 88);
         TriggerVolumeDef->add("luaCallbackOnStay", "char*", 92);
         TriggerVolumeDef->add("boxWidth", "float", 96);
         TriggerVolumeDef->add("boxLength", "float", 100);
         TriggerVolumeDef->add("boxHeight", "float", 104);
         TriggerVolumeDef->add("sphereRadius", "float", 108);
         TriggerVolumeDef->add("capsuleHeight", "float", 112);
         TriggerVolumeDef->add("capsuleRadius", "float", 116);
         TriggerVolumeDef->add("serverOnly", "bool", 120);
    registerNullableType("triggerVolume");

    noun->add("gravityData", "asset", 308);

    noun->add("triggerVolume", "nullable", TriggerVolumeDef, 292);

    auto ProjectileDef = add_struct("projectile", 12);
         auto CollisionVolumeDef = add_struct("creatureCollisionVolume", 20);
              CollisionVolumeDef->add("shape", "enum", 0);
              CollisionVolumeDef->add("boxWidth", "float", 4);
              CollisionVolumeDef->add("boxLength", "float", 8);
              CollisionVolumeDef->add("boxHeight", "float", 12);
              CollisionVolumeDef->add("sphereRadius", "float", 16);
         registerNullableType("creatureCollisionVolume");

         ProjectileDef->add("creatureCollisionVolume", "nullable", CollisionVolumeDef, 0);
         ProjectileDef->add("otherCollisionVolume", "nullable", CollisionVolumeDef, 4);
         ProjectileDef->add("targetType", "enum", 8);
    registerNullableType("projectile");
    noun->add("projectile", "nullable", ProjectileDef, 296);

    auto OrbitDef = add_struct("orbit", 12);
         OrbitDef->add("orbitHeight", "float", 0);
         OrbitDef->add("orbitRadius", "float", 4);
         OrbitDef->add("orbitSpeed", "float", 8);
    registerNullableType("orbit");
    noun->add("orbit", "nullable", OrbitDef, 300);

    auto LocomotionTuning = add_struct("locomotionTuning", 12);
         LocomotionTuning->add("acceleration", "float", 0);
         LocomotionTuning->add("deceleration", "float", 4);
         LocomotionTuning->add("turnRate", "float", 8);
    registerNullableType("locomotionTuning");
    noun->add("locomotionTuning", "nullable", LocomotionTuning, 304);

    auto SharedComponentData = add_struct("SharedComponentData", 40);
        auto AudioTriggerDef = add_struct("audioTrigger", 32);
             AudioTriggerDef->add("type", "enum", 0);
             AudioTriggerDef->add("sound", "key", 16);
             AudioTriggerDef->add("is3D", "bool", 20);
             AudioTriggerDef->add("retrigger", "bool", 21);
             AudioTriggerDef->add("hardStop", "bool", 22);
             AudioTriggerDef->add("hardStop", "bool", 22);
             AudioTriggerDef->add("isVoiceover", "bool", 23);
             AudioTriggerDef->add("voiceLifetime", "float", 24);
             AudioTriggerDef->add("triggerVolume", "nullable", TriggerVolumeDef, 28);
        registerNullableType("audioTrigger");

        auto TeleporterDef = add_struct("teleporter", 12);
             TeleporterDef->add("destinationMarkerId", "uint32_t", 0);
             TeleporterDef->add("triggerVolume", "nullable", TriggerVolumeDef, 4);
             TeleporterDef->add("deferTriggerCreation", "bool", 8);
        registerNullableType("teleporter");

        auto EventListenerDef = add_struct("eventListenerDef", 8);
             auto EventListenerData = add_struct("listener", 40);
                  EventListenerData->add("event", "key", 0);
                  EventListenerData->add("callback", "key", 28);
                  EventListenerData->add("luaCallback", "char*", 36);
             EventListenerDef->addStructArray("listener", EventListenerData, 0);
        registerNullableType("eventListenerDef");

        auto SpawnPointDef = add_struct("spawnPointDef", 8);
        SpawnPointDef->add("sectionType", "enum", 0);
        SpawnPointDef->add("activatesSpike", "bool", 4);
        registerNullableType("spawnPointDef");

        auto SpawnTriggerDef = add_struct("spawnTrigger",   28);
             SpawnTriggerDef->add("triggerVolume", "nullable", TriggerVolumeDef, 0);
             SpawnTriggerDef->add("deathEvent", "key", 16);
             SpawnTriggerDef->add("challengeOverride", "uint32_t", 20); //??? - off_FDAB24
             SpawnTriggerDef->add("waveOverride", "uint32_t", 24); //??? - off_FDAB24
        registerNullableType("spawnTrigger");
        
        auto InteractableDef = add_struct("interactable", 72);
             InteractableDef->add("numUsesAllowed", "uint32_t", 0); //??? - off_FDAB24
             InteractableDef->add("interactableAbility", "key", 16);
             InteractableDef->add("startInteractEvent", "key", 32);
             InteractableDef->add("endInteractEvent", "key", 48);
             InteractableDef->add("optionalInteractEvent", "key", 64);
             InteractableDef->add("challengeValue", "uint32_t", 68); //??? - off_FDAB24
        registerNullableType("interactable");
        
        auto GameObjectGfxStateTuning = add_struct("defaultGfxState", 24);
             GameObjectGfxStateTuning->add("name", "key", 12);
             GameObjectGfxStateTuning->add("animationStartTime", "float", 16);
             GameObjectGfxStateTuning->add("animationRate", "float", 20);
        registerNullableType("defaultGfxState");
        
        auto CombatantDef = add_struct("combatant", 16);
             CombatantDef->add("deathEvent", "key", 12);
        registerNullableType("combatant");
        
        auto TriggerVolumeComponentDef = add_struct("triggerComponent", 4);
             TriggerVolumeComponentDef->add("triggerVolume", "nullable", TriggerVolumeDef, 0);
        registerNullableType("triggerComponent");
        
        auto SpaceshipSpawnPointDef = add_struct("spaceshipSpawnPoint", 4);
             SpaceshipSpawnPointDef->add("index", "uint32_t", 0); //??? - off_FDAB24
        registerNullableType("spaceshipSpawnPoint");

        SharedComponentData->add("audioTrigger", "nullable", AudioTriggerDef, 0);
        SharedComponentData->add("teleporter", "nullable", TeleporterDef, 4);
        SharedComponentData->add("eventListenerDef", "nullable", EventListenerDef, 8);
        SharedComponentData->add("spawnPointDef", "nullable", SpawnPointDef, 16);
        SharedComponentData->add("spawnTrigger", "nullable", SpawnTriggerDef, 12);
        SharedComponentData->add("interactable", "nullable", InteractableDef, 20);
        SharedComponentData->add("defaultGfxState", "nullable", GameObjectGfxStateTuning, 24);
        SharedComponentData->add("combatant", "nullable", CombatantDef, 28);
        SharedComponentData->add("triggerComponent", "nullable", TriggerVolumeComponentDef, 32);
        SharedComponentData->add("spaceshipSpawnPoint", "nullable", SpaceshipSpawnPointDef, 36);
    noun->add(SharedComponentData, 252);

    noun->add("isFlora", "bool", 328);
    noun->add("isMineral", "bool", 329);
    noun->add("isCreature", "bool", 330);
    noun->add("isPlayer", "bool", 331);
    noun->add("isSpawned", "bool", 332);

    noun->add("toonType", "key", 324);
    noun->add("modelEffect", "key", 348);
    noun->add("removalEffect", "key", 364);
    noun->add("meleeDeathEffect", "key", 396);
    noun->add("meleeCritEffect", "key", 412);
    noun->add("energyDeathEffect", "key", 428);
    noun->add("energyCritEffect", "key", 444);
    noun->add("plasmaDeathEffect", "key", 460);
    noun->add("plasmaCritEffect", "key", 476);

    //  NonPlayerClass
    auto nonPlayerClass = add_struct("NonPlayerClass");
    nonPlayerClass->add("testingOnly", "bool", 0);
    nonPlayerClass->add("name", "cLocalizedAssetString", 16);
    nonPlayerClass->add("creatureType", "enum", 4);
    nonPlayerClass->add("aggroRange", "float", 56);
    nonPlayerClass->add("alertRange", "float", 60);
    nonPlayerClass->add("dropAggroRange", "float", 64);
    nonPlayerClass->add("mNPCType", "enum", 68);
    nonPlayerClass->add("npcRank", "uint32_t", 68); //??? - off_FDAB24
    nonPlayerClass->add("mpClassAttributes", "asset", 12);
    nonPlayerClass->add("mpClassEffect", "asset", 8);
    nonPlayerClass->add("description", "cLocalizedAssetString", 80);
    nonPlayerClass->addArray("dropType", "enum", 40);
    nonPlayerClass->add("dropDelay", "float", 52);
    nonPlayerClass->add("targetable", "bool", 76);
    nonPlayerClass->add("playerCountHealthScale", "float", 100);

    auto cLongDescription = add_struct("cLongDescription", 20);
         cLongDescription->add("description", "cLocalizedAssetString", 0);
    nonPlayerClass->addStructArray("cLongDescription", cLongDescription, 104);

    auto cEliteAffix = add_struct("cEliteAffix", 12);
    cEliteAffix->add("mpNPCAffix", "asset", 0);
    cEliteAffix->add("minDifficulty", "uint32_t", 4);
    cEliteAffix->add("maxDifficulty", "uint32_t", 8);
    nonPlayerClass->addStructArray("eliteAffix", cEliteAffix, 112);

    nonPlayerClass->add("playerPet", "bool", 120);

    //  PlayerClass
    auto PlayerClass = add_struct("PlayerClass");
         PlayerClass->add("testingOnly", "bool", 0);
         PlayerClass->add("speciesName", "char*", 16);
         PlayerClass->add("nameLocaleKey", "key", 32);
         PlayerClass->add("shortNameLocaleKey", "key", 48);
         PlayerClass->add("creatureType", "enum", 4);
         PlayerClass->add("localeTableID", "key", 64);
         PlayerClass->add("homeworld", "enum", 68);
         PlayerClass->add("creatureClass", "enum", 72);
         PlayerClass->add("primaryAttribute", "enum", 76);
         PlayerClass->add("unlockLevel", "uint32_t", 80);
         PlayerClass->add("basicAbility", "uint32_t", 96);
         PlayerClass->add("specialAbility1", "key", 112);
         PlayerClass->add("specialAbility2", "key", 128);
         PlayerClass->add("specialAbility3", "key", 144);
         PlayerClass->add("passiveAbility", "key", 160);
         PlayerClass->add("sharedAbilityOffset", "cSPVector3", 172);
         PlayerClass->add("sharedAbilityOffset", "key", 172);
         PlayerClass->add("mpClassAttributes", "asset", 12);
         PlayerClass->add("mpClassEffect", "asset", 8);
         PlayerClass->add("originalHandBlock", "key", 196);
         PlayerClass->add("originalFootBlock", "key", 212);
         PlayerClass->add("originalWeaponBlock", "key", 228);
         PlayerClass->add("weaponMinDamage", "float", 232);
         PlayerClass->add("weaponMaxDamage", "float", 236);
         PlayerClass->add("noHands", "bool", 248);
         PlayerClass->add("noFeet", "bool", 249);
         PlayerClass->addArray("descriptionTag", "enum", 164);
         PlayerClass->addArray("editableCharacterPart", "cKeyAsset", 240);

    //   CharacterAnimation
    auto characterAnimation = add_struct("CharacterAnimation");
    characterAnimation->add("gaitOverlay", "uint32_t", 80);
    characterAnimation->add("overrideGait", "char", 0);
    characterAnimation->add("ignoreGait", "bool", 84);
    characterAnimation->add("morphology", "key", 100);
    characterAnimation->add("preAggroIdleAnimState", "key", 116);
    characterAnimation->add("idleAnimState", "key", 132);
    characterAnimation->add("lobbyIdleAnimState", "key", 148);
    characterAnimation->add("specialIdleAnimState", "key", 164);
    characterAnimation->add("walkStopState", "key", 180);
    characterAnimation->add("victoryIdleAnimState", "key", 196);
    characterAnimation->add("combatIdleAnimState", "key", 212);
    characterAnimation->add("moveAnimState", "key", 228);
    characterAnimation->add("combatMoveAnimState", "key", 244);
    characterAnimation->add("deathAnimState", "key", 260);
    characterAnimation->add("aggroAnimState", "key", 276);
    characterAnimation->add("aggroAnimDuration", "float", 280);
    characterAnimation->add("subsequentAggroAnimState", "key", 296);
    characterAnimation->add("subsequentAggroAnimDuration", "float", 300);
    characterAnimation->add("enterPassiveIdleAnimState", "key", 316);
    characterAnimation->add("enterPassiveIdleAnimDuration", "float", 320);
    characterAnimation->add("danceEmoteAnimState", "key", 336);
    characterAnimation->add("meleeDeathAnimState", "key", 352);
    characterAnimation->add("meleeCritDeathAnimState", "key", 368);
    characterAnimation->add("meleeCritKnockbackDeathAnimState", "key", 384);
    characterAnimation->add("cyberCritDeathAnimState", "key", 400);
    characterAnimation->add("cyberCritKnockbackDeathAnimState", "key", 416);
    characterAnimation->add("plasmaCritDeathAnimState", "key", 432);
    characterAnimation->add("plasmaCritKnockbackDeathAnimState", "key", 448);
    characterAnimation->add("bioCritDeathAnimState", "key", 464);
    characterAnimation->add("bioCritKnockbackDeathAnimState", "key", 480);
    characterAnimation->add("necroCritDeathAnimState", "key", 496);
    characterAnimation->add("necroCritKnockbackDeathAnimState", "key", 512);
    characterAnimation->add("spacetimeCritDeathAnimState", "key", 528);
    characterAnimation->add("spacetimeCritKnockbackDeathAnimState", "key", 544);
    characterAnimation->add("bodyFadeAnimState", "key", 560);
    characterAnimation->add("randomAbility1AnimState", "key", 576);
    characterAnimation->add("randomAbility2AnimState", "key", 592);
    characterAnimation->add("randomAbility3AnimState", "key", 608);
    characterAnimation->add("overlay1AnimState", "key", 624);
    characterAnimation->add("overlay2AnimState", "key", 640);
    characterAnimation->add("overlay3AnimState", "key", 656);

    //   NPCAffix
    auto NPCAffix = add_struct("NPCAffix");
         NPCAffix->add("modifier", "key", 12);
         NPCAffix->add("mpChildAffix", "key", 16);
         NPCAffix->add("mpParentAffix", "key", 20);
         NPCAffix->add("description", "cLocalizedAssetString", 24);

    //   ClassAttributes
    auto classAttributes = add_struct("ClassAttributes");
         classAttributes->add("baseHealth", "float", 0);
         classAttributes->add("baseMana", "float", 4);
         classAttributes->add("baseStrength", "float", 8);
         classAttributes->add("baseDexterity", "float", 12);
         classAttributes->add("baseMind", "float", 16);
         classAttributes->add("basePhysicalDefense", "float", 20);
         classAttributes->add("baseMagicalDefense", "float", 24);
         classAttributes->add("baseEnergyDefense", "float", 28);
         classAttributes->add("baseCritical", "float", 32);
         classAttributes->add("baseCombatSpeed", "float", 36);
         classAttributes->add("baseNonCombatSpeed", "float", 40);
         classAttributes->add("baseStealthDetection", "float", 44);
         classAttributes->add("baseMovementSpeedBuff", "float", 48);
         classAttributes->add("maxHealth", "float", 52);
         classAttributes->add("maxMana", "float", 56);
         classAttributes->add("maxStrength", "float", 60);
         classAttributes->add("maxDexterity", "float", 64);
         classAttributes->add("maxMind", "float", 68);
         classAttributes->add("maxPhysicalDefense", "float", 72);
         classAttributes->add("maxMagicalDefense", "float", 76);
         classAttributes->add("maxEnergyDefense", "float", 80);
         classAttributes->add("maxCritical", "float", 84);

         
    //   AIDefinition
    auto AIDefinition = add_struct("AIDefinition");
         auto cAINode = add_struct("cAINode", 28);
             cAINode->add("mpPhaseData", "key", 0);
             cAINode->add("mpConditionData", "key", 4);
             cAINode->add("nodeX", "uint32_t", 12);
             cAINode->add("nodeY", "uint32_t", 16);
             cAINode->add("output", "uint32_t", 20); //array?
         AIDefinition->addStructArray("cAINode", cAINode, 0);

         AIDefinition->add("deathAbility", "key", 20);
         AIDefinition->add("deathCondition", "key", 36);
         AIDefinition->add("firstAggroAbility", "key", 52);
         AIDefinition->add("firstAggroAbility2", "key", 68);
         AIDefinition->add("firstAlertAbility", "key", 84);
         AIDefinition->add("subsequentAggroAbility", "key", 100);
         AIDefinition->add("passiveAbility", "key", 116);
         AIDefinition->add("aggroType", "uint32_t", 116);
         AIDefinition->add("combatIdle", "char", 284);
         AIDefinition->add("combatIdleCooldown", "uint32_t", 364);
         AIDefinition->add("combatIdle2", "char", 448);
         AIDefinition->add("combatIdle2Condition", "key", 540);
         AIDefinition->add("combatIdle2Cooldown", "uint32_t", 544);
         AIDefinition->add("passiveIdle", "char", 368);
         AIDefinition->add("preAggroIdle", "char", 124);
         AIDefinition->add("preAggroIdle2", "char", 204);
         AIDefinition->add("useSecondaryStart", "float", 636);
         AIDefinition->add("targetTooFar", "char", 548);
         AIDefinition->add("targetTooFarCooldown", "char", 628);
         AIDefinition->add("faceTarget", "bool", 632);
         AIDefinition->add("alwaysRunAI", "bool", 633);
         AIDefinition->add("randomizeCooldowns", "bool", 634);

	auto LootData = add_struct("LootData", 4);
		LootData->add("name", "char*", 0);

    //   LootPrefix
	auto LootPrefix = add_struct("LootPrefix");
		LootPrefix->add("prefixId", "uint32_t", 0);
        LootPrefix->addStructArray("partTypes", LootData, 4);
        LootPrefix->addStructArray("classTypes", LootData, 12);
        LootPrefix->addStructArray("scienceTypes", LootData, 20);
        LootPrefix->addStructArray("modifierGranted", LootData, 496);
        LootPrefix->addStructArray("abilityImproved", LootData, 504);
        LootPrefix->add("minLevel", "uint32_t", 28); //??? - off_FDAB24
        LootPrefix->add("maxLevel", "uint32_t", 32); //??? - off_FDAB24
        LootPrefix->add("strength", "float", 36);
        LootPrefix->add("dexterity", "float", 40);
        LootPrefix->add("mind", "float", 44);
        LootPrefix->add("health", "float", 52);
        LootPrefix->add("mana", "float", 56);
        LootPrefix->add("critical", "float", 76);
        LootPrefix->add("criticalDamage", "float", 124);
        LootPrefix->add("physicalDefense", "float", 64);
        LootPrefix->add("energyDefense", "float", 72);
        LootPrefix->add("projectileSpeed", "float", 140);
        LootPrefix->add("cooldown", "float", 132);
        LootPrefix->add("aoeDamage", "float", 184);
        LootPrefix->add("aoeResistance%", "float", 144);
        LootPrefix->add("movementSpeed", "float", 228);
        LootPrefix->add("lifeSteal", "float", 176);
        LootPrefix->add("manaSteal", "float", 244);
        LootPrefix->add("attackSpeed", "float", 128);
        LootPrefix->add("damageLifeType", "float", 196);
        LootPrefix->add("damageTechType", "float", 188);
        LootPrefix->add("damageQuantumType", "float", 192);
        LootPrefix->add("damageNecroType", "float", 204);
        LootPrefix->add("damagePlasmaType", "float", 200);
        LootPrefix->add("resistLifeType", "float", 216);
        LootPrefix->add("resistTechType", "float", 208);
        LootPrefix->add("resistQuantumType", "float", 212);
        LootPrefix->add("resistNecroType", "float", 224);
        LootPrefix->add("resistPlasmaType", "float", 220);
        LootPrefix->add("aoeRadius", "float", 284);
        LootPrefix->add("petDamage", "float", 288);
        LootPrefix->add("petHealth", "float", 292);
        LootPrefix->add("crystalFind%", "float", 296);
        LootPrefix->add("lootFind%", "float", 320);
        LootPrefix->add("dnaDropped%", "float", 300);
        LootPrefix->add("rangeIncrease", "float", 304);
        LootPrefix->add("orbEffect", "float", 308);
        LootPrefix->add("overdriveBuildup%", "float", 312);
        LootPrefix->add("overdriveDuration%", "float", 316);
        LootPrefix->add("surefooted%", "float", 324);
        LootPrefix->add("buffDuration%", "float", 326);
        LootPrefix->add("debuffShorten%", "float", 240);
        LootPrefix->add("debuffLengthen%", "float", 248);
        LootPrefix->add("dotDamage%", "float", 376);
        LootPrefix->add("dotDamage%", "float", 376);
        LootPrefix->add("aggroInc%", "float", 380);
        LootPrefix->add("aggroDec%", "float", 384);
        LootPrefix->add("physDmg%", "float", 388);
        LootPrefix->add("energyDmg%", "float", 396);
        LootPrefix->add("energyAbDmg%", "float", 400);
        LootPrefix->add("immStun", "float", 328);
        LootPrefix->add("immBanish", "float", 276);
        LootPrefix->add("immKnockBack", "float", 280);
        LootPrefix->add("immSleep", "float", 336);
        LootPrefix->add("immTaunt", "float", 340);
        LootPrefix->add("immTerror", "float", 344);
        LootPrefix->add("immSilence", "float", 348);
        LootPrefix->add("immCurse", "float", 352);
        LootPrefix->add("immPoison", "float", 356);
        LootPrefix->add("immBurn", "float", 360);
        LootPrefix->add("immRoot", "float", 364);
        LootPrefix->add("immSlow", "float", 368);
        LootPrefix->add("immPull", "float", 372);
        LootPrefix->add("channelTime%", "float", 404);
        LootPrefix->add("ccDur%", "float", 408);
        LootPrefix->add("dotDur%", "float", 412);
        LootPrefix->add("aoeDurInc%", "float", 416);
        LootPrefix->add("healInc%", "float", 420);
        LootPrefix->add("deploy", "float", 440);
        LootPrefix->add("physDmgDecFlat", "float", 444);
        LootPrefix->add("energyDmgDecFlat", "float", 448);
        LootPrefix->add("minWpnDmg", "float", 452);
        LootPrefix->add("maxWpnDmg", "float", 456);
        LootPrefix->add("minWpnDmg", "float", 460);
        LootPrefix->add("maxWpnDmg%", "float", 464);
        LootPrefix->add("attDmg", "float", 468);
        LootPrefix->add("attDmg%", "float", 472);
        LootPrefix->add("xpBoost%", "float", 480);

    //   LootSuffix
        auto LootSuffix = add_struct("LootSuffix");
        LootSuffix->add("suffixId", "uint32_t", 0);
        LootSuffix->add("suffixName", "key", 16);
        LootSuffix->add("minLevel", "uint32_t", 20); //??? - off_FDAB24
        LootSuffix->add("maxLevel", "uint32_t", 24); //??? - off_FDAB24
        LootSuffix->addStructArray("partTypes", LootData, 28);
        LootSuffix->addStructArray("classTypes", LootData, 36);
        LootSuffix->addStructArray("scienceTypes", LootData, 44);
        LootSuffix->add("isUnique", "bool", 52);
        LootSuffix->add("isBasic", "bool", 53);
        LootSuffix->addStructArray("modifierGranted", LootData, 520);
        LootSuffix->addStructArray("abilityImproved", LootData, 528);
        LootSuffix->add("strength", "float", 56);
        LootSuffix->add("dexterity", "float", 60);
        LootSuffix->add("mind", "float", 64);
        LootSuffix->add("health", "float", 72);
        LootSuffix->add("mana", "float", 76);
        LootSuffix->add("mana", "float", 76);
        LootSuffix->add("critical", "float", 96);
        LootSuffix->add("criticalDamage", "float", 144);
        LootSuffix->add("physicalDefense", "float", 84);
        LootSuffix->add("energyDefense", "float", 92);
        LootSuffix->add("projectileSpeed", "float", 160);
        LootSuffix->add("cooldown", "float", 152);
        LootSuffix->add("aoeDamage", "float", 204);
        LootSuffix->add("aoeResistance%", "float", 164);
        LootSuffix->add("movementSpeed", "float", 248);
        LootSuffix->add("lifeSteal", "float", 196);
        LootSuffix->add("manaSteal", "float", 264);
        LootSuffix->add("attackSpeed", "float", 148);
        LootSuffix->add("damageLifeType", "float", 216);
        LootSuffix->add("damageTechType", "float", 208);
        LootSuffix->add("damageQuantumType", "float", 212);
        LootSuffix->add("damageNecroType", "float", 224);
        LootSuffix->add("damagePlasmaType", "float", 220);
        LootSuffix->add("resistLifeType", "float", 236);
        LootSuffix->add("resistTechType", "float", 228);
        LootSuffix->add("resistQuantumType", "float", 232);
        LootSuffix->add("resistNecroType", "float", 244);
        LootSuffix->add("resistPlasmaType", "float", 240);
        LootSuffix->add("aoeRadius", "float", 304);
        LootSuffix->add("petDamage", "float", 308);
        LootSuffix->add("petHealth", "float", 312);
        LootSuffix->add("crystalFind%", "float", 316);
        LootSuffix->add("lootFind%", "float", 340);
        LootSuffix->add("dnaDropped%", "float", 320);
        LootSuffix->add("rangeIncrease%", "float", 324);
        LootSuffix->add("orbEffect", "float", 328);
        LootSuffix->add("overdriveBuildup%", "float", 332);
        LootSuffix->add("overdriveDuration%", "float", 336);
        LootSuffix->add("surefooted%", "float", 344);
        LootSuffix->add("buffDuration%", "float", 256);
        LootSuffix->add("debuffShorten%", "float", 260);
        LootSuffix->add("debuffLengthen%", "float", 268);
        LootSuffix->add("dotDamage%", "float", 396);
        LootSuffix->add("aggroInc%", "float", 400);
        LootSuffix->add("aggroDec%", "float", 404);
        LootSuffix->add("physDmg%", "float", 408);
        LootSuffix->add("physAbDmg%", "float", 412);
        LootSuffix->add("energyDmg%", "float", 416);
        LootSuffix->add("energyAbDmg%", "float", 420);
        LootSuffix->add("immStun%", "float", 348);
        LootSuffix->add("immBanish", "float", 296);
        LootSuffix->add("immKnockBack", "float", 300);
        LootSuffix->add("immSleep", "float", 356);
        LootSuffix->add("immTaunt", "float", 360);
        LootSuffix->add("immTerror", "float", 364);
        LootSuffix->add("immSilence", "float", 368);
        LootSuffix->add("immCurse", "float", 372);
        LootSuffix->add("immPoison", "float", 376);
        LootSuffix->add("immBurn", "float", 380);
        LootSuffix->add("immRoot", "float", 384);
        LootSuffix->add("immSlow", "float", 388);
        LootSuffix->add("immPull", "float", 392);
        LootSuffix->add("channelTime%", "float", 424);
        LootSuffix->add("ccDur%", "float", 428);
        LootSuffix->add("dotDur%", "float", 432);
        LootSuffix->add("aoeDurInc%", "float", 436);
        LootSuffix->add("healInc%", "float", 440);
        LootSuffix->add("deploy", "float", 460);
        LootSuffix->add("physDmgDecFlat", "float", 464);
        LootSuffix->add("energyDmgDecFlat", "float", 468);
        LootSuffix->add("minWpnDmg", "float", 472);
        LootSuffix->add("maxWpnDmg", "float", 476);
        LootSuffix->add("minWpnDmg%", "float", 480);
        LootSuffix->add("maxWpnDmg%", "float", 484);
        LootSuffix->add("attDmg", "float", 488);
        LootSuffix->add("attDmg%", "float", 492);
        LootSuffix->add("xpBoost%", "float", 500);

    auto cAudioEventData = add_struct("cAudioEventData", 32);
         cAudioEventData->add("sound", "key", 12);
         cAudioEventData->add("bIs3D", "bool", 16);
         cAudioEventData->add("bIsVoiceOver", "bool", 18);
         cAudioEventData->add("bHasLocalOffset", "bool", 17);
         cAudioEventData->add("localOffset", "cSPVector3", 20);

    auto cHardpointInfo = add_struct("cHardpointInfo", 24); /////// NEED FIXXXXXXXXXXXXX
         cHardpointInfo->add("type", "enum", 0);
         cHardpointInfo->add("bodyCap", "enum", 4);
         cHardpointInfo->add("localOffset", "cSPVector3", 8);
         cHardpointInfo->add("attractor", "bool", 20);
         cHardpointInfo->add("attached", "bool", 21);
    registerNullableType("cHardpointInfo");

    auto cEffectEventData = add_struct("cEffectEventData", 64);
         cEffectEventData->add("name", "key", 12);
         cEffectEventData->add("bScaleWithObject", "bool", 16);
         cEffectEventData->add("localizedTextKey", "key", 24);
         cEffectEventData->add("bHasTextValue", "bool", 17);
         cEffectEventData->add("bSetModelPointer", "bool", 18);
         cEffectEventData->add("bSetModelPointer", "bool", 18);
         cEffectEventData->add("bZUpAlignment", "bool", 19);
         cEffectEventData->add("bCreatureOrientationAlignment", "bool", 20);
         cEffectEventData->add("bUseTargetPoint", "bool", 21);
         cEffectEventData->add("bSetDirectionFromSecondaryObject", "bool", 22);
         cEffectEventData->add("objectHardpoint", "nullable", cHardpointInfo, 22);
         cEffectEventData->add("secondaryObjectHardpoint", "nullable", cHardpointInfo, 44);
         cEffectEventData->add("screenShakeScaleLocalPlayer", "float", 48);
         cEffectEventData->add("screenShakeScaleEveryone", "float", 52);
         cEffectEventData->add("screenShakeScaleLocalPlayerCritical", "float", 56);
         cEffectEventData->add("screenShakeScaleEveryoneCritical", "float", 60);

    //   ServerEventDef
    auto ServerEventDef = add_struct("ServerEventDef");
         ServerEventDef->addStructArray("audio", cAudioEventData, 0);
         ServerEventDef->addStructArray("effects", cEffectEventData, 8);

    //   Markerset
    auto cLabsMarker = add_struct("cLabsMarker", 200);
         cLabsMarker->add("markerName", "char*", 0);
         cLabsMarker->add("markerId", "uint32_t", 4);
         cLabsMarker->add("nounDef", "asset", 8);
         cLabsMarker->add("pos", "cSPVector3", 28);
         cLabsMarker->add("rotDegrees", "cSPVector3", 40);
         cLabsMarker->add("scale", "float", 52);
         cLabsMarker->add("dimensions", "cSPVector3", 56);
         cLabsMarker->add("visible", "bool", 68);
         cLabsMarker->add("castShadows", "bool", 69);
         cLabsMarker->add("backFaceShadows", "bool", 70);
         cLabsMarker->add("onlyShadows", "bool", 71);
         cLabsMarker->add("createWithCollision", "bool", 72);
         cLabsMarker->add("debugDisplayKDTree", "bool", 73);
         cLabsMarker->add("debugDisplayNormals", "bool", 74);
         cLabsMarker->add("navMeshSetting", "enum", 76);
         cLabsMarker->add("assetOverrideId", "uint64_t", 16);
         cLabsMarker->add( SharedComponentData, 156);

         auto cPointLightData = add_struct("cPointLightData", 84);
              cPointLightData->add("diffuse_color", "cSPVector3", 0);
              cPointLightData->add("diffuse_lamp_power", "float", 12);
              cPointLightData->add("specular_lamp_power", "float", 16);
              cPointLightData->add("inner_radius", "float", 20);
              cPointLightData->add("radius", "float", 24);
              cPointLightData->add("gobo", "key", 40);
              cPointLightData->add("frames", "uint32_t", 40); //??? - off_FDAB24
              cPointLightData->add("has_spec", "bool", 48);
              cPointLightData->add("enable", "bool", 49);
              cPointLightData->add("show_volume", "bool", 50);
              cPointLightData->add("wind_blown", "bool", 52);
              cPointLightData->add("wind_pivot_pos", "cSPVector3", 56);
              cPointLightData->add("wind_pivot_rot", "cSPVector3", 68);
              cPointLightData->add("wind_flex", "float", 80);
         registerNullableType("cPointLightData");
         cLabsMarker->add("cPointLightData", "nullable", cPointLightData, 92);

         auto cSpotLightData = add_struct("cSpotLightData", 96);
              cSpotLightData->add("diffuse_color", "cSPVector3", 0);
              cSpotLightData->add("diffuse_lamp_power", "float", 12);
              cSpotLightData->add("specular_lamp_power", "float", 16);
              cSpotLightData->add("inner_radius", "float", 20);
              cSpotLightData->add("radius", "float", 24);
              cSpotLightData->add("falloff", "float", 28);
              cSpotLightData->add("length", "float", 32);
              cSpotLightData->add("shadow_bias", "float", 36);
              cSpotLightData->add("gobo", "key", 52);
              cSpotLightData->add("frames", "key", 56); //??? - off_FDAB24
              cSpotLightData->add("has_spec", "bool", 60);
              cSpotLightData->add("enable", "bool", 61);
              cSpotLightData->add("shadow_caster", "bool", 62);
              cSpotLightData->add("show_frustum", "bool", 63);
              cSpotLightData->add("show_volume", "bool", 64);
              cSpotLightData->add("wind_blown", "bool", 66);
              cSpotLightData->add("wind_pivot_pos", "cSPVector3", 68);
              cSpotLightData->add("wind_pivot_rot", "cSPVector3", 80);
              cSpotLightData->add("wind_flex", "float", 92);
         registerNullableType("cSpotLightData");
         cLabsMarker->add("cSpotLightData", "nullable", cSpotLightData, 96);

         auto cLineLightData = add_struct("cLineLightData", 84);
              cLineLightData->add("diffuse_color", "cSPVector3", 0);
              cLineLightData->add("diffuse_lamp_power", "float", 12);
              cLineLightData->add("specular_lamp_power", "float", 16);
              cLineLightData->add("inner_radius", "float", 20);
              cLineLightData->add("radius", "float", 24);
              cLineLightData->add("length", "float", 28);
              cLineLightData->add("gobo", "key", 44);
              cLineLightData->add("has_spec", "bool", 60);
              cLineLightData->add("enable", "bool", 49);
              cLineLightData->add("show_volume", "bool", 50);
              cLineLightData->add("wind_blown", "bool", 52);
              cLineLightData->add("wind_pivot_pos", "cSPVector3", 56);
              cLineLightData->add("wind_pivot_rot", "cSPVector3", 68);
              cLineLightData->add("wind_flex", "float", 80);
         registerNullableType("cLineLightData");
         cLabsMarker->add("cLineLightData", "nullable", cLineLightData, 100);

         auto cParallelLightData = add_struct("cParallelLightData", 84);
         cParallelLightData->add("diffuse_color", "cSPVector3", 24);
         cParallelLightData->add("diffuse_lamp_power", "float", 36);
         cParallelLightData->add("specular_lamp_power", "float", 40);
         cParallelLightData->add("fill_diffuse", "cSPVector3", 44);
         cParallelLightData->add("fill_diffuse_power", "cSPVector3", 56);
         cParallelLightData->add("fill_specular_power", "cSPVector3", 60);
         cParallelLightData->add("has_spec", "bool", 64);
         cParallelLightData->add("enable", "bool", 65);
         cParallelLightData->add("type", "enum", 68);
         cParallelLightData->add("radius", "float", 72);
         cParallelLightData->add("inner_radius", "float", 76);
         cParallelLightData->add("shadowed", "enum", 80);
         registerNullableType("cParallelLightData");
         cLabsMarker->add("cParallelLightData", "nullable", cParallelLightData, 104);

         auto cGraphicsData = add_struct("cGraphicsData", 1304);
               cGraphicsData->add("camera_far_clip", "float", 0);
               cGraphicsData->add("shadow_dir", "cSPVector3", 4);
               cGraphicsData->add("shadow_transparency", "float", 16);
               cGraphicsData->add("shadow_zbias", "float", 20);
               cGraphicsData->add("shadow_camera_z_fade", "float", 24);
               cGraphicsData->add("shadow_view_left", "float", 28);
               cGraphicsData->add("shadow_view_top", "float", 32);
               cGraphicsData->add("shadow_view_right", "float", 36);
               cGraphicsData->add("shadow_view_bottom", "float", 40);
               cGraphicsData->add("shadow_view_height", "float", 44);
               cGraphicsData->add("shadow_view_near", "float", 48);
               cGraphicsData->add("shadow_view_far", "float", 52);
               cGraphicsData->add("shadow_cull_left", "float", 56);
               cGraphicsData->add("shadow_cull_top", "float", 60);
               cGraphicsData->add("shadow_cull_right", "float", 64);
               cGraphicsData->add("shadow_cull_bottom", "float", 68);
               cGraphicsData->add("shadow_cull_near", "float", 72);
               cGraphicsData->add("shadow_cull_far", "float", 76);
               cGraphicsData->add("bloom", "float", 80);
               cGraphicsData->add("brightness", "float", 84);
               cGraphicsData->add("contrast", "float", 88);
               cGraphicsData->add("saturation", "float", 92);
               cGraphicsData->add("hue", "float", 96);
               cGraphicsData->add("color_adjust_texture", "char", 100);
               cGraphicsData->add("wind_dir", "cSPVector2", 164);
               cGraphicsData->add("wind_str", "float", 172);
               cGraphicsData->add("fog_color", "cSPVector3", 272);
               cGraphicsData->add("fog_density", "float", 284);
               cGraphicsData->add("fog_amplitude", "float", 288);
               cGraphicsData->add("fog_ground_start", "float", 292);
               cGraphicsData->add("fog_ground_end", "float", 296);
               cGraphicsData->add("fog_anim_rate", "float", 300);
               cGraphicsData->add("fog_strip_length", "uint32_t", 300); //??? - off_FDAB24
               cGraphicsData->add("fog_dir", "cSPVector2", 308);
               cGraphicsData->add("fog_rate", "float", 316);
               cGraphicsData->add("fog_scale", "cSPVector2", 320);
               cGraphicsData->add("fog_texture", "char", 328);
               cGraphicsData->add("cloud_shadow_texture", "char", 176);
               cGraphicsData->add("cloud_shadow_direction", "cSPVector2", 240);
               cGraphicsData->add("cloud_shadow_tile", "cSPVector2", 248);
               cGraphicsData->add("cloud_shadow_rate", "float", 256);
               cGraphicsData->add("cloud_shadow_start", "float", 260);
               cGraphicsData->add("cloud_shadow_end", "float", 264);
               cGraphicsData->add("cloud_shadow_alpha", "float", 268);
               cGraphicsData->add("player_diffuse", "float", 392);
               cGraphicsData->add("player_specular", "float", 396);
               cGraphicsData->add("player_desaturate", "float", 400);
               cGraphicsData->add("player_emissive_level", "float", 404);
               cGraphicsData->add("creature_diffuse", "float", 408);
               cGraphicsData->add("creature_specular", "float", 412);
               cGraphicsData->add("creature_desaturate", "float", 416);
               cGraphicsData->add("creature_emissive_level", "float", 420);
               cGraphicsData->add("flora_diffuse", "float", 424);
               cGraphicsData->add("flora_specular", "float", 428);
               cGraphicsData->add("flora_desaturate", "float", 432);
               cGraphicsData->add("flora_emissive_level", "float", 436);
               cGraphicsData->add("mineral_diffuse", "float", 440);
               cGraphicsData->add("mineral_specular", "float", 444);
               cGraphicsData->add("mineral_desaturate", "float", 448);
               cGraphicsData->add("mineral_emissive_level", "float", 452);
               cGraphicsData->add("toonCenterMin", "float", 456);
               cGraphicsData->add("toonCenterMax", "float", 468);
               cGraphicsData->add("toonAdjacentMin", "float", 460);
               cGraphicsData->add("toonAdjacentMax", "float", 472);
               cGraphicsData->add("toonCornerMin", "float", 464);
               cGraphicsData->add("toonCornerMax", "float", 476);

             auto cWaterSimData = add_struct("cWaterSimData", 200);
                  cWaterSimData->add("water_pos", "cSPVector2", 0);
                  cWaterSimData->add("water_pos_vari", "cSPVector2", 8);
                  cWaterSimData->add("water_size", "cSPVector2", 16);
                  cWaterSimData->add("water_size_vari", "cSPVector2", 24);
                  cWaterSimData->add("water_angle", "float", 32);
                  cWaterSimData->add("water_angle_vari", "float", 36);
                  cWaterSimData->add("water_intensity", "float", 40);
                  cWaterSimData->add("water_intensity_vari", "float", 44);
                  cWaterSimData->add("water_freq", "float", 48);
                  cWaterSimData->add("water_freq_vari", "float", 52);
                  cWaterSimData->add("water_wave_speed", "float", 56);
                  cWaterSimData->add("water_dampening", "float", 60);
                  cWaterSimData->add("water_normal_scale", "float", 64);
                  cWaterSimData->add("water_brush", "char", 68);
                  cWaterSimData->add("water_mask", "char", 132);
                  cWaterSimData->add("water_blend", "bool", 196);
             registerNullableType("cWaterSimData");

               cGraphicsData->add("water0", cWaterSimData, 480);
               cGraphicsData->add("water1", cWaterSimData, 680);
               cGraphicsData->add("water2", cWaterSimData, 880);
               cGraphicsData->add("water3", cWaterSimData, 1080);

               cGraphicsData->add("camera_blend", "bool", 1280);
               cGraphicsData->add("color_blend", "bool", 1281);
               cGraphicsData->add("shadow_blend", "bool", 1282);
               cGraphicsData->add("bloom_blend", "bool", 1283);
               cGraphicsData->add("wind_blend", "bool", 1284);
               cGraphicsData->add("cloud_blend", "bool", 1285);
               cGraphicsData->add("fog_blend", "bool", 1286);
               cGraphicsData->add("levels_blend", "bool", 1287);
               cGraphicsData->add("radius", "float", 1292);
               cGraphicsData->add("inner_radius", "float", 1296);
               cGraphicsData->add("global", "bool", 1288);
               cGraphicsData->add("display_volume", "bool", 1300);
         registerNullableType("cGraphicsData");
         cLabsMarker->add("cGraphicsData", "nullable", cGraphicsData, 112);

         auto cAnimatedData = add_struct("cAnimatedData", 4);
              cAnimatedData->add("animator", "char*", 0);
         registerNullableType("cAnimatedData");
         cLabsMarker->add("animatedData", "nullable", cAnimatedData, 124);

         auto cAnimatorData = add_struct("cAnimatorData", 16);
              cAnimatorData->add("animator_name", "char*", 0);
              cAnimatorData->add("rate", "float", 4);
              cAnimatorData->add("delay", "float", 8);
              cAnimatorData->add("track", "char*", 12);
         registerNullableType("cAnimatorData");
         cLabsMarker->add("animatorData", "nullable", cAnimatorData, 120);

         auto cCameraComponentData = add_struct("cCameraComponentData", 20);
              cCameraComponentData->add("azimuth", "float", 0);
              cCameraComponentData->add("elevation", "float", 4);
              cCameraComponentData->add("distance", "float", 8);
              cCameraComponentData->add("transitionRate", "float", 12);
              cCameraComponentData->add("duration", "float", 16);
         registerNullableType("cCameraComponentData");
         cLabsMarker->add("cameraComponentData", "nullable", cCameraComponentData, 116);

         auto cDecalData = add_struct("cDecalData", 268);
              cDecalData->add("size", "cSPVector3", 0);
              cDecalData->add("material", "char", 12);
              cDecalData->add("layer", "uint32_t", 76); //??? - off_FDAB24
              cDecalData->add("diffuse", "char", 80);
              cDecalData->add("normal", "char", 144);
              cDecalData->add("diffuseTint", "cSPVector3", 208);
              cDecalData->add("opacity", "float", 220);
              cDecalData->add("specularTint", "cSPVector3", 224);
              cDecalData->add("opacityNormal", "cSPVector3", 236);
              cDecalData->add("tile", "cSPVector2", 240);
              cDecalData->add("normalLevel", "float", 248);
              cDecalData->add("glowLevel", "float", 252);
              cDecalData->add("emissiveLevel", "float", 256);
              cDecalData->add("specExponent", "float", 260);
              cDecalData->add("enable", "bool", 264);
              cDecalData->add("display_volume", "bool", 265);
         registerNullableType("cDecalData");
         cLabsMarker->add("decalData", "nullable", cDecalData, 128);

         auto cWaterData = add_struct("cWaterData", 432);
              cWaterData->add("size", "cSPVector2", 0);
              cWaterData->add("reflection", "char", 8);
              cWaterData->add("mask", "char", 72);
              cWaterData->add("normal_mask", "char", 136);
              cWaterData->add("splash_effect", "char", 200);
              cWaterData->add("foam_effect", "char", 264);
              cWaterData->add("diffuseTint", "cSPVector3", 328);
              cWaterData->add("specularTint", "cSPVector3", 340);
              cWaterData->add("depthFogColor", "cSPVector3", 352);
              cWaterData->add("tile", "cSPVector2", 364);
              cWaterData->add("normalLevel", "float", 372);
              cWaterData->add("specExponent", "float", 376);
              cWaterData->add("fresnel_bias", "float", 380);
              cWaterData->add("fresnel_power", "float", 384);
              cWaterData->add("refract_level", "float", 388);
              cWaterData->add("reflect_level", "float", 392);
              cWaterData->add("depth_fog", "float", 396);
              cWaterData->add("refract_cue_bias", "float", 400);
              cWaterData->add("refract_cue_scale", "float", 404);
              cWaterData->add("reflect_cue_bias", "float", 408);
              cWaterData->add("reflect_cue_scale", "float", 412);
              cWaterData->add("normal_cue_bias", "float", 416);
              cWaterData->add("normal_cue_scale", "float", 420);
              cWaterData->add("simulation", "uint32_t", 424); //??? - off_FDAB24
              cWaterData->add("soft_edges", "bool", 428);
              cWaterData->add("interactive", "bool", 429);
              cWaterData->add("enable", "bool", 430);
              cWaterData->add("display_volume", "bool", 431);
         registerNullableType("cWaterData");
         cLabsMarker->add("waterData", "nullable", cWaterData, 132);

         auto cGrassData = add_struct("cGrassData", 292);
              cGrassData->add("diffuse", "char", 0);
              cGrassData->add("normal", "char", 64);
              cGrassData->add("mat", "char", 128);
              cGrassData->add("diffuseTint", "cSPVector3", 192);
              cGrassData->add("specularTint", "cSPVector3", 204);
              cGrassData->add("size", "cSPVector3", 216);
              cGrassData->add("offset", "cSPVector2", 228);
              cGrassData->add("tile", "cSPVector2", 236);
              cGrassData->add("normalLevel", "float", 244);
              cGrassData->add("flexibility", "float", 248);
              cGrassData->add("glowLevel", "float", 252);
              cGrassData->add("emissiveLevel", "float", 256);
              cGrassData->add("density", "float", 260);
              cGrassData->add("posVari", "float", 264);
              cGrassData->add("heightVari", "float", 268);
              cGrassData->add("width", "float", 272);
              cGrassData->add("bend", "float", 276);
              cGrassData->add("bendVari", "float", 280);
              cGrassData->add("seed", "float", 284); //??? - off_FDAB24
              cGrassData->add("cast_shadows", "bool", 288);
              cGrassData->add("enable", "bool", 289);
              cGrassData->add("display_volume", "bool", 290);
         registerNullableType("cGrassData");
         cLabsMarker->add("grassData", "nullable", cGrassData, 136);

         auto cMapCameraData = add_struct("cMapCameraData", 84);
              cMapCameraData->add("name", "char", 0);
              cMapCameraData->add("fov", "float", 64);
              cMapCameraData->add("aspect", "float", 68);
              cMapCameraData->add("near", "float", 72);
              cMapCameraData->add("far", "float", 76);
              cMapCameraData->add("show_bounds", "bool", 80);
              cMapCameraData->add("show_pip", "bool", 81);
         registerNullableType("cMapCameraData");
         cLabsMarker->add("mapCameraData", "nullable", cMapCameraData, 140);

         auto cOccluderData = add_struct("cOccluderData", 44);
              cOccluderData->add("name", "char", 0);
              cOccluderData->add("width", "uint32_t", 32); //??? - off_FDAB24
              cOccluderData->add("height", "uint32_t", 36); //??? - off_FDAB24
              cOccluderData->add("active", "bool", 40);
          registerNullableType("cOccluderData");
          cLabsMarker->add("occluderData", "nullable", cOccluderData, 144);

         auto cSplineCameraData = add_struct("cSplineCameraData", 132);
              auto cSplineCameraNodeBaseData = add_struct("cSplineCameraNodeBaseData", 128);
		           cSplineCameraNodeBaseData->add("name", "char", 0);
		           cSplineCameraNodeBaseData->add("fov", "float", 32);
		           cSplineCameraNodeBaseData->add("near", "float", 36);
		           cSplineCameraNodeBaseData->add("far", "float", 40);
		           cSplineCameraNodeBaseData->add("knot", "uint32_t", 44); //??? - off_FDAB24
		           cSplineCameraNodeBaseData->add("wait", "float", 48);
		           cSplineCameraNodeBaseData->add("skipable", "bool", 52);
		           cSplineCameraNodeBaseData->add("message", "char", 53);
              //addType("struct:cSplineCameraNodeBaseData", DataType::STRUCT, 128, "cSplineCameraNodeBaseData");
              //cSplineCameraData->add("node", "struct:SharedComponentData", 0);
              cSplineCameraData->add(cSplineCameraNodeBaseData, 0);
              cSplineCameraData->add("duration", "float", 128);
         registerNullableType("cSplineCameraData");
         cLabsMarker->add("splineCameraData", "nullable", cSplineCameraData, 148);

         cLabsMarker->add("splineCameraNodeData", "nullable", cSplineCameraNodeBaseData, 152);

         cLabsMarker->add("volumeDef", "nullable", cVolumeDef, 88);

         cLabsMarker->add("ignoreOnXBox", "bool", 80);
         cLabsMarker->add("ignoreOnMinSpec", "bool", 81);
         cLabsMarker->add("ignoreOnPC", "bool", 82);
         cLabsMarker->add("highSpecOnly", "uint32_t", 83);
         cLabsMarker->add("targetMarkerId", "uint32_t", 84);

    auto MarkerSet = add_struct("MarkerSet");
         MarkerSet->addStructArray("markers", cLabsMarker, 0);
         MarkerSet->add("group", "key", 20);
         MarkerSet->add("weight", "float", 24);
         MarkerSet->addArray("condition", "enum", 32);

    //   Level
    auto LevelMarkerset = add_struct("LevelMarkerset", 4);
	     LevelMarkerset->add("markersetAsset", "asset", 0);

    auto DirectorClass = add_struct("DirectorClass", 16);
         DirectorClass->add("mpNoun", "asset", 0);
         DirectorClass->add("minDifficulty", "uint32_t", 4); //??? - off_FDAB24
         DirectorClass->add("maxDifficulty", "uint32_t", 8); //??? - off_FDAB24
         DirectorClass->add("hordeLegal", "bool", 12);

    auto LevelConfig = add_struct("LevelConfig", 40);
         LevelConfig->addStructArray("minion", DirectorClass, 0, 20);
         LevelConfig->addStructArray("special", DirectorClass, 4, 20);
         LevelConfig->addStructArray("boss", DirectorClass, 8, 20);
         LevelConfig->addStructArray("agent", DirectorClass, 12, 20);
         LevelConfig->addStructArray("captain", DirectorClass, 16, 20);
    registerNullableType("LevelConfig");

    auto LevelCameraSettings = add_struct("LevelCameraSettings", 12);
         LevelCameraSettings->add("pitchOverride", "float", 0);
         LevelCameraSettings->add("yawOverride", "float", 4);
         LevelCameraSettings->add("distanceOverride", "float", 8);
    registerNullableType("LevelCameraSettings");

    auto Level = add_struct("Level");
         Level->addStructArray("markersets", LevelMarkerset, 0);
         Level->add("music", "key", 20);
         Level->add("navMesh", "key", 36);
         Level->add("physicsMesh", "key", 52);
         Level->add("renderingConfig", "key", 68);
         Level->add("footstepEffect", "key", 84);
         Level->add("levelConfig", "nullable", LevelConfig, 88);
         Level->add("firstTimeConfig", "nullable", LevelConfig, 92);
         Level->add("planetConfig", "asset", 96);
         Level->add("levelObjectives", "asset", 100);
         Level->add("name", "uint32_t", 104);
         Level->add("briefDescription", "uint32_t", 108);
         Level->add("description", "uint32_t", 112);
         Level->add("primaryType", "enum", 116);
         Level->add("secondaryType", "enum", 120);
         Level->add("planetScreenBG", "key", 144);
         Level->add("cameraSettings", "nullable", LevelCameraSettings, 148);

    //   AffixTuning
    auto AffixTuning = add_struct("AffixTuning");
         AffixTuning->addArray("positiveChance", "uint32_t", 0);
         AffixTuning->addArray("minorChance", "uint32_t", 8);
         AffixTuning->addArray("majorChance", "uint32_t", 16);

    //   CharacterType
    auto CharacterType = add_struct("CharacterType");
         CharacterType->add("BaseResistance_Technology", "float", 0);
         CharacterType->add("BaseResistance_Spacetime", "float", 12);
         CharacterType->add("BaseResistance_Life", "float", 24);
         CharacterType->add("BaseResistance_Elements", "float", 36);
         CharacterType->add("BaseResistance_Supernatural", "float", 48);
         CharacterType->add("DamageMultiplier_Technology", "float", 60);
         CharacterType->add("DamageMultiplier_Spacetime", "float", 72);
         CharacterType->add("DamageMultiplier_Life", "float", 84);
         CharacterType->add("DamageMultiplier_Elements", "float", 96);
         CharacterType->add("DamageMultiplier_Supernatural", "float", 108);
         CharacterType->add("UIColor", "uint32_t", 120);

    //   Condition
    auto cAssetProperty = add_struct("cAssetProperty", 188);
	     cAssetProperty->add("name", "char", 4);
	     cAssetProperty->add("value", "char", 88);
	     cAssetProperty->add("type", "guid", 84);
    auto cAssetPropertyList = add_struct("cAssetPropertyList", 8);
         cAssetPropertyList->addStructArray("mpAssetProperties", cAssetProperty, 0);
    auto Condition = add_struct("Condition");
         Condition->add("condition", "key", 12);
         Condition->add("conditionProps", cAssetPropertyList, 16);
         Condition->add("activateOnce", "bool", 24);
         Condition->add("checkOnSequenceEnd", "bool", 25);
         Condition->add("activateTime", "float", 28);
         Condition->add("checkTimeInterval", "float", 32);

    //   CrystalTuning
	auto CrystalLevel = add_struct("CrystalLevel", 8);
         CrystalLevel->add("offset", "uint32_t", 0);
         CrystalLevel->add("probability", "float", 4);

	auto CrystalDropDef = add_struct("CrystalDropDef", 16);
         CrystalDropDef->add("minLevel", "uint32_t", 0);
         CrystalDropDef->add("maxLevel", "uint32_t", 4);
         CrystalDropDef->add("weight", "uint32_t", 8);
         CrystalDropDef->add("mpNoun", "asset", 12);

	auto CrystalTuning = add_struct("CrystalTuning");
	  	 CrystalTuning->add("threeInARowBonusPercent", "float", 0);
	  	 CrystalTuning->addStructArray("CrystalLevel", CrystalLevel, 12);
	 	 CrystalTuning->addStructArray("crystal", CrystalDropDef, 4);

    //   DifficultyTuning
    auto DifficultyTuning = add_struct("DifficultyTuning");
         DifficultyTuning->addArray("HealthPercentIncrease", "float", 0);
         DifficultyTuning->addArray("DamagePercentIncrease", "float", 8);
         DifficultyTuning->addArray("ItemLevelRange", "cSPVector2", 16);
         DifficultyTuning->addArray("GearScoreRange", "cSPVector2", 24);
         DifficultyTuning->add("GearScoreMax", "cSPVector2", 32);
         DifficultyTuning->addArray("ExpectedAvatarLevel", "uint32_t", 40);
         DifficultyTuning->addArray("RatingConversion", "float", 48);
         DifficultyTuning->add("StarModeHealthMult", "float", 56);
         DifficultyTuning->add("StarModeDamageMult", "float", 60);
         DifficultyTuning->add("StarModeEliteChanceAdd", "float", 64);
         DifficultyTuning->add("StarModeSuggestedLevelAdd", "float", 68);

    //   DirectorTuning
    auto DirectorTuning = add_struct("DirectorTuning");
         DirectorTuning->addArray("HealthPercentIncrease", "float", 0);
         DirectorTuning->addArray("DamagePercentIncrease", "uint32_t", 8);

    //   EliteNPCGlobals
    auto cAffixDifficultyTuning = add_struct("cAffixDifficultyTuning", 24);
         cAffixDifficultyTuning->add("minAffixes", "uint32_t", 0);
         cAffixDifficultyTuning->add("maxAffixes", "uint32_t", 4);
         cAffixDifficultyTuning->add("chanceToSpawn", "float", 8);
         cAffixDifficultyTuning->add("specialMinAffixes", "uint32_t", 12);
         cAffixDifficultyTuning->add("specialMaxAffixes", "uint32_t", 16);
         cAffixDifficultyTuning->add("specialChanceToSpawn", "float", 20);

    auto EliteNPCGlobals = add_struct("EliteNPCGlobals");
         EliteNPCGlobals->addStructArray("perLevelTuning", cAffixDifficultyTuning, 0);
         EliteNPCGlobals->add("textColor", "cSPVector4", 8);

    //   LevelObjectives
    auto LevelKey = add_struct("LevelKey", 16);
         LevelKey->add("name", "key", 12);

    auto LevelObjectives = add_struct("LevelObjectives");
         LevelObjectives->addStructArray("objective", LevelKey, 0);
         LevelObjectives->addStructArray("affix", LevelKey, 8);
         LevelObjectives->addStructArray("positiveAffix", LevelKey, 16);
         LevelObjectives->addStructArray("minorAffix", LevelKey, 24);
         LevelObjectives->addStructArray("majorAffix", LevelKey, 32);

    //   LootPreferences
    auto LootPreferences = add_struct("LootPreferences");
         LootPreferences->add("slotChance_Offense", "float", 0);
         LootPreferences->add("slotChance_Defense", "float", 4);
         LootPreferences->add("slotChance_Utility", "float", 8);
         LootPreferences->add("slotChance_Weapon", "float", 12);
         LootPreferences->add("slotChance_Foot", "float", 16);
         LootPreferences->add("slotChance_Hand", "float", 20);
         LootPreferences->add("scienceChance_Tech", "float", 24);
         LootPreferences->add("scienceChance_Quantum", "float", 28);
         LootPreferences->add("scienceChance_Bio", "float", 32);
         LootPreferences->add("scienceChance_Plasma", "float", 36);
         LootPreferences->add("scienceChance_Necro", "float", 40);
         LootPreferences->add("npcDropRarityChances_Basic", "char", 44);
         LootPreferences->add("npcDropRarityChances_Uncommon", "char", 172);
         LootPreferences->add("npcDropRarityChances_Rare", "char", 300);
         LootPreferences->add("npcDropRarityChances_Epic", "char", 428);
         LootPreferences->add("majorLevelMultiplier", "uint32_t", 556);
         LootPreferences->add("minorLevelMultiplier", "uint32_t", 560);
         LootPreferences->add("lastMinorLevelBonus", "uint32_t", 564);
         LootPreferences->add("rarityLevelMultiplier", "uint32_t", 568);
         LootPreferences->add("pointValue_BasePoints", "float", 572);
         LootPreferences->add("pointValue_ExponentBasesPerItemLevel", "float", 576);
         LootPreferences->add("pointValue_ExtraStatsBonusFactor", "float", 704);
         LootPreferences->add("pointCost_Strength", "uint32_t", 708);
         LootPreferences->add("pointCost_Dexterity", "uint32_t", 712);
         LootPreferences->add("pointCost_Mind", "uint32_t", 716);
         LootPreferences->add("pointCost_Health", "uint32_t", 720);
         LootPreferences->add("pointCost_Mana", "uint32_t", 724);
         LootPreferences->add("pointCost_Critical", "uint32_t", 728);
         LootPreferences->add("pointCost_Dodge", "uint32_t", 732);
         LootPreferences->add("pointCost_Deflection", "uint32_t", 736);
         LootPreferences->add("pointCost_PhysicalDamageDecreaseFlat", "uint32_t", 740);
         LootPreferences->add("pointCost_EnergyDamageDecreaseFlat", "uint32_t", 744);
         LootPreferences->add("pointCost_MinWeaponDamage", "uint32_t", 748);
         LootPreferences->add("pointCost_MaxWeaponDamage", "uint32_t", 752);
         LootPreferences->add("pointCost_DirectAttackDamage", "uint32_t", 756);
         LootPreferences->add("basic_StandardDist", "float", 760);
         LootPreferences->add("basic_SuffixDist", "float", 764);
         LootPreferences->add("uncommon_SuffixDist", "float", 768);
         LootPreferences->add("uncommon_Prefix1Dist", "float", 772);
         LootPreferences->add("uncommon_Prefix2Dist", "float", 776);
         LootPreferences->add("uncommon_StandardDist", "float", 780);
         LootPreferences->add("rare_SuffixDist", "float", 784);
         LootPreferences->add("rare_Prefix1Dist", "float", 788);
         LootPreferences->add("rare_Prefix2Dist", "float", 792);
         LootPreferences->add("rare_StandardDist", "float", 796);
         LootPreferences->add("epic_SuffixDist", "float", 800);
         LootPreferences->add("epic_Prefix1Dist", "float", 804);
         LootPreferences->add("epic_Prefix2Dist", "float", 808);
         LootPreferences->add("epic_StandardDist", "float", 812);
         LootPreferences->add("itemCost_Multiplier", "float", 816);
         LootPreferences->add("itemCost_LevelPower", "float", 820);
         LootPreferences->add("itemCost_RoundingVal", "uint32_t", 824);
         LootPreferences->add("baseStatScale_Other", "float", 828);
         LootPreferences->add("baseStatScale_Hand", "float", 832);
         LootPreferences->add("baseStatScale_HandTwo", "float", 836);
         LootPreferences->add("baseStatScale_Feet", "float", 840);
         LootPreferences->add("baseStatScale_FeetTwo", "float", 844);
         LootPreferences->add("baseStatScale_Offense", "float", 848);
         LootPreferences->add("baseStatScale_Defense", "float", 852);
         LootPreferences->add("baseStatScale_Utility", "float", 856);
         LootPreferences->add("minDropLevel_Hands", "uint32_t", 860);
         LootPreferences->add("minDropLevel_Feet", "uint32_t", 864);
         LootPreferences->add("minDropLevel_Weapons", "uint32_t", 868);
         LootPreferences->add("minDropLevel_Offense", "uint32_t", 872);
         LootPreferences->add("minDropLevel_Defense", "uint32_t", 876);
         LootPreferences->add("minDropLevel_Utility", "uint32_t", 880);
         LootPreferences->add("weaponDamage_Scaler", "float", 884);

    //   LootRigblock
    auto LootRigblock = add_struct("LootRigblock");
         LootRigblock->add("rigblockId", "uint32_t", 0);
         LootRigblock->add("rigblockName", "key", 16);
         LootRigblock->add("rigblockPropKey", "key", 32);
         LootRigblock->add("rigblockPartType", "char*", 36);
         LootRigblock->addStructArray("classTypes", LootData, 40);
         LootRigblock->addStructArray("scienceTypes", LootData, 48);
         LootRigblock->addStructArray("playerCharacters", LootData, 56);
         LootRigblock->add("minLevel", "uint32_t", 64);
         LootRigblock->add("maxLevel", "uint32_t", 68);
         LootRigblock->add("rigblockPngKey", "key", 84);
         LootRigblock->add("rigblockCategoryKey", "key", 100);
         LootRigblock->add("isUnique", "bool", 104);

    //   MagicNumbers
    auto MagicNumbers = add_struct("MagicNumbers");
         MagicNumbers->add("DamagePerPointOfStrength", "float", 0);
         MagicNumbers->add("DamagePerPointOfDexterity", "float", 4);
         MagicNumbers->add("DamagePerPointOfMind", "float", 8);
         MagicNumbers->add("HealthPerPointofStrength", "float", 12);
         MagicNumbers->add("PhysicalDefensePerPointofDexterity", "float", 16);
         MagicNumbers->add("CritRatingPerPointofDexterity", "float", 20);
         MagicNumbers->add("EnergyDefensePerPointofMind", "float", 24);
         MagicNumbers->add("ManaPerPointofMind", "float", 28);
         MagicNumbers->add("DefenseRatingDecreaseMultiplier", "float", 32);
         MagicNumbers->add("DefenseRatingDecreaseBase", "float", 36);
         MagicNumbers->add("CriticalRatingDecreaseMultiplier", "float", 40);
         MagicNumbers->add("CriticalRatingDecreaseBase", "float", 44);
         MagicNumbers->add("CriticalDamageBonus", "float", 48);
         MagicNumbers->add("PrimaryAttributeIgnoreAmount", "uint32_t", 52);
         MagicNumbers->add("LeechEffectivenessForAoE", "float", 56);
         MagicNumbers->add("LeechEffectivenessForAbilities", "float", 60);
         MagicNumbers->add("LeechEffectivenessForBasics", "float", 64);
         MagicNumbers->add("DodgePercentCap", "float", 68);
         MagicNumbers->add("ResistPercentCap", "float", 72);

    //   NavPowerTuning
    auto NavMeshLayer = add_struct("NavMeshLayer", 24);
	     NavMeshLayer->add("name", "char*", 0);
	     NavMeshLayer->add("voxelTestSize", "float", 4);
	     NavMeshLayer->add("agentRadius", "float", 8);
	     NavMeshLayer->add("agentHeight", "float", 12);
	     NavMeshLayer->add("maxStepSize", "float", 16);
	     NavMeshLayer->add("maxWalkableSlopeDegrees", "float", 20);


    auto NavPowerTuning = add_struct("NavPowerTuning");
         NavPowerTuning->add("maxWalkableSlopeDegrees", "float", 0);
         NavPowerTuning->addStructArray("navMeshLayers", NavMeshLayer, 4);

    //   ObjectExtents
    auto ExtentsCategory = add_struct("NavMeshLayer", 16);
         ExtentsCategory->add("extents", "cSPVector3", 0);
         ExtentsCategory->add("graphicsScale", "float", 12);


    auto ObjectExtents = add_struct("ObjectExtents");
         ObjectExtents->add("critterVertical", ExtentsCategory, 0);
         ObjectExtents->add("critterHorizontal", ExtentsCategory, 16);
         ObjectExtents->add("minionVertical", ExtentsCategory, 32);
         ObjectExtents->add("minionHorizontal", ExtentsCategory, 48);
         ObjectExtents->add("eliteMinionVertical", ExtentsCategory, 64);
         ObjectExtents->add("eliteMinionHorizontal", ExtentsCategory, 80);
         ObjectExtents->add("playerTempest", ExtentsCategory, 96);
         ObjectExtents->add("playerRavager", ExtentsCategory, 112);
         ObjectExtents->add("playerSentinel", ExtentsCategory, 128);
         ObjectExtents->add("lieutenantVertical", ExtentsCategory, 144);
         ObjectExtents->add("lieutenantHorizontal", ExtentsCategory, 160);
         ObjectExtents->add("eliteLieutenantVertical", ExtentsCategory, 176);
         ObjectExtents->add("eliteLieutenantHorizontal", ExtentsCategory, 192);
         ObjectExtents->add("captainVertical", ExtentsCategory, 208);
         ObjectExtents->add("captainHorizontal", ExtentsCategory, 224);
         ObjectExtents->add("bossVertical", ExtentsCategory, 240);
         ObjectExtents->add("bossHorizontal", ExtentsCategory, 256);

    //   SectionConfig
    auto DirectorBucket = add_struct("DirectorBucket", 16);
         DirectorBucket->add("numMinions", "uint32_t", 0);
         DirectorBucket->add("numSpecials", "uint32_t", 4);
         DirectorBucket->add("difficulty", "uint32_t", 8);
         DirectorBucket->add("chance", "float", 12);

    auto SectionConfig = add_struct("SectionConfig");
         SectionConfig->addStructArray("bucket", DirectorBucket, 0);

    //   SpaceshipTuning
    auto cSpaceshipCameraTuning = add_struct("DirectorBucket", 20);
         cSpaceshipCameraTuning->add("name", "key", 0);
         cSpaceshipCameraTuning->add("weight", "float", 16);

    auto SpaceshipTuning = add_struct("SpaceshipTuning");
         SpaceshipTuning->add("level", "asset", 0);
         SpaceshipTuning->add("conga_line_move_speed", "float", 4);
         SpaceshipTuning->add("dias_offset", "cSPVector3", 8);
         SpaceshipTuning->add("ui_delay_after_animation", "float", 20);
         SpaceshipTuning->add("ui_delay_mouseover", "float", 24);
         SpaceshipTuning->add("dias_light_default_color", "cSPVector3", 28);
         SpaceshipTuning->add("dias_light_swap_color", "cSPVector3", 40);
         SpaceshipTuning->add("dias_light_locked_color", "cSPVector3", 52);
         SpaceshipTuning->add("dias_light_collectable_color", "cSPVector3", 64);
         SpaceshipTuning->add("dias_light_in_deck_color", "cSPVector3", 76);
         SpaceshipTuning->addStructArray("camera_start_collection", cSpaceshipCameraTuning, 88);
         SpaceshipTuning->addStructArray("camera_start_map", cSpaceshipCameraTuning, 96);
         SpaceshipTuning->addStructArray("camera_start_planet", cSpaceshipCameraTuning, 104);
         SpaceshipTuning->addStructArray("camera_collection_map", cSpaceshipCameraTuning, 112);
         SpaceshipTuning->addStructArray("camera_collection_editor", cSpaceshipCameraTuning, 120);
         SpaceshipTuning->addStructArray("camera_map_collection", cSpaceshipCameraTuning, 128);
         SpaceshipTuning->addStructArray("camera_map_editor", cSpaceshipCameraTuning, 136);
         SpaceshipTuning->addStructArray("camera_editor_collection", cSpaceshipCameraTuning, 144);
         SpaceshipTuning->addStructArray("camera_editor_map", cSpaceshipCameraTuning, 152);
         SpaceshipTuning->addStructArray("camera_map_planet", cSpaceshipCameraTuning, 160);
         SpaceshipTuning->addStructArray("camera_planet_game", cSpaceshipCameraTuning, 168);
         SpaceshipTuning->addStructArray("camera_planet_collection", cSpaceshipCameraTuning, 176);
         SpaceshipTuning->addStructArray("camera_planet_editor", cSpaceshipCameraTuning, 184);
         SpaceshipTuning->add("small_placard_offset_base", "cSPVector3", 192);
         SpaceshipTuning->add("small_placard_rotation_base", "cSPVector3", 204);
         SpaceshipTuning->add("small_placard_scale_base", "float", 216);
         SpaceshipTuning->addArray("small_placard_offset", "cSPVector3", 220);
         SpaceshipTuning->addArray("small_placard_rotation", "cSPVector3", 228);
         SpaceshipTuning->addArray("small_placard_scale", "cSPVector3", 236);
         SpaceshipTuning->add("small_deck_placard_offset_base", "cSPVector3", 244);
         SpaceshipTuning->add("small_deck_placard_rotation_base", "cSPVector3", 256);
         SpaceshipTuning->add("small_deck_placard_scale_base", "float", 268);
         SpaceshipTuning->addArray("small_deck_placard_offset", "cSPVector3", 272);
         SpaceshipTuning->addArray("small_deck_placard_rotation", "cSPVector3", 280);
         SpaceshipTuning->addArray("small_deck_placard_scale", "float", 288);
         SpaceshipTuning->add("large_placard_offset_base", "cSPVector3", 296);
         SpaceshipTuning->add("large_placard_rotation_base", "cSPVector3", 308);
         SpaceshipTuning->add("large_placard_scale_base", "float", 320);
         SpaceshipTuning->addArray("large_placard_offset", "cSPVector3", 324);
         SpaceshipTuning->addArray("large_placard_rotation", "cSPVector3", 332);
         SpaceshipTuning->addArray("large_placard_scale", "float", 340);
         SpaceshipTuning->add("large_deck_placard_offset_base", "cSPVector3", 348);
         SpaceshipTuning->add("large_deck_placard_rotation_base", "cSPVector3", 360);
         SpaceshipTuning->add("large_deck_placard_scale_base", "float", 372);
         SpaceshipTuning->addArray("large_deck_placard_offset", "cSPVector3", 376);
         SpaceshipTuning->addArray("large_deck_placard_rotation", "cSPVector3", 384);
         SpaceshipTuning->addArray("large_deck_placard_scale", "float", 384);
         SpaceshipTuning->add("context_menu_offset_base", "cSPVector3", 400);
         SpaceshipTuning->add("context_menu_rotation_base", "cSPVector3", 412);
         SpaceshipTuning->add("context_menu_scale_base", "float", 424);
         SpaceshipTuning->addArray("context_menu_offset", "cSPVector3", 428);
         SpaceshipTuning->addArray("context_menu_rotation", "cSPVector3", 436);
         SpaceshipTuning->addArray("context_menu_scale", "float", 444);
         SpaceshipTuning->add("deck_context_menu_offset_base", "cSPVector3", 452);
         SpaceshipTuning->add("deck_context_menu_rotation_base", "cSPVector3", 464);
         SpaceshipTuning->add("deck_context_menu_scale_base", "float", 476);
         SpaceshipTuning->addArray("deck_context_menu_offset", "cSPVector3", 480);
         SpaceshipTuning->addArray("deck_context_menu_rotation", "cSPVector3", 488);
         SpaceshipTuning->addArray("deck_context_menu_scale", "float", 496);
         SpaceshipTuning->add("empty_squad_slot_offset_base", "cSPVector3", 504);
         SpaceshipTuning->add("empty_squad_slot_rotation_base", "cSPVector3", 516);
         SpaceshipTuning->add("empty_squad_slot_scale_base", "float", 528);
         SpaceshipTuning->addArray("empty_squad_slot_offset", "cSPVector3", 532);
         SpaceshipTuning->addArray("empty_squad_slot_rotation", "cSPVector3", 540);
         SpaceshipTuning->addArray("empty_squad_slot_scale", "cSPVector3", 548);
         SpaceshipTuning->add("controls_offset", "cSPVector3", 556);
         SpaceshipTuning->add("controls_rotation", "cSPVector3", 568);
         SpaceshipTuning->add("controls_scale", "float", 580);
         SpaceshipTuning->add("squad_controls_offset", "cSPVector3", 584);
         SpaceshipTuning->add("squad_controls_rotation", "cSPVector3", 584);
         SpaceshipTuning->add("squad_controls_scale", "float", 608);
         SpaceshipTuning->add("map_room_ui_offset", "cSPVector3", 612);
         SpaceshipTuning->add("map_room_ui_rotation", "cSPVector3", 624);
         SpaceshipTuning->add("map_room_ui_scale", "float", 636);
         SpaceshipTuning->add("planet_screen_ui_offset", "cSPVector3", 640);
         SpaceshipTuning->add("planet_screen_ui_rotation", "cSPVector3", 652);
         SpaceshipTuning->add("planet_screen_ui_scale", "float", 664);

    //   TestAsset
    auto TestAsset = add_struct("TestAsset");
         TestAsset->add("unk", "uint32_t", 0); // ??? - off_101F478
         TestAsset->add("bar", "uint32_t", 4);

    //   UnlocksTuning
    auto UnlockDef = add_struct("UnlockDef", 80);
         UnlockDef->add("id", "uint32_t", 0);
         UnlockDef->add("prerequisite", "uint32_t", 4);
         UnlockDef->add("cost", "uint32_t", 8);
         UnlockDef->add("level", "uint32_t", 12);
         UnlockDef->add("rank", "uint32_t", 16);
         UnlockDef->add("unlockType", "enum", 20);
         UnlockDef->add("value", "uint32_t", 24);
         UnlockDef->add("unlockFunction", "enum", 28);
         UnlockDef->add("image", "key", 44);
         UnlockDef->add("title", "key", 60);
         UnlockDef->add("description", "key", 76);

    auto UnlocksTuning = add_struct("UnlocksTuning");
         UnlocksTuning->addStructArray("unlock", UnlockDef, 0);

    //   WeaponTuning
    auto WeaponDef = add_struct("WeaponDef", 32);
         WeaponDef->add("refId", "uint64_t", 0);
         WeaponDef->add("itemLevel", "uint32_t", 8);
         WeaponDef->add("rigblockId", "uint32_t", 12);
         WeaponDef->add("suffixId", "uint32_t", 16);
         WeaponDef->add("cost", "uint32_t", 20);
         WeaponDef->add("avatarLevel", "uint32_t", 24);
         WeaponDef->add("chainProgression", "uint32_t", 28);

    auto WeaponTuning = add_struct("WeaponTuning");
         WeaponTuning->addStructArray("weapon", WeaponDef, 0);

    //   Catalog
    auto tags = add_struct("tags", 4);
         tags->add("tag", "char*", 0);

    auto CatalogEntry = add_struct("CatalogEntry", 40);
         CatalogEntry->add("assetNameWType", "char*", 0);
         CatalogEntry->add("compileTime", "int64_t", 8);
         CatalogEntry->add("dataCrc", "uint32_t", 24);
         CatalogEntry->add("typeCrc", "uint32_t", 20);
         CatalogEntry->add("sourceFileNameWType", "char*", 28);
         CatalogEntry->add("version", "uint32_t", 16);
         CatalogEntry->addStructArray("tags", tags, 32);

    auto Catalog = add_struct("Catalog");
         Catalog->addStructArray("entries", CatalogEntry, 0);

    //   ChainLevels
    auto LevelAsset = add_struct("LevelAsset");
         LevelAsset->add("unk", "asset", 0);

    auto ChainLevel = add_struct("ChainLevel", 72);
         ChainLevel->add("unk1", "asset", 0);
         ChainLevel->add("unkInt", "uint32_t", 4); //can be a count array of unk1
         ChainLevel->add("unk2", "asset", 20);
         ChainLevel->add("unk3", "asset", 36);
         ChainLevel->add("unk4", "asset", 52);
         ChainLevel->add("unk5", "asset", 68);

    auto ChainLevels = add_struct("ChainLevels");
         ChainLevels->addStructArray("ChainLevel", ChainLevel, 0);
         ChainLevels->add("unkFloat", "float", 8);

    //   PVPLevels
    auto PVPLevels = add_struct("PVPLevels");
         PVPLevels->addArray("levels", "asset", 0); //unk data name
}