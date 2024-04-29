#undef COMPONENT_NAME
#define COMPONENT_NAME PlaneBehaviorComponent

#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(PlaneBehaviorComponent)
#elif defined(COMPONENT_DECLARATION)



#else

// === DEFINITIONS ===

#include "../game_g.h"
#include <raymath.h>

static void PlaneBehaviorComponentUpdateTick(SceneObject* sceneObject, SceneComponentId sceneComponentId,
    float delta, void* componentData)
{
    if (delta <= 0.0f) {
        return;
    }
    PlaneBehaviorComponent* planeBehavior = (PlaneBehaviorComponent*)componentData;

    LinearVelocityComponent* velocity;
    SceneGraph_getComponentOrFindByType(psg.sceneGraph, sceneObject->id, &planeBehavior->velocityComponentId, psg.LinearVelocityComponentId, (void**)&velocity);
    if (velocity == NULL) {
        return;
    }
    planeBehavior->time += delta;

    Vector3 accel = Vector3Scale(Vector3Subtract(velocity->velocity, planeBehavior->prevVelocity), 1 / delta);
    float accX = accel.x * .125 + planeBehavior->smoothedXAccel * .875f;
    planeBehavior->smoothedXAccel = accX;
    // float accmax = 5.0f;
    // accel.x = fmaxf(-accmax, fminf(accel.x, accmax));
    planeBehavior->rolling = planeBehavior->rolling * .85f - accX * .25f;

    float yaw = sinf(planeBehavior->time * 3 + planeBehavior->phase * .5f);
    float roll = sinf(planeBehavior->time * 2.3f + planeBehavior->phase) + yaw * .25f;
    float pitch = sinf(planeBehavior->time * 2.7f + planeBehavior->phase * .7f) + yaw * .5f;

    SceneGraph_setLocalRotation(sceneObject->graph, sceneObject->id,
        (Vector3) {
            pitch * 3 + fabsf(planeBehavior->rolling) * .015f,
            yaw * 2 - planeBehavior->rolling * .25f,
            roll * 5 + planeBehavior->rolling });
    SceneGraph_setLocalRotation(psg.sceneGraph, planeBehavior->propeller, (Vector3) { 0, 0, -planeBehavior->time * 360 * 4 });

    planeBehavior->prevVelocity = velocity->velocity;
}

#include "../util/component_macros.h"

BEGIN_COMPONENT_REGISTRATION(PlaneBehaviorComponent) {
    .updateTick = PlaneBehaviorComponentUpdateTick,
} END_COMPONENT_REGISTRATION

#endif