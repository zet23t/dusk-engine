#include "../plane_sim_g.h"

#include "raymath.h"

static cJSON* parsedCloudCfg;

#define MAX_CLOUDS 8
static SceneObjectId cloudList[MAX_CLOUDS];
static float cloudSpeed = 0.1f;
static float cloudSpawnRangeX = 10.0f;
static float cloudSpawnMinZ = 30.0f;
static float cloudSpawnMaxZ = 60.0f;
static float cloudSpawnMinY = 10.0f;
static float cloudSpawnMaxY = 20.0f;
static float cloudSpawnMinScale = 1.5f;
static float cloudSpawnMaxScale = 3.0f;
static float cloudDespawnZ = -15.0f;

static MappedVariable mappedCloudVariables[] = {
    { .name = "cloudSpeed", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSpeed },
    { .name = "cloudSpawnRangeX", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSpawnRangeX },
    { .name = "cloudSpawnMinZ", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSpawnMinZ },
    { .name = "cloudSpawnMaxZ", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSpawnMaxZ },
    { .name = "cloudSpawnMinY", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSpawnMinY },
    { .name = "cloudSpawnMaxY", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSpawnMaxY },
    { .name = "cloudSpawnMinScale", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSpawnMinScale },
    { .name = "cloudSpawnMaxScale", .type = VALUE_TYPE_FLOAT, .floatValue = &cloudSpawnMaxScale },
    { 0 }
};

static void UpdateCloudSystem()
{
    if (parsedCloudCfg != psg.levelConfig) {
        parsedCloudCfg = psg.levelConfig;
        cJSON* cloudCfg = cJSON_GetObjectItem(parsedCloudCfg, "cloudCfg");
        if (cloudCfg == NULL) {
            TraceLog(LOG_ERROR, "cloudCfg not found in level config");
        } else {
            ReadMappedVariables(cloudCfg, mappedCloudVariables);
        }
    }
    for (int i = 0; i < MAX_CLOUDS; i++) {
        SceneObjectId cloudId = cloudList[i];
        SceneObject* cloudObj = SceneGraph_getObject(psg.sceneGraph, cloudId);
        if (cloudObj == 0) {
            cloudId = SceneGraph_createObject(psg.sceneGraph, "cloud");
            cloudList[i] = cloudId;
            AddMeshRendererComponent(cloudId, psg.cloudList[0], 1.0f);
            SceneGraph_setLocalPosition(psg.sceneGraph, cloudId, (Vector3) { 
                GetRandomFloat(-cloudSpawnRangeX, cloudSpawnRangeX), 
                GetRandomFloat(cloudSpawnMinY, cloudSpawnMaxY), 
                GetRandomFloat(cloudSpawnMinZ, cloudSpawnMaxZ) });
            float scale = GetRandomFloat(cloudSpawnMinScale, cloudSpawnMaxScale);
            SceneGraph_setLocalScale(psg.sceneGraph, cloudId, (Vector3) { scale, scale, scale });
            SceneGraph_setLocalRotation(psg.sceneGraph, cloudId, (Vector3) { GetRandomFloat(-10, 10), GetRandomFloat(0, 360), GetRandomFloat(-10, 10) });
        } else {
            Vector3 pos = SceneGraph_getLocalPosition(psg.sceneGraph, cloudId);
            pos.z -= cloudSpeed;
            SceneGraph_setLocalPosition(psg.sceneGraph, cloudId, pos);
            if (pos.z < cloudDespawnZ) {
                SceneGraph_destroyObject(psg.sceneGraph, cloudId);
            }
        }
    }
    // DrawMesh(*psg.cloudList[0], psg.model.materials[1], MatrixIdentity());
}