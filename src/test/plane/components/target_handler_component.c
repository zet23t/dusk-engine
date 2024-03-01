#include "../plane_sim_g.h"

typedef struct TargetHandlerComponent {
    cJSON* objects;
    const char* onHitSpawn;
} TargetHandlerComponent;

static int TargetHandlerComponentOnHit(SceneGraph* graph, SceneObjectId target, SceneObjectId bullet)
{
    TraceLog(LOG_INFO, "Target hit");
    HealthComponent* healthComponent;
    if (SceneGraph_getComponentByType(graph, target, psg.healthComponentId, (void**)&healthComponent, 0)) {
        healthComponent->health -= 1;
    } else
        SceneGraph_destroyObject(graph, target);

    Vector3 position = SceneGraph_getLocalPosition(graph, bullet);
    Vector3 velocity = { 0, 0, 0 };
    LinearVelocityComponent* velocityComponent;
    if (SceneGraph_getComponentByType(graph, bullet, psg.linearVelocityComponentId, (void**)&velocityComponent, 0)) {
        velocity = velocityComponent->velocity;
    }
    
    SceneGraph_destroyObject(graph, bullet);
    return 1;
}

static void TargetHandlerComponentInitialize(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void* arg)
{
    TargetHandlerComponent* targetHandler = (TargetHandlerComponent*)componentData;
    memset(targetHandler, 0, sizeof(TargetHandlerComponent));

    targetHandler->objects = ((ComponentInitializer*)arg)->objects;
    cJSON* config = ((ComponentInitializer*)arg)->config;

    if (config) {
        TargetComponent targetComponent = (TargetComponent) { 0 };
        MappedVariable mapped[] = {
            { .name = "onHitSpawn", .type = VALUE_TYPE_STRING, .stringValue = &targetHandler->onHitSpawn },
            { .name = "colliderMask", .type = VALUE_TYPE_INT32, .uint32Value = &targetComponent.colliderMask, .isRequired = 1},
            { .name = "radius", .type = VALUE_TYPE_FLOAT, .floatValue = &targetComponent.radius, .isRequired = 1},
            { 0 }
        };
        ReadMappedVariables(config, mapped);

        targetComponent.onHit = TargetHandlerComponentOnHit;

        TraceLog(LOG_INFO, "TargetHandlerComponentInitialize: %s %d %f", targetHandler->onHitSpawn, targetComponent.colliderMask, targetComponent.radius);

        SceneGraph_addComponent(psg.sceneGraph, sceneObject->id, psg.targetComponentId, &targetComponent);
    }
}

void TargetHandlerComponentRegister()
{
    psg.targetHandlerComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "TargetHandlerComponent", sizeof(TargetHandlerComponent),
        (SceneComponentTypeMethods) {
            .onInitialize = TargetHandlerComponentInitialize,
        });
}