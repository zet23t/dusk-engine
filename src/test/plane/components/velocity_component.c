#include "../plane_sim_g.h"
#include <raymath.h>

static float calculateFrameTimeIndependentDragFactor(float dragFactor, float deltaTime)
{
    return 1 / (1 + (deltaTime * dragFactor));
}

static void LinearVelocityComponentUpdateTick(SceneObject* sceneObject, SceneComponentId sceneComponentId,
    float delta, void* componentData)
{
    LinearVelocityComponent* velocity = (LinearVelocityComponent*)componentData;
    velocity->velocity.x *= calculateFrameTimeIndependentDragFactor(velocity->drag.x, psg.deltaTime);
    velocity->velocity.y *= calculateFrameTimeIndependentDragFactor(velocity->drag.y, psg.deltaTime);
    velocity->velocity.z *= calculateFrameTimeIndependentDragFactor(velocity->drag.z, psg.deltaTime);
    
    Vector3 position = SceneGraph_getLocalPosition(psg.sceneGraph, sceneObject->id);
    position = Vector3Add(position, Vector3Scale(velocity->velocity, psg.deltaTime));
    SceneGraph_setLocalPosition(psg.sceneGraph, sceneObject->id, position);
}

void LinearVelocityComponentRegister()
{
    psg.linearVelocityComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "LinearVelocity", sizeof(LinearVelocityComponent),
        (SceneComponentTypeMethods) {
            .updateTick = LinearVelocityComponentUpdateTick,
        });
}