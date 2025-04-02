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
    addType("cSPVector2", DataType::VECTOR2, 8);
    addType("cSPVector3", DataType::VECTOR3, 12);
    addType("cSPQuaternion", DataType::QUATERNION, 16);
    addType("char", DataType::CHAR, 1);
    addType("char*", DataType::CHAR_PTR, 4);
    addType("key", DataType::KEY, 4);
    addType("asset", DataType::ASSET, 4);
    addType("nullable", DataType::NULLABLE, 4);
    addType("array", DataType::ARRAY, 4);
    addType("enum", DataType::ENUM, 4);

    // Register filetypes
    registerFileType(".Phase", { "Phase" }, 68);
    registerFileType(".Noun", { "Noun" }, 480);

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

    auto bbox = add_struct("cSPBoundingBox");
    bbox->add("min", "cSPVector3", 56);
    bbox->add("max", "cSPVector3", 68);
    addType("struct:cSPBoundingBox", DataType::STRUCT, 24, "cSPBoundingBox");
    noun->add("boundingBox", "struct:cSPBoundingBox", 24);

    noun->add("presetExtents", "enum", 80);
    noun->add("voice", "key", 96);
    noun->add("foot", "key", 112);
    noun->add("flightSound", "key", 128);

    auto cGameObjectGfxStates = add_struct("gfxStates", 8);
        cGameObjectGfxStates->add("stateKey", "uint32_t", 0);
            auto cGameObjectGfxStateData = add_struct("state", 56);
                 cGameObjectGfxStateData->add("name", "key", 12);
                 cGameObjectGfxStateData->add("model", "key", 28);
                 cGameObjectGfxStateData->add("prefab", "key", 48);
                 cGameObjectGfxStateData->add("animation", "key", 44);
                 cGameObjectGfxStateData->add("animationLoops", "bool", 52);
        registerArrayType("state");
        cGameObjectGfxStates->add("state", "array", cGameObjectGfxStateData, 4);

    registerNullableType("gfxStates");
    noun->add("gfxStates", "nullable", cGameObjectGfxStates, 132);

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

    noun->add("eliteAssetIds", "array", 172);
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

    auto TriggerVolumeDef = add_struct("triggerVolume", 136);
    TriggerVolumeDef->add("onEnter", "uint32_t", 12);
    TriggerVolumeDef->add("onExit", "uint32_t", 28);
    TriggerVolumeDef->add("onStay", "uint32_t", 44);

        auto TriggerVolumeEvents = add_struct("events", 32);
        TriggerVolumeEvents->add("onEnterEvent", "uint32_t", 12);
        TriggerVolumeEvents->add("onExitEvent", "uint32_t", 28);
        registerNullableType("events");

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
    noun->add("triggerVolume", "nullable", TriggerVolumeDef, 292);

    auto LocomotionTuning = add_struct("locomotionTuning", 12);
    LocomotionTuning->add("acceleration", "float", 0);
    LocomotionTuning->add("deceleration", "float", 4);
    LocomotionTuning->add("turnRate", "float", 8);
    registerNullableType("locomotionTuning");
    noun->add("locomotionTuning", "nullable", LocomotionTuning, 304);

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
}