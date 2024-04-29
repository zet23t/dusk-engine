#include "../game_g.h"

#include <math.h>
#include "../util/util_math.h"

static void onParticleUpdate(SceneGraph* graph, SceneObjectId objectId, SceneComponentId componentId, float dt, struct UpdateCallbackComponent* component)
{
    Vector3 scale = SceneGraph_getLocalScale(graph, objectId);
    scale.x = fmaxf(0, scale.x - dt * 2);
    scale.y = fmaxf(0, scale.y - dt * 2);
    scale.z = fmaxf(0, scale.z - dt * 2);
    SceneGraph_setLocalScale(graph, objectId, scale);
}

static void spawnHitEffect(Vector3 position, Vector3 impactVelocity)
{
    const float vSpread = 10.0f;
    const float drag = 5.0f;
    for (int i = 0; i < 5; i++) {
        SceneObjectId effect = SceneGraph_createObject(psg.sceneGraph, "hit_effect");
        SceneGraph_setLocalPosition(psg.sceneGraph, effect, position);
        AddMeshRendererComponent(effect, psg.meshHitParticle1, 1.0f);
        AddAutoDestroyComponent(effect, 1.0f);
        
        AddLinearVelocityComponent(effect, 
                (Vector3) {
                    GetRandomFloat(-vSpread, vSpread) - impactVelocity.x * .5f,
                    GetRandomFloat(-vSpread, vSpread) - impactVelocity.y * .5f,
                    GetRandomFloat(-vSpread, vSpread) - impactVelocity.z * .5f,
                },
                (Vector3) { 0, -9.8f, 0 },
                (Vector3) { drag, drag, drag }
        );

        SceneGraph_addComponent(psg.sceneGraph, effect, psg.updateCallbackComponentId,
            &(UpdateCallbackComponent) {
                .update = onParticleUpdate,
            });
    }
}

static int onTargetHit(SceneGraph* graph, SceneObjectId target, SceneObjectId bullet)
{
    HealthComponent* healthComponent;
    if (SceneGraph_getComponentByType(graph, target, psg.HealthComponentId, (void**)&healthComponent, 0)) {
        healthComponent->health -= 1;
    } else
        SceneGraph_destroyObject(graph, target);

    Vector3 position = SceneGraph_getLocalPosition(graph, bullet);
    LinearVelocityComponent* velocity;
    if (SceneGraph_getComponentByType(graph, bullet, psg.LinearVelocityComponentId, (void**)&velocity, 0)) {
        spawnHitEffect(position, velocity->velocity);
    } else
        spawnHitEffect(position, (Vector3) { 0, 0, 0 });

    SceneGraph_destroyObject(graph, bullet);
    return 1;
}

static SceneObjectId instantiate_target(Vector3 position)
{
    SceneObjectId target = SceneGraph_createObject(psg.sceneGraph, "target");
    SceneGraph_setLocalPosition(psg.sceneGraph, target, position);
    AddMeshRendererComponent(target, psg.meshPlane2, 0.0f);
    SceneGraph_addComponent(psg.sceneGraph, target, psg.TargetComponentId,
        &(TargetComponent) {
            .radius = 0.8f,
            .colliderMask = 1,
            .onHit = onTargetHit,
        });
    AddHealthComponent(target, 3, 3);
    
    return target;
}

#define MAX_TARGETS 5
typedef struct TargetSpawnSystem {
    SceneObjectId targets[MAX_TARGETS];
} TargetSpawnSystem;

void HandleTargetSpawnSystem(SceneObject *object, SceneComponentId componentId, float dt, void *targetSpawnSystemPtr)
{
    TargetSpawnSystem *targetSpawnSystem = (TargetSpawnSystem*)targetSpawnSystemPtr;

    for (int i = 0; i < MAX_TARGETS; i++) {
        if (SceneGraph_getObject(psg.sceneGraph, targetSpawnSystem->targets[i]) == NULL) {
            Vector3 position = (Vector3) { GetRandomValue(-5, 5), 0, 20 };
            targetSpawnSystem->targets[i] = instantiate_target(position);
        } else {
            Vector3 pos = SceneGraph_getLocalPosition(psg.sceneGraph, targetSpawnSystem->targets[i]);
            if (pos.z < -5) {
                SceneGraph_destroyObject(psg.sceneGraph, targetSpawnSystem->targets[i]);
            } else {
                pos.z -= psg.deltaTime * 2.5f;
                SceneGraph_setLocalPosition(psg.sceneGraph, targetSpawnSystem->targets[i], pos);
            }
        }
    }
}

void RegisterTargetSpawnSystem()
{
    psg.targetSpawnSystemId = SceneGraph_registerComponentType(psg.sceneGraph, "TargetSpawnSystem", sizeof(TargetSpawnSystem),
        (SceneComponentTypeMethods) {
            .updateTick = HandleTargetSpawnSystem,
        });
}
