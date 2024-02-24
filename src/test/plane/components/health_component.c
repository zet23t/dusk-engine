#include "../plane_sim_g.h"

static void HealthComponentUpdate(SceneObject* node, SceneComponentId sceneComponentId, float dt, void* component)
{
    HealthComponent* health = (HealthComponent*)component;
    if (health->health <= 0) {
        SceneGraph_destroyObject(psg.sceneGraph, node->id);
    }
}

void HealthComponentRegister()
{
    psg.healthComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "Health", sizeof(HealthComponent),
        (SceneComponentTypeMethods) {
            .updateTick = HealthComponentUpdate,
        });
}