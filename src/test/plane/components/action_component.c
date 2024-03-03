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
            int n = 0;
            for (int i = 0; i < arrayLengh; i++)
            {
                cJSON *actionCfg = cJSON_GetArrayItem(actions, i);
                cJSON *type = cJSON_GetObjectItemCaseSensitive(actionCfg, "type");
                Action action = {0};
                if (cJSON_IsString(type))
                {
                    if (strcmp(type->valuestring, "enable_object") == 0)
                    {
                        action.actionType = ACTION_TYPE_ENABLE_OBJECT;
                    }
                    else if (strcmp(type->valuestring, "disable_object") == 0)
                    {
                        action.actionType = ACTION_TYPE_DISABLE_OBJECT;
                    }
                    else if (strcmp(type->valuestring, "destroy_object") == 0)
                    {
                        action.actionType = ACTION_TYPE_DESTROY_OBJECT;
                    }
                    else if (strcmp(type->valuestring, "enable_component") == 0)
                    {
                        action.actionType = ACTION_TYPE_ENABLE_COMPONENT;
                    }
                    else if (strcmp(type->valuestring, "disable_component") == 0)
                    {
                        action.actionType = ACTION_TYPE_DISABLE_COMPONENT;
                    }
                    else if (strcmp(type->valuestring, "destroy_component") == 0)
                    {
                        action.actionType = ACTION_TYPE_DESTROY_COMPONENT;
                    }
                    else if (strcmp(type->valuestring, "enable_child_object") == 0)
                    {
                        action.actionType = ACTION_TYPE_ENABLE_CHILD_OBJECT;
                    }
                    else if (strcmp(type->valuestring, "disable_child_object") == 0)
                    {
                        action.actionType = ACTION_TYPE_DISABLE_CHILD_OBJECT;
                    }
                    else if (strcmp(type->valuestring, "destroy_child_object") == 0)
                    {
                        action.actionType = ACTION_TYPE_DESTROY_CHILD_OBJECT;
                    }
                    else if (strcmp(type->valuestring, "enable_child_component") == 0)
                    {
                        action.actionType = ACTION_TYPE_ENABLE_CHILD_COMPONENT;
                    }
                    else if (strcmp(type->valuestring, "disable_child_component") == 0)
                    {
                        action.actionType = ACTION_TYPE_DISABLE_CHILD_COMPONENT;
                    }
                    else if (strcmp(type->valuestring, "destroy_child_component") == 0)
                    {
                        action.actionType = ACTION_TYPE_DESTROY_CHILD_COMPONENT;
                    }
                    else
                    {
                        TraceLog(LOG_ERROR, "Unknown action type: %s", type->valuestring);
                        continue;
                    }
                }
                else
                {
                    TraceLog(LOG_ERROR, "Action type is not a string");
                    continue;
                }
                cJSON *target = cJSON_GetObjectItemCaseSensitive(actionCfg, "targetName");
                
                if (cJSON_IsString(target))
                {
                    action.targetName = strdup(target->valuestring);
                }
            }
        }
    }
}

void ActionComponentRegister()
{
    psg.actionComponentTypeId = SceneGraph_registerComponentType(psg.sceneGraph, "ActionComponent", sizeof(ActionComponent),
        (SceneComponentTypeMethods) {
            .onInitialize = ActionComponent_onInitialize,
        });
}