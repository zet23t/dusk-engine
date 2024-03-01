#include "../plane_sim_g.h"
#include <memory.h>
#include <stdlib.h>

typedef struct LevelEvent {
    float time;
    char* message;
    char* spawnPrefabId;
    int triggered;
    cJSON* overrides;
} LevelEvent;

typedef struct LevelSystem {
    float time;
    LevelEvent* events;
    int eventCount;

    cJSON* objects;
} LevelSystem;

static void LevelSystemInit(LevelSystem* levelSystem)
{
    levelSystem->time = 0;
    levelSystem->events = NULL;
    levelSystem->eventCount = 0;

    cJSON* level = cJSON_GetObjectItemCaseSensitive(psg.levelConfig, "level");
    if (level == NULL) {
        TraceLog(LOG_ERROR, "No level object in level config (how ironic)");
        return;
    }

    levelSystem->objects = cJSON_GetObjectItemCaseSensitive(level, "objects");
    if (levelSystem->objects == NULL) {
        TraceLog(LOG_ERROR, "No objects object in level config");
        return;
    }

    cJSON* events = cJSON_GetObjectItemCaseSensitive(level, "events");
    if (!cJSON_IsArray(events)) {
        TraceLog(LOG_INFO, "No events array in level config");
        return;
    }

    levelSystem->eventCount = cJSON_GetArraySize(events);
    levelSystem->events = (LevelEvent*)malloc(sizeof(LevelEvent) * levelSystem->eventCount);
    memset(levelSystem->events, 0, sizeof(LevelEvent) * levelSystem->eventCount);
    for (int i = 0; i < levelSystem->eventCount; i++) {
        cJSON* event = cJSON_GetArrayItem(events, i);
        if (!cJSON_IsObject(event)) {
            TraceLog(LOG_ERROR, "level event loading: Event %d is not an object", i);
            continue;
        }
        cJSON* time = cJSON_GetObjectItemCaseSensitive(event, "time");
        if (!cJSON_IsNumber(time)) {
            TraceLog(LOG_ERROR, "level event loading: Event %d has no time", i);
            continue;
        }
        levelSystem->events[i].time = (float)time->valuedouble;
        levelSystem->events[i].message = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(event, "message"));
        levelSystem->events[i].spawnPrefabId = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(event, "prefab"));
        levelSystem->events[i].overrides = event;
    }
}

static void LevelSystemDestroy(SceneObject* node, SceneComponentId sceneComponentId, void* component)
{
    LevelSystem* levelSystem = (LevelSystem*)component;

    if (levelSystem->events != NULL) {
        free(levelSystem->events);
    }
}

static void LevelSystemUpdate(SceneObject* node, SceneComponentId sceneComponentId, float dt, void* component)
{
    LevelSystem* levelSystem = (LevelSystem*)component;

    if (levelSystem->objects == NULL) {
        LevelSystemInit(levelSystem);
        return;
    }

    levelSystem->time += dt;
    for (int i = 0; i < levelSystem->eventCount; i += 1) {
        if (levelSystem->events[i].triggered || levelSystem->events[i].time > levelSystem->time) {
            continue;
        }

        levelSystem->events[i].triggered = 1;
        if (levelSystem->events[i].message != NULL) {
            TraceLog(LOG_INFO, "Level event: %s", levelSystem->events[i].message);
        }

        if (levelSystem->events[i].spawnPrefabId != NULL) {
            cJSON* prefab = cJSON_GetObjectItemCaseSensitive(levelSystem->objects, levelSystem->events[i].spawnPrefabId);
            if (prefab == NULL) {
                TraceLog(LOG_ERROR, "Prefab %s not found in level config", levelSystem->events[i].spawnPrefabId);
                continue;
            }
            SceneObjectId objectId = InstantiateFromJSON(node->graph, levelSystem->objects, levelSystem->events[i].spawnPrefabId);
            SceneObject_ApplyJSONValues(node->graph, levelSystem->objects, objectId, levelSystem->events[i].overrides);
        }
    }
}

void LevelSystemRegister()
{
    psg.levelSystemId = SceneGraph_registerComponentType(psg.sceneGraph, "LevelSystem", sizeof(LevelSystem),
        (SceneComponentTypeMethods) {
            .updateTick = LevelSystemUpdate,
            .onDestroy = LevelSystemDestroy,
        });
}