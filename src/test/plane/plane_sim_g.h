#ifndef __PLANE_SIM_G_H__
#define __PLANE_SIM_G_H__

#include "external/cjson.h"
#include "shared/scene_graph/scene_graph.h"
#include <memory.h>
#include <inttypes.h>
#include <raylib.h>
#include <raymath.h>



#define VALUE_TYPE_FLOAT 0
#define VALUE_TYPE_INT 1
#define VALUE_TYPE_INT32 2
#define VALUE_TYPE_STRING 3
#define VALUE_TYPE_BOOL 4
#define VALUE_TYPE_VEC2 5
#define VALUE_TYPE_VEC3 6

typedef struct ComponentInitializer {
    cJSON* config;
    cJSON* objects;
    void* memData;
} ComponentInitializer;

typedef struct MappedVariable {
    const char* name;
    int type;
    int isRequired;
    union {
        float* floatValue;
        int* intValue;
        int32_t* int32Value;
        uint32_t* uint32Value;
        const char** stringValue;
        bool* boolValue;
        Vector2* vec2Value;
        Vector3* vec3Value;
    };

} MappedVariable;

void ReadMappedVariables(cJSON *map, MappedVariable *variables);

typedef struct MeshTileConfig {
    Mesh* mesh;
    // corner identifiers for all 4 rotations
    uint32_t cornerConfigs[4];
} MeshTileConfig;

typedef struct PSG {
    float time;
    float deltaTime;

    SceneGraph* sceneGraph;

    cJSON *levelConfig;

    SceneComponentTypeId meshRendererComponentId;
    SceneComponentTypeId planeBehaviorComponentId;
    SceneComponentTypeId linearVelocityComponentId;
    SceneComponentTypeId shootingComponentId;
    SceneComponentTypeId autoDestroyComponentId;
    SceneComponentTypeId bulletComponentId;
    SceneComponentTypeId targetComponentId;
    SceneComponentTypeId healthComponentId;
    SceneComponentTypeId updateCallbackComponentId;
    SceneComponentTypeId enemyPlaneBehaviorComponentId;
    SceneComponentTypeId movementPatternComponentId;
    SceneComponentTypeId cameraComponentId;
    SceneComponentTypeId targetHandlerComponentId;
    
    SceneComponentTypeId targetSpawnSystemId;
    SceneComponentTypeId levelSystemId;
    SceneComponentTypeId cloudSystemId;

    SceneObjectId camera;

    Model model;
    Mesh* meshPlane;
    Mesh* meshPlane2;
    Mesh* meshPropellerPin;
    Mesh* meshPropellerBlade;
    Mesh* meshPlayerBullet;
    Mesh* meshTarget;
    Mesh* meshHitParticle1;
    Mesh* meshUiBorder;
    
    Mesh** leafTreeList;
    int leafTreeCount;

    MeshTileConfig* meshTiles;
    int meshTileCount;

    Mesh** cloudList;
    int cloudCount;

    int disableDrawMesh;
    SceneObjectId playerPlane;

    ShaderLocationIndex litAmountIndex;
} PSG;

typedef struct ShootingConfig ShootingConfig;
typedef struct ShootingComponent ShootingComponent;

typedef struct CameraComponent
{
    float fov;
    float nearPlane;
    float farPlane;
} CameraComponent;

typedef struct HealthComponent {
    float health;
    float maxHealth;
} HealthComponent;

typedef struct EnemyPlaneBehaviorComponent {
    float phase;
    float rolling;
    float smoothedXAccel;
    float time;
    Vector3 prevVelocity;
    SceneObjectId propeller;
    SceneComponentId velocityComponentId;
} EnemyPlaneBehaviorComponent;

typedef struct MovementPatternComponent {
    float time;
    float speed;
    SceneComponentId velocityComponentId;
} MovementPatternComponent;

typedef struct UpdateCallbackComponent {
    void (*update)(SceneGraph*, SceneObjectId, SceneComponentId, float dt, struct UpdateCallbackComponent*);
    void* data;
} UpdateCallbackComponent;

typedef void (*OnShootFn)(SceneGraph*, SceneComponentId shooter, ShootingComponent*, ShootingConfig* shootingComponent);

typedef struct ShootingConfig {
    float shotInterval;
    float bulletSpeed;
    float bulletLifetime;
    OnShootFn onShoot;
} ShootingConfig;

typedef struct AutoDestroyComponent
{
    float lifeTimeLeft;
} AutoDestroyComponent;

typedef struct ShootingComponent {
    float cooldown;
    int32_t shooting;
    ShootingConfig config;
    SceneObjectId spawnPoint;
} ShootingComponent;

typedef struct BulletComponent {
    float radius;
    float damage;
    uint32_t colliderFlags;
} BulletComponent;

typedef int (*OnHit)(SceneGraph*, SceneObjectId target, SceneObjectId bullet);
typedef struct TargetComponent {
    float radius;
    uint32_t colliderMask;
    OnHit onHit;
} TargetComponent;

typedef struct MeshRendererComponent {
    float litAmount;
    Material *material;
    Mesh* mesh;
} MeshRendererComponent;

typedef struct PlaneBehaviorComponent {
    float phase;
    float rolling;
    float smoothedXAccel;
    float time;
    Vector3 prevVelocity;
    SceneObjectId propeller;
    SceneComponentId velocityComponentId;
} PlaneBehaviorComponent;

typedef struct LinearVelocityComponent {
    Vector3 velocity;
    Vector3 acceleration;
    Vector3 drag;
} LinearVelocityComponent;

extern PSG psg;

Camera3D CameraComponent_getCamera3D(SceneGraph* sceneGraph, SceneObjectId nodeId);
void SceneObject_ApplyJSONValues(SceneGraph* sceneGraph, cJSON* objects, SceneObjectId nodeId, cJSON* object);
SceneObjectId InstantiateFromJSON(SceneGraph* sceneGraph, cJSON* objects, const char* rootId);

SceneComponentId AddMeshRendererComponent(SceneObjectId id, Mesh* mesh, float litAmount);
void AddLinearVelocityComponent(SceneObjectId objectId, Vector3 velocity, Vector3 acceleration, Vector3 drag);
void AddAutoDestroyComponent(SceneObjectId objectId, float lifeTime);
void AddHealthComponent(SceneObjectId objectId, int health, int maxHealth);

#endif