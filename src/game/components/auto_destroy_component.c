#include "../game_g.h"

static void AutoDestroyComponentUpdate(SceneObject* node, SceneComponentId sceneComponentId, float dt, void* component)
{
    AutoDestroyComponent* autoDestroy = (AutoDestroyComponent*)component;
    autoDestroy->lifeTimeLeft -= psg.deltaTime;
    if (autoDestroy->lifeTimeLeft <= 0) {
        SceneGraph_destroyObject(psg.sceneGraph, node->id);
    }
}

static void AutoDestroyComponentInitialize(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void *arg)
{
    AutoDestroyComponent* autoDestroy = (AutoDestroyComponent*)componentData;
    ComponentInitializer *ci = (ComponentInitializer*) arg;
    if (ci->memData)
    {
        *autoDestroy = *(AutoDestroyComponent*)ci->memData;
        return;
    }
    autoDestroy->lifeTimeLeft = 0;
    if (ci->config)
    {
        MappedVariable mapped[] = {
            {.name="lifeTime", .type=VALUE_TYPE_FLOAT, .floatValue=&autoDestroy->lifeTimeLeft},
            {0}
        };
        ReadMappedVariables(ci->config, mapped);
        return;
    }
}

void AddAutoDestroyComponent(SceneObjectId objectId, float lifeTime)
{
    AutoDestroyComponent component = {
        .lifeTimeLeft = lifeTime,
    };
    SceneGraph_addComponent(psg.sceneGraph, objectId, psg.autoDestroyComponentId, &(ComponentInitializer) {
        .memData = &component,
    });
}

void AutoDestroyComponentRegister()
{
    psg.autoDestroyComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "AutoDestroyComponent", sizeof(AutoDestroyComponent),
        (SceneComponentTypeMethods) {
            .updateTick = AutoDestroyComponentUpdate,
            .onInitialize = AutoDestroyComponentInitialize,
        });
}