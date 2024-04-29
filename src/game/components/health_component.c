#include "../game_g.h"

static void HealthComponentUpdate(SceneObject* node, SceneComponentId sceneComponentId, float dt, void* component)
{
    HealthComponent* health = (HealthComponent*)component;
    if (health->health <= 0) {
        SceneGraph_destroyObject(psg.sceneGraph, node->id);
    }
}

static void HealthComponentInitialize(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void *arg)
{
    HealthComponent* health = (HealthComponent*)componentData;
    ComponentInitializer *ci = (ComponentInitializer*) arg;
    if (ci->memData)
    {
        *health = *(HealthComponent*)ci->memData;
        return;
    }
    health->health = 1;
    health->maxHealth = 1;
    if (ci->config)
    {
        MappedVariable mapped[] = {
            {.name="health", .type=VALUE_TYPE_FLOAT, .floatValue=&health->health},
            {.name="maxHealth", .type=VALUE_TYPE_FLOAT, .floatValue=&health->maxHealth},
            {0}
        };
        ReadMappedVariables(ci->config, mapped);
        return;
    }
}

void AddHealthComponent(SceneObjectId objectId, int health, int maxHealth)
{
    HealthComponent component = {
        .health = health,
        .maxHealth = maxHealth,
    };
    SceneGraph_addComponent(psg.sceneGraph, objectId, psg.HealthComponentId, &(ComponentInitializer) {
        .memData = &component,
    });
}