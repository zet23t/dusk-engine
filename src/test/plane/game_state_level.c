#include "config.h"
#include "math.h"
#include "game_state_level.h"
#include "plane_sim_g.h"
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void onShoot(SceneGraph* graph, SceneComponentId shooter, ShootingComponent* shootingComponent, ShootingConfig* shootingConfig)
{
    SceneComponent* shooting;
    shooting = SceneGraph_getComponent(graph, shooter, NULL);
    if (shooting == NULL) {
        return;
    }
    SceneObjectId bullet = SceneGraph_createObject(graph, "bullet");
    SceneGraph_setLocalPosition(psg.sceneGraph, bullet, SceneGraph_getWorldPosition(psg.sceneGraph, shootingComponent->spawnPoint));
    AddMeshRendererComponent(bullet, psg.meshPlayerBullet, 1.0f);

    Vector3 forward = SceneGraph_getWorldForward(psg.sceneGraph, shootingComponent->spawnPoint);
    forward = Vector3Scale(forward, shootingConfig->bulletSpeed);
    forward.y = 0;
    AddLinearVelocityComponent(bullet, forward, Vector3Zero(), Vector3Zero());

    AddAutoDestroyComponent(bullet, shootingConfig->bulletLifetime);
    SceneGraph_addComponent(graph, bullet, psg.bulletComponentId,
        &(BulletComponent) {
            .radius = 0.01f,
            .colliderFlags = 1,
            .damage = 1 });
}
static Material _trailMaterial = { 0 };

static void AddWingTrail(SceneObjectId parent, float x, float y, float speed, float lifetime)
{
    if (_trailMaterial.shader.id == 0) {
        Texture2D texture = LoadTexture("assets/wing-trail.png");
        SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
        _trailMaterial = LoadMaterialDefault();
        SetMaterialTexture(&_trailMaterial, MATERIAL_MAP_DIFFUSE, texture);
    }
    SceneObjectId wingTrail = SceneGraph_createObject(psg.sceneGraph, "wing-trail-left");
    SceneGraph_setParent(psg.sceneGraph, wingTrail, parent);
    SceneGraph_setLocalPosition(psg.sceneGraph, wingTrail, (Vector3) { x, 0, y });
    
    SceneComponentId trailId = AddTrailRendererComponent(wingTrail, 10 / lifetime, lifetime, (Vector3) { 0, 0, -speed }, 20, _trailMaterial, 0);
    TrailRendererComponent* trail = NULL;
    SceneComponent* component = SceneGraph_getComponent(psg.sceneGraph, trailId, (void**)&trail);
    if (component)
    {
        TrailRendererComponent_addTrailWidth(trail, 0.0f, 0.0f);
        TrailRendererComponent_addTrailWidth(trail, 0.05f, 0.23f);
        TrailRendererComponent_addTrailWidth(trail, 0.02f, 0.5f);
        TrailRendererComponent_addTrailWidth(trail, 0.001f, 0.8f);
    }
}

SceneObjectId add_propeller(SceneObjectId plane, float x, float y, float z)
{
    SceneObjectId propeller = SceneGraph_createObject(psg.sceneGraph, "propeller");
    SceneGraph_setParent(psg.sceneGraph, propeller, plane);
    SceneGraph_setLocalPosition(psg.sceneGraph, propeller, (Vector3) { x,y,z });
    AddMeshRendererComponent(propeller, psg.meshPropellerPin, 0.0f);

    for (int i = 0; i < 3; i += 1) {
        SceneObjectId propellerBlade = SceneGraph_createObject(psg.sceneGraph, "propeller-blade");
        SceneGraph_setParent(psg.sceneGraph, propellerBlade, propeller);
        SceneGraph_setLocalPosition(psg.sceneGraph, propellerBlade, (Vector3) { 0, 0, 0 });
        SceneGraph_setLocalRotation(psg.sceneGraph, propellerBlade, (Vector3) { 0, 0, 120 * i });
        AddMeshRendererComponent(propellerBlade, psg.meshPropellerBlade, 0.0f);
    }
    return propeller; 
}

