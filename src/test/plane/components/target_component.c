#include "../plane_sim_g.h"

static void TargetComponentUpdate(SceneObject* node, SceneComponentId sceneComponentId, float dt, void* component)
{
    TargetComponent* target = (TargetComponent*)component;
    int mask = target->colliderMask;
    SceneComponentType *bulletType = SceneGraph_getComponentType(psg.sceneGraph, psg.bulletComponentId);
    BulletComponent* bulletComponents = (BulletComponent*)bulletType->componentData;
    Vector3 pos = SceneGraph_getWorldPosition(psg.sceneGraph, node->id);
    for (int i=0; i < bulletType->components_count; i++) {
        SceneComponent *bulletComponent = &bulletType->components[i];
        if (bulletComponent->id.version == 0) continue;
        BulletComponent *bulletData = &bulletComponents[i];
        if (bulletData->colliderFlags & mask) {
            float r = target->radius + bulletData->radius;
            float r2 = r * r;
            Vector3 bulletPos = SceneGraph_getWorldPosition(psg.sceneGraph, bulletComponent->objectId);
            float dx,dy,dz;
            dx = bulletPos.x - pos.x;
            dy = bulletPos.y - pos.y;
            dz = bulletPos.z - pos.z;
            float d2 = dx*dx + dy*dy + dz*dz;
            if (d2 < r2 && !target->onHit(node->graph, node->id, bulletComponent->objectId)) {
                break;
            }
        }
    }
}

void TargetComponentRegister()
{
    psg.targetComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "Target", sizeof(TargetComponent),
        (SceneComponentTypeMethods) {
            .updateTick = TargetComponentUpdate,
        });
}