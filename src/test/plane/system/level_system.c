#include "../plane_sim_g.h"
#include <memory.h>
#include <stdlib.h>

static void LevelSystemUpdate(SceneObject* node, SceneComponentId sceneComponentId, float dt, void* component)
{
    LevelSystem* levelSystem = (LevelSystem*)component;

    levelSystem->time += dt;
    
    for (LevelEvent *event = levelSystem->events; event->onTrigger != NULL; event++) {
        if (event->triggered || event->time > levelSystem->time) {
            continue;
        }

        event->triggered = 1;
        event->onTrigger(node->graph, event->userData);

        // if (event->showMessage != NULL) {
        //     TraceLog(LOG_INFO, "Level event, show message: %s", event->showMessage);
        //     SceneObjectId messageId = SceneGraph_createObject(node->graph, "message");
        //     SceneGraph_setParent(node->graph, messageId, psg.uiRootId);
        //     SceneGraph_setLocalPosition(node->graph, messageId, (Vector3) { 0, 5, 0 });
        //     SceneGraph_addComponent(node->graph, messageId, psg.textComponentId, &(TextComponent) {
        //         .color = (Color) { 255, 255, 255, 255 },
        //         .text = event->showMessage,
        //         .align = (Vector2) {.5f, .0f},
        //         .fontSize = 10,
        //         .fontSpacing = 2
        //     });
        //     SceneComponentId timerId = AddTimerComponent(messageId, event->messageDuration, 0);
        //     TimerComponentAddAction(timerId, ACTION_TYPE_DESTROY_OBJECT, "");
        //     // AddAutoDestroyComponent(messageId, event->messageDuration);
        // }

        // if (event->spawnPrefabId != NULL) {
        //     cJSON* prefab = cJSON_GetObjectItemCaseSensitive(levelSystem->objects, event->spawnPrefabId);
        //     if (prefab == NULL) {
        //         TraceLog(LOG_ERROR, "Prefab %s not found in level config", event->spawnPrefabId);
        //         continue;
        //     }
        //     SceneObjectId objectId = InstantiateFromJSON(node->graph, levelSystem->objects, event->spawnPrefabId);
        //     SceneObject_ApplyJSONValues(node->graph, levelSystem->objects, objectId, event->overrides);
        // }
    }
}

void LevelSystemRegister()
{
    psg.levelSystemId = SceneGraph_registerComponentType(psg.sceneGraph, "LevelSystem", sizeof(LevelSystem),
        (SceneComponentTypeMethods) {
            .updateTick = LevelSystemUpdate,
        });
}