SceneObjectId plane_instantiate(Vector3 position)
{
    SceneObjectId plane = SceneGraph_createObject(psg.sceneGraph, "plane");
    SceneGraph_setLocalPosition(psg.sceneGraph, plane, position);
    AddMeshRendererComponent(plane, psg.meshPlane, 0.0f);

    AddWingTrail(plane, 0.95f, 0.55f, 30, .25f);
    AddWingTrail(plane, -0.95f, 0.55f, 30, .25f);

    SceneObjectId propeller = add_propeller(plane, 0, 0.062696f, 0.795618f);

    SceneGraph_addComponent(psg.sceneGraph, plane, psg.planeBehaviorComponentId,
        &(PlaneBehaviorComponent) {
            .phase = GetRandomValue(0, 1000),
            .rolling = 0,
            .time = 0,
            .prevVelocity = (Vector3) { 0, 0, 0 },
            .propeller = propeller,
        });

    const float drag = 5.0f;
    AddLinearVelocityComponent(plane, Vector3Zero(), Vector3Zero(), (Vector3) { drag, drag, drag });

    ShootingConfig shootingConfig = {
        .shotInterval = 0.15f,
        .bulletSpeed = 20.0f,
        .bulletLifetime = .75f,
        .onShoot = (OnShootFn)onShoot,
    };

    SceneObjectId spawnPoint1 = SceneGraph_createObject(psg.sceneGraph, "spawn-point-1");
    SceneGraph_setParent(psg.sceneGraph, spawnPoint1, plane);
    SceneGraph_setLocalPosition(psg.sceneGraph, spawnPoint1, (Vector3) { 0.566518f, -0.018f, 0.617959f });

    SceneGraph_addComponent(psg.sceneGraph, plane, psg.shootingComponentId,
        &(ShootingComponent) {
            .spawnPoint = spawnPoint1,
            .config = shootingConfig,
        });

    SceneObjectId spawnPoint2 = SceneGraph_createObject(psg.sceneGraph, "spawn-point-2");
    SceneGraph_setParent(psg.sceneGraph, spawnPoint2, plane);
    SceneGraph_setLocalPosition(psg.sceneGraph, spawnPoint2, (Vector3) { -0.566518f, -0.018f, 0.617959f });

    SceneGraph_addComponent(psg.sceneGraph, plane, psg.shootingComponentId,
        &(ShootingComponent) {
            .spawnPoint = spawnPoint2,
            .config = shootingConfig,
        });

    return plane;
}

static void PropellerRotator(SceneGraph* graph, SceneObjectId objectId, SceneComponentId componentId, float dt, struct UpdateCallbackComponent *component)
{
    float speed = component->floatValue;
    Vector3 rotation = SceneGraph_getLocalRotation(graph, objectId);
    SceneGraph_setLocalRotation(graph, objectId, (Vector3) { 0, 0, rotation.z + speed * dt });
}

static void SpawnEnemy(SceneGraph *g, float x, float y)
{
    SceneObjectId enemy = SceneGraph_createObject(g, "enemy");
    SceneGraph_setLocalPosition(g, enemy, (Vector3) { x, 0, y });
    SceneGraph_setLocalRotation(g, enemy, (Vector3) { 0, 180, 0 });
    SceneGraph_addComponent(g, enemy, psg.enemyBehaviorComponentId,
        &(EnemyBehaviorComponent) {
            .agility = 2.0f,
            .initialized = 1,
            .behaviorType = 0,
            .points[0] = (Vector3) { x + 1, 0, y - 5 },
            .points[1] = (Vector3) { x + 2, 0, y + 5},
            .points[2] = (Vector3) { x - 2, 0, y + 5},
            .pointCount = 3,
            .velocity = 3.50f,
            .velocityComponentId = 
                AddLinearVelocityComponent(enemy, Vector3Zero(), Vector3Zero(), Vector3Zero())
        });
    SceneObjectId propeller = add_propeller(enemy, 0, 0.08f, 0.6f);
    SceneGraph_addComponent(g, propeller, psg.updateCallbackComponentId,
        &(UpdateCallbackComponent) {
            .update = PropellerRotator,
            .floatValue = 3000.0f
        });
    AddMeshRendererComponent(enemy, psg.meshPlane2, 1.0f);
    AddWingTrail(enemy, 1.0f, 0.3f, 0, 2.0f);
    AddWingTrail(enemy, -1.0f, 0.3f, 0, 2.0f);
}

