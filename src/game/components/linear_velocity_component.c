#include "../game_g.h"
#include <raymath.h>

static float calculateFrameTimeIndependentDragFactor(float dragFactor, float deltaTime)
{
    return 1 / (1 + (deltaTime * dragFactor));
}

static void LinearVelocityComponentInitialize(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void *arg)
{
    LinearVelocityComponent* velocity = (LinearVelocityComponent*)componentData;
    ComponentInitializer *ci = (ComponentInitializer*) arg;
    if (ci->memData)
    {
        *velocity = *(LinearVelocityComponent*)ci->memData;
        return;
    }
    
    velocity->velocity = (Vector3) { 0, 0, 0 };
    velocity->acceleration = (Vector3) { 0, 0, 0 };
    velocity->drag = (Vector3) { 0, 0, 0 };
    if (ci->config)
    {
        MappedVariable mapped[] = {
            {.name="drag", .type=VALUE_TYPE_VEC3, .vec3Value=&velocity->drag},
            {.name="acceleration", .type=VALUE_TYPE_VEC3, .vec3Value=&velocity->acceleration},
            {.name="velocity", .type=VALUE_TYPE_VEC3, .vec3Value=&velocity->velocity},
            {0}
        };
        ReadMappedVariables(ci->config, mapped);
        return;
    }
}

static void LinearVelocityComponentUpdateTick(SceneObject* sceneObject, SceneComponentId sceneComponentId,
    float delta, void* componentData)
{
    LinearVelocityComponent* velocity = (LinearVelocityComponent*)componentData;
    velocity->velocity.x *= calculateFrameTimeIndependentDragFactor(velocity->drag.x, psg.deltaTime);
    velocity->velocity.y *= calculateFrameTimeIndependentDragFactor(velocity->drag.y, psg.deltaTime);
    velocity->velocity.z *= calculateFrameTimeIndependentDragFactor(velocity->drag.z, psg.deltaTime);

    // if I am not mistaken, this is a simple Euler integration for acceleration on velocity
    velocity->velocity.x += velocity->acceleration.x * psg.deltaTime * .5f;
    velocity->velocity.y += velocity->acceleration.y * psg.deltaTime * .5f;
    velocity->velocity.z += velocity->acceleration.z * psg.deltaTime * .5f;
    Vector3 position = SceneGraph_getLocalPosition(psg.sceneGraph, sceneObject->id);
    position = Vector3Add(position, Vector3Scale(velocity->velocity, psg.deltaTime));
    SceneGraph_setLocalPosition(psg.sceneGraph, sceneObject->id, position);
    velocity->velocity.x += velocity->acceleration.x * psg.deltaTime * .5f;
    velocity->velocity.y += velocity->acceleration.y * psg.deltaTime * .5f;
    velocity->velocity.z += velocity->acceleration.z * psg.deltaTime * .5f;
}

SceneComponentId AddLinearVelocityComponent(SceneObjectId objectId, Vector3 velocity, Vector3 acceleration, Vector3 drag)
{
    LinearVelocityComponent component = {
        .velocity = velocity,
        .acceleration = acceleration,
        .drag = drag,
    };
    return SceneGraph_addComponent(psg.sceneGraph, objectId, psg.LinearVelocityComponentId, &(ComponentInitializer) {
        .memData = &component,
    });
}

Vector3 GetVelocity(SceneObjectId objectId)
{
    LinearVelocityComponent* velocity;
    SceneComponent* component = SceneGraph_getComponentByType(psg.sceneGraph, objectId, psg.LinearVelocityComponentId, (void**)&velocity, 0);
    if (component == NULL)
    {
        return (Vector3) { 0, 0, 0 };
    }
    return velocity->velocity;

}