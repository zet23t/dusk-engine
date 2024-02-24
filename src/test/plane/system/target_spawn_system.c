#include "../plane_sim_g.h"

#include <math.h>

#define RAND_MAX 0x7fffff
#define MAX_TARGETS 5
static SceneObjectId targets[MAX_TARGETS];
static float GetRandomFloat(float min, float max)
{
    return min + (max - min) * ((float)GetRandomValue(0, RAND_MAX) / (float)RAND_MAX);
}

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
        SceneGraph_addComponent(psg.sceneGraph, effect, psg.meshRendererComponentId,
            &(MeshRendererComponent) {
                .material = psg.model.materials[2],
                .mesh = psg.meshHitParticle1,
            });
        SceneGraph_addComponent(psg.sceneGraph, effect, psg.autoDestroyComponentId,
            &(AutoDestroyComponent) {
                .lifeTimeLeft = 1.0f,
            });
        
        SceneGraph_addComponent(psg.sceneGraph, effect, psg.linearVelocityComponentId,
            &(LinearVelocityComponent) {
                .velocity = (Vector3) {
                    GetRandomFloat(-vSpread, vSpread) - impactVelocity.x * .5f,
                    GetRandomFloat(-vSpread, vSpread) - impactVelocity.y * .5f,
                    GetRandomFloat(-vSpread, vSpread) - impactVelocity.z * .5f,
                },
                .acceleration = (Vector3) { 0, -9.8f, 0 },
                .drag = (Vector3) { drag, drag, drag },
            });

        SceneGraph_addComponent(psg.sceneGraph, effect, psg.updateCallbackComponentId,
            &(UpdateCallbackComponent) {
                .update = onParticleUpdate,
            });
    }
}

static int onTargetHit(SceneGraph* graph, SceneObjectId target, SceneObjectId bullet)
{
    HealthComponent* healthComponent;
    if (SceneGraph_getComponentByType(graph, target, psg.healthComponentId, (void**)&healthComponent, 0)) {
        healthComponent->health -= 1;
    } else
        SceneGraph_destroyObject(graph, target);

    Vector3 position = SceneGraph_getLocalPosition(graph, bullet);
    LinearVelocityComponent* velocity;
    if (SceneGraph_getComponentByType(graph, bullet, psg.linearVelocityComponentId, (void**)&velocity, 0)) {
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
    SceneGraph_addComponent(psg.sceneGraph, target, psg.meshRendererComponentId,
        &(MeshRendererComponent) {
            .material = psg.model.materials[1],
            .mesh = psg.meshTarget,
        });
    SceneGraph_addComponent(psg.sceneGraph, target, psg.targetComponentId,
        &(TargetComponent) {
            .radius = 0.8f,
            .colliderMask = 1,
            .onHit = onTargetHit,
        });
    SceneGraph_addComponent(psg.sceneGraph, target, psg.healthComponentId,
        &(HealthComponent) {
            .health = 3,
            .maxHealth = 3,
        });

    return target;
}

void HandleTargetSpawnSystem()
{
    for (int i = 0; i < MAX_TARGETS; i++) {
        if (SceneGraph_getObject(psg.sceneGraph, targets[i]) == NULL) {
            Vector3 position = (Vector3) { GetRandomValue(-5, 5), 0, 20 };
            targets[i] = instantiate_target(position);
        } else {
            Vector3 pos = SceneGraph_getLocalPosition(psg.sceneGraph, targets[i]);
            if (pos.z < -5) {
                SceneGraph_destroyObject(psg.sceneGraph, targets[i]);
            } else {
                pos.z -= psg.deltaTime * 2.5f;
                SceneGraph_setLocalPosition(psg.sceneGraph, targets[i], pos);
            }
        }
    }
}