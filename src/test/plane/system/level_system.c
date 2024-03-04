#include "../plane_sim_g.h"
#include <memory.h>
#include <stdlib.h>

typedef struct LevelEvent {
    float time;
    char* message;
    char* showMessage;
    char* spawnPrefabId;
    int triggered;
    float messageDuration;
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
        levelSystem->events[i].showMessage = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(event, "showMessage"));
        levelSystem->events[i].messageDuration = (float)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(event, "messageDuration"));
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
        LevelEvent* event = &levelSystem->events[i];
        if (event->triggered || event->time > levelSystem->time) {
            continue;
        }

        event->triggered = 1;
        if (event->message != NULL) {
            TraceLog(LOG_INFO, "Level event: %s", event->message);
        }

        if (event->showMessage != NULL) {
            TraceLog(LOG_INFO, "Level event, show message: %s", event->showMessage);
            SceneObjectId messageId = SceneGraph_createObject(node->graph, "message");
            SceneGraph_setParent(node->graph, messageId, psg.uiRootId);
            SceneGraph_setLocalPosition(node->graph, messageId, (Vector3) { 0, 5, 0 });
            SceneGraph_addComponent(node->graph, messageId, psg.textComponentId, &(TextComponent) {
                .color = (Color) { 255, 255, 255, 255 },
                .text = event->showMessage,
                .align = (Vector2) {.5f, .0f},
                .fontSize = 10,
                .fontSpacing = 2
            });
            SceneComponentId timerId = AddTimerComponent(messageId, event->messageDuration, 0);
            TimerComponentAddAction(timerId, ACTION_TYPE_DESTROY_OBJECT, "");
            // AddAutoDestroyComponent(messageId, event->messageDuration);
        }

        if (event->spawnPrefabId != NULL) {
            cJSON* prefab = cJSON_GetObjectItemCaseSensitive(levelSystem->objects, event->spawnPrefabId);
            if (prefab == NULL) {
                TraceLog(LOG_ERROR, "Prefab %s not found in level config", event->spawnPrefabId);
                continue;
            }
            SceneObjectId objectId = InstantiateFromJSON(node->graph, levelSystem->objects, event->spawnPrefabId);
            SceneObject_ApplyJSONValues(node->graph, levelSystem->objects, objectId, event->overrides);
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