static void level1_e1(SceneGraph *g, void *)
{
    SpawnEnemy(g, 0, 5);
}

int GameStateLevel_Init()
{
    SceneGraph_clear(psg.sceneGraph);
    SceneObjectId systemsId = SceneGraph_createObject(psg.sceneGraph, "systems");
    // SceneGraph_addComponent(psg.sceneGraph, systemsId, psg.targetSpawnSystemId, NULL);
    SceneGraph_addComponent(psg.sceneGraph, systemsId, psg.cloudSystemId, NULL);
    static LevelEvent events[] = {
        {.time = 1.0f, .onTrigger = level1_e1 },
        {0}
    };
    SceneGraph_addComponent(psg.sceneGraph, systemsId, psg.levelSystemId, &(LevelSystem){
        .time = 0.0f,
        .events = events
    });
    SceneGraph_addComponent(psg.sceneGraph, systemsId, psg.playerInputHandlerId, NULL);
    SceneGraph_addComponent(psg.sceneGraph, systemsId, psg.groundTileSystemId, NULL);

    psg.camera = SceneGraph_createObject(psg.sceneGraph, "camera");
    SceneGraph_setLocalPosition(psg.sceneGraph, psg.camera, (Vector3) { 0, 100, -25 });
    SceneGraph_setLocalRotation(psg.sceneGraph, psg.camera, (Vector3) { 74.5, 0, 0 });
    SceneGraph_addComponent(psg.sceneGraph, psg.camera, psg.cameraComponentId,
        &(CameraComponent) {
            .fov = 10,
            .nearPlane = 64.0f,
            .farPlane = 256.0f,
        });

    psg.playerPlane = plane_instantiate((Vector3) { 0, 0, 0 });

    SceneObjectId uiPlaneId = SceneGraph_createObject(psg.sceneGraph, "ui-plane");
    psg.uiRootId = uiPlaneId;
    SceneGraph_setParent(psg.sceneGraph, uiPlaneId, psg.camera);
    SceneGraph_setLocalPosition(psg.sceneGraph, uiPlaneId, (Vector3) { 0, 0, 70.0f });
    // SceneGraph_addComponent(psg.sceneGraph, uiPlaneId, psg.textComponentId,
    //     &(TextComponent) {
    //         .text = "Hello, world!",
    //         .fontSize = 40,
    //         .color = WHITE,
    //     });
    // SceneGraph_setLocalRotation(psg.sceneGraph, uiPlaneId, (Vector3) { 0, 0, 0 });

    cJSON* uiConfig = cJSON_GetObjectItemCaseSensitive(psg.levelConfig, "ui");
    if (!cJSON_IsObject(uiConfig)) {
        TraceLog(LOG_ERROR, "No 'ui' object in level config");
        return 0;
    }
    cJSON* uiRootName = cJSON_GetObjectItemCaseSensitive(uiConfig, "root");
    if (!cJSON_IsString(uiRootName)) {
        TraceLog(LOG_ERROR, "No 'root' string in 'ui' object in level config");
        return 0;
    }

    cJSON* objects = cJSON_GetObjectItemCaseSensitive(uiConfig, "objects");

    SceneObjectId groupLeftId = SceneGraph_createObject(psg.sceneGraph, "group-left");
    SceneGraph_setLocalPosition(psg.sceneGraph, groupLeftId, (Vector3) { 8, 0, 0 });
    SceneGraph_setParent(psg.sceneGraph, groupLeftId, uiPlaneId);
    SceneObjectId borderLeftId = SceneGraph_createObject(psg.sceneGraph, "border-left");
    SceneGraph_setParent(psg.sceneGraph, borderLeftId, groupLeftId);
    SceneGraph_setLocalPosition(psg.sceneGraph, borderLeftId, (Vector3) { 0, 5, 0 });

    SceneObjectId uiCanvas = InstantiateFromJSON(psg.sceneGraph, objects, uiRootName->valuestring);
    SceneGraph_setParent(psg.sceneGraph, uiCanvas, uiPlaneId);

    return 0;
}