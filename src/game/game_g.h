#ifndef __game_G_H__
#define __game_G_H__

#include "external/cjson.h"
#include "shared/scene_graph/scene_graph.h"
#include "shared/touch_util.h"
#include "resource_manager.h"
#include "messagehub.h"
#include <inttypes.h>
#include <memory.h>
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
    int *valuePresentCounter;
} MappedVariable;

void ReadMappedVariables(cJSON* map, MappedVariable* variables);

typedef struct MeshTileConfig {
    Mesh* mesh;
    // corner identifiers for all 4 rotations
    uint32_t cornerConfigs[4];
} MeshTileConfig;

typedef struct PSG {
    float time;
    float deltaTime;
    int hasMouse;

    ResourceManager resourceManager;

    SceneGraph* sceneGraph;

    cJSON* levelConfig;

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
    SceneComponentTypeId textComponentId;
    SceneComponentTypeId timerComponentTypeId;
    SceneComponentTypeId trailRendererComponentTypeId;
    SceneComponentTypeId tweenComponentId;
    SceneComponentTypeId enemyBehaviorComponentId;

    SceneComponentTypeId targetSpawnSystemId;
    SceneComponentTypeId levelSystemId;
    SceneComponentTypeId cloudSystemId;
    SceneComponentTypeId playerInputHandlerId;
    SceneComponentTypeId groundTileSystemId;
    SceneComponentTypeId gameUiSystemId;

#define COMPONENT(t) SceneComponentTypeId t##Id;
#include "component_list.h"
#undef COMPONENT

    SceneObjectId camera;
    SceneObjectId uiRootId;

    Model model;
    Mesh* meshPlane;
    Mesh* meshPlane2;
    Mesh* meshPropellerPin;
    Mesh* meshPropellerBlade;
    Mesh* meshPlayerBullet;
    Mesh* meshTarget;
    Mesh* meshHitParticle1;

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

typedef struct SpriteAsset
{
    Texture2D texture;
    Rectangle source;
    Vector4 scale9frame;
} SpriteAsset;

typedef struct LevelEventData {
    float x;
    float y;
    float formationStep;
    uint8_t n;
    uint8_t pointCount;
    Vector2 points[4];
} LevelEventData;

typedef struct LevelEvent {
    float time;
    int triggered;
    void (*onTrigger)(SceneGraph*, struct LevelEvent* userData);

    union {
        void *userData;
        Vector3 userDataVec3;
        float userDataFloat;
        LevelEventData levelEventData;
    };
} LevelEvent;

typedef struct LevelSystem {
    float time;
    LevelEvent* events;
} LevelSystem;

typedef struct ShootingConfig ShootingConfig;
typedef struct ShootingComponent ShootingComponent;

typedef struct EnemyBehaviorComponent
{
    int32_t behaviorType;
    int32_t phase;
    float time, velocity, agility;
    Vector3 points[4];
    int8_t pointCount;
    int8_t initialized;
    int8_t autoDestroy;
    SceneComponentId velocityComponentId;
} EnemyBehaviorComponent;

typedef struct TextComponent {
    Font font;
    char* text;
    Vector2 align;
    Vector2 offset;
    float fontSize;
    float fontSpacing;
    float lineSpacing;
    Color color;
} TextComponent;

void TextComponent_setText(SceneGraph* graph, SceneComponentId textComponentId, const char *str);

typedef struct TrailNode {
    Vector3 position;
    Vector3 velocity;
    float time;
    float widthPercent;
} TrailNode;

typedef struct TrailWidthStep {
    float width;
    float percent;
} TrailWidthStep;

typedef struct TrailRendererComponent {
    Mesh mesh;
    Material material;
    TrailNode* nodes;
    TrailWidthStep* trailWidths;
    uint16_t nodeCount;
    uint16_t nodeCapacity;
    uint8_t trailWidthCount;
    // if 0 = stretch texture over entire.
    float uvDistanceFactor;
    unsigned int meshIsDirty : 1;
    unsigned int ownsMaterial : 1;

    float currentWidthPercentage;
    float widthDecayRate;
    float emitterRate;
    float maxLifeTime;
    float time;
    float lastEmitTime;
    Vector3 emitterVelocity;
    Vector3 lastPosition;
} TrailRendererComponent;

typedef struct CameraComponent {
    float fov;
    float nearPlane;
    float farPlane;
} CameraComponent;

typedef struct GroundTileModification {
    int x, y, width, height;
    char type;
    float treeChance;
} GroundTileModification;
typedef struct GroundTileSpawner {
    int x, y, width, height;
    union {
        void *arg;
        Vector3 vec3[4];
    };
    void (*spawn)(SceneGraph*, SceneObjectId parent, int x, int y, struct GroundTileSpawner* arg);
} GroundTileSpawner;
#define COLUMN_COUNT 6
#define ROW_COUNT 8
typedef struct GroundTileConfigComponent {
    float offset;
    SceneObjectId groundTiles[ROW_COUNT * COLUMN_COUNT];
    GroundTileModification modifications[16];
    GroundTileSpawner spawners[16];
    float waterLineLevel;
    float highgroundLevel;
    float baseSpeed;
} GroundTileConfigComponent;

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
    union
    {
        void* data;
        SceneObjectId objectId;
        SceneComponentId componentId;
        float floatValue;
        Vector3 vec3Value;
    };
    
} UpdateCallbackComponent;

typedef void (*OnShootFn)(SceneGraph*, SceneComponentId shooter, ShootingComponent*, ShootingConfig* shootingComponent);

typedef struct ShootingConfig {
    float shotInterval;
    float bulletSpeed;
    float bulletLifetime;
    int bulletMask;
    OnShootFn onShoot;
} ShootingConfig;

typedef struct AutoDestroyComponent {
    float lifeTimeLeft;
} AutoDestroyComponent;

typedef struct ShootingComponent {
    float cooldown;
    int8_t shooting;
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

#define COMPONENT_DECLARATION
#include "component_list.h"
#undef COMPONENT_DECLARATION

extern PSG psg;

Camera3D CameraComponent_getCamera3D(SceneGraph* sceneGraph, SceneObjectId nodeId);
void SceneObject_ApplyJSONValues(SceneGraph* sceneGraph, cJSON* objects, SceneObjectId nodeId, cJSON* object);
SceneObjectId InstantiateFromJSON(SceneGraph* sceneGraph, cJSON* objects, const char* rootId);

SceneComponentId AddMeshRendererComponent(SceneObjectId id, Mesh* mesh, float litAmount);
SceneComponentId AddMeshRendererComponentByName(SceneObjectId id, const char *name, float litAmount);
SceneComponentId AddLinearVelocityComponent(SceneObjectId objectId, Vector3 velocity, Vector3 acceleration, Vector3 drag);
void AddAutoDestroyComponent(SceneObjectId objectId, float lifeTime);
void AddHealthComponent(SceneObjectId objectId, int health, int maxHealth);
SceneComponentId AddTimerComponent(SceneObjectId objectId, float duration, int repeat);
SceneComponentId AddTrailRendererComponent(SceneObjectId objectId, float emitterRate, float maxLifeTime, Vector3 emitterVelocity, int maxVertexCount, Material material, int ownsMaterial);
void TrailRendererComponent_addTrailWidth(TrailRendererComponent* trailRendererComponent, float width, float percent);
void TrailRendererComponent_setMaterial(TrailRendererComponent* trailRendererComponent, Material material, int ownsMaterial);

Vector3 GetVelocity(SceneObjectId objectId);
#endif