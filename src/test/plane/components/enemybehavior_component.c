#include "../plane_sim_g.h"
#include <stdio.h>
#include <string.h>

void EnemyBehaviorComponent_onInitialize(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void* initArg)
{
    EnemyBehaviorComponent* enemyBehaviorComponent = (EnemyBehaviorComponent*)componentData;
    ComponentInitializer* initializer = (ComponentInitializer*)initArg;
    if (initializer->memData != NULL) {
        memcpy(enemyBehaviorComponent, initializer->memData, sizeof(EnemyBehaviorComponent));
    } else {
        memset(enemyBehaviorComponent, 0, sizeof(EnemyBehaviorComponent));
        int pointCount = 0;
        enemyBehaviorComponent->agility = 5.0f;
        MappedVariable mapped[] = {
            { .name = "behaviorType", .type = VALUE_TYPE_INT32, .int32Value = &enemyBehaviorComponent->behaviorType },
            { .name = "agility", .type = VALUE_TYPE_FLOAT, .floatValue = &enemyBehaviorComponent->agility },
            { .name = "p1", .type = VALUE_TYPE_VEC3, .vec3Value = &enemyBehaviorComponent->points[0], .valuePresentCounter = &pointCount },
            { .name = "p2", .type = VALUE_TYPE_VEC3, .vec3Value = &enemyBehaviorComponent->points[1], .valuePresentCounter = &pointCount },
            { .name = "p3", .type = VALUE_TYPE_VEC3, .vec3Value = &enemyBehaviorComponent->points[2], .valuePresentCounter = &pointCount },
            { .name = "p4", .type = VALUE_TYPE_VEC3, .vec3Value = &enemyBehaviorComponent->points[3], .valuePresentCounter = &pointCount },
            { 0 }
        };
        
        ReadMappedVariables(initializer->config, mapped);
        enemyBehaviorComponent->pointCount = (int8_t) pointCount;
    }
}

void EnemyBehaviorComponent_onUpdate(SceneObject* sceneObject, SceneComponentId sceneComponentId, float deltaTime, void* componentData)
{
    EnemyBehaviorComponent* enemyBehaviorComponent = (EnemyBehaviorComponent*)componentData;
    LinearVelocityComponent* velocity;
    SceneComponent* velocityComponent = SceneGraph_getComponentOrFindByType(sceneObject->graph, sceneObject->id, &enemyBehaviorComponent->velocityComponentId, psg.linearVelocityComponentId, (void**)&velocity);
    if (!enemyBehaviorComponent->initialized)
    {
        enemyBehaviorComponent->initialized = 1;
        enemyBehaviorComponent->velocity = Vector3Length(velocity->velocity);
        Matrix worldMatrix = SceneObject_getWorldMatrix(sceneObject);
        for (int i = 0; i < 4; i++) {
            enemyBehaviorComponent->points[i] = Vector3Transform(enemyBehaviorComponent->points[i], worldMatrix);
        }
    }
    enemyBehaviorComponent->time += deltaTime;
    if (velocityComponent && enemyBehaviorComponent->pointCount > enemyBehaviorComponent->phase)
    {
        Vector3 position = SceneGraph_getWorldPosition(sceneObject->graph, sceneObject->id);
        Vector3 direction = Vector3Normalize(Vector3Subtract(enemyBehaviorComponent->points[enemyBehaviorComponent->phase], position));
        
        
        velocity->velocity = Vector3Lerp(velocity->velocity, Vector3Scale(direction, enemyBehaviorComponent->velocity), deltaTime * enemyBehaviorComponent->agility);
        velocity->velocity = Vector3Scale(Vector3Normalize(velocity->velocity), enemyBehaviorComponent->velocity);
        if (Vector3Distance(position, enemyBehaviorComponent->points[enemyBehaviorComponent->phase]) < 1.0f)
        {
            enemyBehaviorComponent->phase++;
            if (enemyBehaviorComponent->phase >= enemyBehaviorComponent->pointCount)
            {
                enemyBehaviorComponent->phase = 0;
            }
        }
    }
}

void EnemyBehaviorComponent_onDraw(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void *userdata)
{
    EnemyBehaviorComponent* enemyBehaviorComponent = (EnemyBehaviorComponent*)componentData;
    Vector3 position = SceneGraph_getWorldPosition(sceneObject->graph, sceneObject->id);
    DrawLine3D(enemyBehaviorComponent->points[0], position, RED);
    DrawLine3D(enemyBehaviorComponent->points[0], enemyBehaviorComponent->points[1], RED);
    DrawLine3D(enemyBehaviorComponent->points[1], enemyBehaviorComponent->points[2], RED);
    DrawLine3D(enemyBehaviorComponent->points[2], enemyBehaviorComponent->points[3], RED);
}

void EnemyBehaviorComponentRegister()
{
    psg.enemyBehaviorComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "EnemyBehaviorComponent", sizeof(EnemyBehaviorComponent),
        (SceneComponentTypeMethods) {
            .onInitialize = EnemyBehaviorComponent_onInitialize,
            .draw = EnemyBehaviorComponent_onDraw,
            .onDestroy = NULL,
            .updateTick = EnemyBehaviorComponent_onUpdate,
        });
}