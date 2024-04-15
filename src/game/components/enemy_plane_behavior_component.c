#include "../game_g.h"

static void EnemyPlaneBehaviorComponentUpdate(SceneObject* node, SceneComponentId sceneComponentId, float dt, void* component)
{
    // EnemyPlaneBehaviorComponent* enemyPlaneBehavior = (EnemyPlaneBehaviorComponent*)component;
    // enemyPlaneBehavior->timeSinceLastShot += dt;
    // if (enemyPlaneBehavior->timeSinceLastShot >= enemyPlaneBehavior->shotInterval) {
    //     enemyPlaneBehavior->timeSinceLastShot = 0;
    //     SceneObject* bullet = SceneGraph_createObject(psg.sceneGraph);
    //     SceneGraph_addComponent(psg.sceneGraph, bullet->id, psg.bulletComponentId, &(BulletComponent) {0});
    //     SceneGraph_addComponent(psg.sceneGraph, bullet->id, psg.transformComponentId, &(TransformComponent) {
    //         .position = enemyPlaneBehavior->position,
    //         .rotation = enemyPlaneBehavior->rotation,
    //     });
    //     SceneGraph_addComponent(psg.sceneGraph, bullet->id, psg.velocityComponentId, &(VelocityComponent) {
    //         .velocity = enemyPlaneBehavior->direction,
    //     });
    // }
}

void EnemyPlaneBehaviorComponentRegister()
{
    psg.enemyPlaneBehaviorComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "EnemyPlaneBehavior", sizeof(EnemyPlaneBehaviorComponent),
        (SceneComponentTypeMethods) {
            .updateTick = EnemyPlaneBehaviorComponentUpdate,
        });
}