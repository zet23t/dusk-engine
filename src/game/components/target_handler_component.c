#include "../game_g.h"
#include "../util/util_math.h"

typedef struct SpawnAction {
    const char* prefab;
    int spawnCount;
    Vector2 bulletVelocityFactor;
    Vector3 minRandomVelocityRange;
    Vector3 maxRandomVelocityRange;
} SpawnAction;

static SpawnAction SpawnActionFromJson(cJSON* json)
{
    SpawnAction action = { .spawnCount = 1, .bulletVelocityFactor = (Vector2) { 0, 0 } };
    MappedVariable mapped[] = {
        { .name = "prefab", .type = VALUE_TYPE_STRING, .stringValue = &action.prefab, .isRequired = 1 },
        { .name = "spawnCount", .type = VALUE_TYPE_INT, .intValue = &action.spawnCount, .isRequired = 0 },
        { .name = "bulletVelocityFactor", .type = VALUE_TYPE_VEC2, .vec2Value = &action.bulletVelocityFactor, .isRequired = 0 },
        { .name = "minRandomVelocityRange", .type = VALUE_TYPE_VEC3, .vec3Value = &action.minRandomVelocityRange, .isRequired = 0 },
        { .name = "maxRandomVelocityRange", .type = VALUE_TYPE_VEC3, .vec3Value = &action.maxRandomVelocityRange, .isRequired = 0 },
        { 0 }
    };
    ReadMappedVariables(json, mapped);
    return action;
}

typedef struct TargetHandlerComponent {
    cJSON* objects;
    const char* onHitSpawn;

    SpawnAction* onHitActions;
    int onHitActionCount;

    SpawnAction* onDestroyActions;
    int onDestroyActionCount;
} TargetHandlerComponent;

static void HandleSpawnActions(SceneGraph *graph, cJSON *objects, Vector3 position, Vector3 velocity, int actionCount, SpawnAction* actions)
{
    for (int i = 0; i < actionCount; i++) {
        SpawnAction action = actions[i];
        for (int j = 0; j < action.spawnCount; j++) {
            SceneObjectId spawn = InstantiateFromJSON(graph, objects, action.prefab);
            SceneGraph_setLocalPosition(graph, spawn, position);
            LinearVelocityComponent* spawnVelocity;
            if (SceneGraph_getComponentByType(graph, spawn, psg.LinearVelocityComponentId, (void**)&spawnVelocity, 0)) {
                float factor = GetRandomFloat(action.bulletVelocityFactor.x, action.bulletVelocityFactor.y);
                float x = GetRandomFloat(action.minRandomVelocityRange.x, action.maxRandomVelocityRange.x);
                float y = GetRandomFloat(action.minRandomVelocityRange.y, action.maxRandomVelocityRange.y);
                float z = GetRandomFloat(action.minRandomVelocityRange.z, action.maxRandomVelocityRange.z);
                spawnVelocity->velocity.x = velocity.x * factor + x;
                spawnVelocity->velocity.y = velocity.y * factor + y;
                spawnVelocity->velocity.z = velocity.z * factor + z;
            }
        }
    }
}

static int TargetHandlerComponentOnHit(SceneGraph* graph, SceneObjectId target, SceneObjectId bullet)
{
    HealthComponent* healthComponent;
    int isDestroyed = 0;
    if (SceneGraph_getComponentByType(graph, target, psg.HealthComponentId, (void**)&healthComponent, 0)) {
        healthComponent->health -= 1;
        isDestroyed = healthComponent->health <= 0;
    } else {
        isDestroyed = 1;
        SceneGraph_destroyObject(graph, target);
    }

    Vector3 position = SceneGraph_getWorldPosition(graph, bullet);
    Vector3 velocity = { 0, 0, 0 };
    Vector3 targetVelocity = { 0, 0, 0 };
    LinearVelocityComponent* velocityComponent;
    if (SceneGraph_getComponentByType(graph, bullet, psg.LinearVelocityComponentId, (void**)&velocityComponent, 0)) {
        velocity = velocityComponent->velocity;
    }
    if (SceneGraph_getComponentByType(graph, target, psg.LinearVelocityComponentId, (void**)&velocityComponent, 0)) {
        targetVelocity = velocityComponent->velocity;
    }

    TargetHandlerComponent* targetHandler;
    if (SceneGraph_getComponentByType(graph, target, psg.targetHandlerComponentId, (void**)&targetHandler, 0)) {
        HandleSpawnActions(graph, targetHandler->objects, position, velocity, targetHandler->onHitActionCount, targetHandler->onHitActions);
        if (isDestroyed) {
            HandleSpawnActions(graph, targetHandler->objects, SceneGraph_getWorldPosition(graph, target), targetVelocity, targetHandler->onDestroyActionCount, targetHandler->onDestroyActions);
        }
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
            { .name = "colliderMask", .type = VALUE_TYPE_INT32, .uint32Value = &targetComponent.colliderMask, .isRequired = 1 },
            { .name = "radius", .type = VALUE_TYPE_FLOAT, .floatValue = &targetComponent.radius, .isRequired = 1 },
            { 0 }
        };
        ReadMappedVariables(config, mapped);

        targetComponent.onHit = TargetHandlerComponentOnHit;

        cJSON* onHit = cJSON_GetObjectItem(config, "onHit");
        if (cJSON_IsArray(onHit)) {
            targetHandler->onHitActionCount = cJSON_GetArraySize(onHit);
            targetHandler->onHitActions = (SpawnAction*)malloc(sizeof(SpawnAction) * targetHandler->onHitActionCount);
            for (int i = 0; i < targetHandler->onHitActionCount; i++) {
                targetHandler->onHitActions[i] = SpawnActionFromJson(cJSON_GetArrayItem(onHit, i));
            }
        }

        cJSON* onDestroy = cJSON_GetObjectItem(config, "onDestroy");
        if (cJSON_IsArray(onDestroy)) {
            targetHandler->onDestroyActionCount = cJSON_GetArraySize(onDestroy);
            targetHandler->onDestroyActions = (SpawnAction*)malloc(sizeof(SpawnAction) * targetHandler->onDestroyActionCount);
            for (int i = 0; i < targetHandler->onDestroyActionCount; i++) {
                targetHandler->onDestroyActions[i] = SpawnActionFromJson(cJSON_GetArrayItem(onDestroy, i));
            }
        }

        SceneGraph_addComponent(psg.sceneGraph, sceneObject->id, psg.TargetComponentId, &targetComponent);
    }
}

static void TargetHandlerComponentOnDestroy(SceneObject* obj, SceneComponentId sceneComponentId, void* componentData)
{
    TargetHandlerComponent* targetHandler = (TargetHandlerComponent*)componentData;
    if (targetHandler->onHitActions) {
        free(targetHandler->onHitActions);
        targetHandler->onHitActions = NULL;
    }
    if (targetHandler->onDestroyActions) {
        free(targetHandler->onDestroyActions);
        targetHandler->onDestroyActions = NULL;
    }
}

void TargetHandlerComponentRegister()
{
    psg.targetHandlerComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "TargetHandlerComponent", sizeof(TargetHandlerComponent),
        (SceneComponentTypeMethods) {
            .onInitialize = TargetHandlerComponentInitialize,
            .onDestroy = TargetHandlerComponentOnDestroy,
        });
}