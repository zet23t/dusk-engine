#include "../game_g.h"

static void UpdateCallbackComponentUpdate(SceneObject* node, SceneComponentId sceneComponentId, float dt, void* component)
{
    UpdateCallbackComponent* updateCallback = (UpdateCallbackComponent*)component;
    updateCallback->update(node->graph, node->id, sceneComponentId, dt, updateCallback);
}

void UpdateCallbackComponentRegister()
{
    psg.updateCallbackComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "UpdateCallback", sizeof(UpdateCallbackComponent),
        (SceneComponentTypeMethods) {
            .updateTick = UpdateCallbackComponentUpdate,
        });
}