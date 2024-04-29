#include "../game_g.h"
#include <math.h>
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
    SceneComponent* velocityComponent = SceneGraph_getComponentOrFindByType(sceneObject->graph, sceneObject->id, &enemyBehaviorComponent->velocityComponentId, psg.LinearVelocityComponentId, (void**)&velocity);
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
        float heading = atan2f(velocity->velocity.x, velocity->velocity.z) * RAD2DEG;
        Vector3 localRot = SceneGraph_getLocalRotation(sceneObject->graph, sceneObject->id);
        if (deltaTime > 0)
        {

            float roll = (localRot.y - heading);
            while (roll > 180.0f) roll -= 360.0f;
            while (roll < -180.0f) roll += 360.0f;
            roll = roll / deltaTime / 60.0f;
            SceneGraph_setLocalRotation(sceneObject->graph, sceneObject->id, 
                (Vector3) { 0, heading, (roll * 40.0f) * .25f + localRot.z * .75f});
        }
        if (Vector3Distance(position, enemyBehaviorComponent->points[enemyBehaviorComponent->phase]) < 1.0f)
        {
            enemyBehaviorComponent->phase++;
            if (enemyBehaviorComponent->phase >= enemyBehaviorComponent->pointCount)
            {
                if (enemyBehaviorComponent->autoDestroy)
                {
                    SceneGraph_destroyObject(sceneObject->graph, sceneObject->id);
                }
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
    for (int i = 1; i < enemyBehaviorComponent->pointCount; i++)
    {
        DrawLine3D(enemyBehaviorComponent->points[i-1], enemyBehaviorComponent->points[i], RED);
    }
}