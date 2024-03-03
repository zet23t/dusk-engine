#include "../plane_sim_g.h"
#include <string.h>

void ActionComponent_onInitialize(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void* initArg)
{
    ActionComponent* actionComponent = (ActionComponent*)componentData;
    ComponentInitializer* initializer = (ComponentInitializer*)initArg;
    if (initializer->memData != NULL)
    {
        memcpy(actionComponent, initializer->memData, sizeof(ActionComponent));
    }
    else
    {
        memset(actionComponent, 0, sizeof(ActionComponent));
        actionComponent->disableOnTrigger = 1;
        cJSON *cfg = initializer->config;
        cJSON *actions = cJSON_GetObjectItemCaseSensitive(cfg, "actions");
        if (cJSON_IsArray(actions))
        {
            int arrayLengh = cJSON_GetArraySize(actions);
            actionComponent->actions = (Action*)malloc(sizeof(Action) * arrayLengh);
            memset(actionComponent->actions, 0, sizeof(Action) * arrayLengh);
            int n = 0;
            for (int i = 0; i < arrayLengh; i++)
            {
                Action action = {0};
                cJSON *actionCfg = cJSON_GetArrayItem(actions, i);
                if (ActionFromJSON(actionCfg, &action) == 0)
                {
                    actionComponent->actions[n++] = action;
                    continue;
                }
            }
            actionComponent->actionCount = n;
        }
    }
}

void ActionComponent_onDestroy(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData)
{
    ActionComponent* actionComponent = (ActionComponent*)componentData;
    for (int i = 0; i < actionComponent->actionCount; i++)
    {
        free(actionComponent->actions[i].targetName);
    }
    free(actionComponent->actions);
}

void ActionComponentRegister()
{
    psg.actionComponentTypeId = SceneGraph_registerComponentType(psg.sceneGraph, "ActionComponent", sizeof(ActionComponent),
        (SceneComponentTypeMethods) {
            .onInitialize = ActionComponent_onInitialize,
            .onDestroy = ActionComponent_onDestroy,
        });
}