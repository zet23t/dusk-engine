#include "../plane_sim_g.h"

static void AutoDestroyComponentUpdate(SceneObject* node, SceneComponentId sceneComponentId, float dt, void* component)
{
    AutoDestroyComponent* autoDestroy = (AutoDestroyComponent*)component;
    autoDestroy->lifeTimeLeft -= psg.deltaTime;
    if (autoDestroy->lifeTimeLeft <= 0) {
        SceneGraph_destroyObject(psg.sceneGraph, node->id);
    }
}

void AutoDestroyComponentRegister()
{
    psg.autoDestroyComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "AutoDestroy", sizeof(AutoDestroyComponent),
        (SceneComponentTypeMethods) {
            .updateTick = AutoDestroyComponentUpdate,
        });
}