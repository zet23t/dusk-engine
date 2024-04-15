#include "../game_g.h"

#include "raymath.h"

static cJSON* parsedCloudCfg;

#define MAX_CLOUDS 8
typedef struct CloudSystem {
    SceneObjectId cloudList[MAX_CLOUDS];
    float cloudSpeed;
    float cloudSpawnRangeX;
    float cloudSpawnMinZ;
    float cloudSpawnMaxZ;
    float cloudSpawnMinY;
    float cloudSpawnMaxY;
    float cloudSpawnMinScale;
    float cloudSpawnMaxScale;
    float cloudDespawnZ;
} CloudSystem;

static void UpdateCloudSystem(SceneObject *object, SceneComponentId componentId, float dt, void *cloudSystemPtr)
{
    CloudSystem *cloudSystem = (CloudSystem *)cloudSystemPtr;
    if (parsedCloudCfg != psg.levelConfig) {
        parsedCloudCfg = psg.levelConfig;
        cJSON* cloudCfg = cJSON_GetObjectItem(parsedCloudCfg, "cloudCfg");
        if (cloudCfg == NULL) {
            TraceLog(LOG_ERROR, "cloudCfg not found in level config");
        } else {
            MappedVariable mappedCloudVariables[] = {
                { .name = "cloudSpeed", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSystem->cloudSpeed },
                { .name = "cloudSpawnRangeX", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSystem->cloudSpawnRangeX },
                { .name = "cloudSpawnMinZ", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSystem->cloudSpawnMinZ },
                { .name = "cloudSpawnMaxZ", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSystem->cloudSpawnMaxZ },
                { .name = "cloudSpawnMinY", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSystem->cloudSpawnMinY },
                { .name = "cloudSpawnMaxY", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSystem->cloudSpawnMaxY },
                { .name = "cloudSpawnMinScale", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSystem->cloudSpawnMinScale },
                { .name = "cloudSpawnMaxScale", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSystem->cloudSpawnMaxScale },
                { .name = "cloudDespawnZ", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSystem->cloudDespawnZ },
                { 0 }
            };

            ReadMappedVariables(cloudCfg, mappedCloudVariables);
        }
    }
    for (int i = 0; i < MAX_CLOUDS; i++) {
        SceneObjectId cloudId = cloudSystem->cloudList[i];
        SceneObject* cloudObj = SceneGraph_getObject(psg.sceneGraph, cloudId);
        if (cloudObj == 0) {
            cloudId = SceneGraph_createObject(psg.sceneGraph, "cloud");
            cloudSystem->cloudList[i] = cloudId;
            AddMeshRendererComponent(cloudId, psg.cloudList[0], 1.0f);
            SceneGraph_setLocalPosition(psg.sceneGraph, cloudId, (Vector3) { 
                GetRandomFloat(-cloudSystem->cloudSpawnRangeX, cloudSystem->cloudSpawnRangeX), 
                GetRandomFloat(cloudSystem->cloudSpawnMinY, cloudSystem->cloudSpawnMaxY), 
                GetRandomFloat(cloudSystem->cloudSpawnMinZ, cloudSystem->cloudSpawnMaxZ) });
            float scale = GetRandomFloat(cloudSystem->cloudSpawnMinScale, cloudSystem->cloudSpawnMaxScale);
            SceneGraph_setLocalScale(psg.sceneGraph, cloudId, (Vector3) { scale, scale, scale });
            SceneGraph_setLocalRotation(psg.sceneGraph, cloudId, (Vector3) { GetRandomFloat(-10, 10), GetRandomFloat(0, 360), GetRandomFloat(-10, 10) });
        } else {
            Vector3 pos = SceneGraph_getLocalPosition(psg.sceneGraph, cloudId);
            pos.z -= (cloudSystem->cloudSpeed + (i*0.1f)) * dt;
            SceneGraph_setLocalPosition(psg.sceneGraph, cloudId, pos);
            if (pos.z < cloudSystem->cloudDespawnZ) {
                SceneGraph_destroyObject(psg.sceneGraph, cloudId);
            }
        }
    }
    // DrawMesh(*psg.cloudList[0], psg.model.materials[1], MatrixIdentity());
}

void CloudSystemRegister()
{
    psg.cloudSystemId = SceneGraph_registerComponentType(psg.sceneGraph, "CloudSystem", sizeof(CloudSystem),
        (SceneComponentTypeMethods) {
            .updateTick = UpdateCloudSystem,
        });
}