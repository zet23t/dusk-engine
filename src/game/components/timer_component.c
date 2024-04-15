#include "../game_g.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void TimerComponent_onInitialize(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void* initArg)
{
    TimerComponent* timerComponent = (TimerComponent*)componentData;
    ComponentInitializer* initializer = (ComponentInitializer*)initArg;
    if (initializer->memData != NULL)
    {
        memcpy(timerComponent, initializer->memData, sizeof(TimerComponent));
    }
    else
    {
        memset(timerComponent, 0, sizeof(TimerComponent));
        TraceLog(LOG_ERROR, "implement this");
        exit(1);
        // cJSON *cfg = initializer->config;
        // cJSON *timerCfg = cJSON_GetObjectItemCaseSensitive(cfg, "timer");
        // if (cJSON_IsObject(timerCfg))
        // {
        //     cJSON *duration = cJSON_GetObjectItemCaseSensitive(timerCfg, "duration");
        //     if (cJSON_IsNumber(duration))
        //     {
        //         timerComponent->duration = duration->valuedouble;
        //     }
        //     cJSON *repeat = cJSON_GetObjectItemCaseSensitive(timerCfg, "repeat");
        //     if (cJSON_IsBool(repeat))
        //     {
        //         timerComponent->repeat = cJSON_IsTrue(repeat);
        //     }
        // }
    }
}

void TimerComponent_onDestroy(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData)
{
    TimerComponent* timerComponent = (TimerComponent*)componentData;
    for (int i=0;i<timerComponent->actionCount;i++)
    {
        free(timerComponent->actions[i].targetName);
    }
    free(timerComponent->actions);
}

void TimerComponent_onUpdate(SceneObject* sceneObject, SceneComponentId sceneComponentId, float deltaTime, void* componentData)
{
    TimerComponent* timerComponent = (TimerComponent*)componentData;
    timerComponent->time += deltaTime;
    if (timerComponent->time >= timerComponent->triggerTime)
    {
        timerComponent->time -= timerComponent->triggerTime;
        TriggerActions(psg.sceneGraph, sceneObject->id, timerComponent->actions, timerComponent->actionCount);
        if (timerComponent->repeatCount-- == 0)
        {
            SceneGraph_destroyComponent(psg.sceneGraph, sceneComponentId);
        }
    }
}

void TimerComponentAddAction(SceneComponentId componentId, int actionType, const char* targetName)
{
    TimerComponent* timerComponent = NULL;
    SceneComponent* component = SceneGraph_getComponent(psg.sceneGraph, componentId, (void**)&timerComponent);
    if (component == NULL)
    {
        TraceLog(LOG_ERROR, "TimerComponentAddAction: component not found");
        return;
    }
    timerComponent->actionCount++;
    timerComponent->actions = (Action*)realloc(timerComponent->actions, sizeof(Action) * timerComponent->actionCount);
    Action* action = &timerComponent->actions[timerComponent->actionCount - 1];
    action->targetName = strdup(targetName);
    action->actionType = actionType;
}

SceneComponentId AddTimerComponent(SceneObjectId objectId, float duration, int repeat)
{
    SceneComponentId componentId = SceneGraph_addComponent(psg.sceneGraph, objectId, psg.timerComponentTypeId, &(ComponentInitializer) {
        .memData = &(TimerComponent) {
            .triggerTime = duration,
            .repeatCount = repeat,
            .actionCount = 0,
            .actions = NULL,
            .time = 0,
        }
    });
    return componentId;
}

void TimerComponentRegister()
{
    psg.timerComponentTypeId = SceneGraph_registerComponentType(psg.sceneGraph, "TimerComponent", sizeof(TimerComponent),
        (SceneComponentTypeMethods) {
            .onInitialize = TimerComponent_onInitialize,
            .onDestroy = TimerComponent_onDestroy,
            .updateTick = TimerComponent_onUpdate,
        });
}

