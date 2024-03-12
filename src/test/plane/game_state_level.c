#include "game_state_level.h"
#include "config.h"
#include "math.h"
#include "plane_sim_g.h"
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/util_math.h"

static Material onShootTrailMaterial = { 0 };

static void onShoot(SceneGraph* graph, SceneComponentId shooter, ShootingComponent* shootingComponent, ShootingConfig* shootingConfig)
{
    SceneComponent* shooting;
    shooting = SceneGraph_getComponent(graph, shooter, NULL);
    if (shooting == NULL) {
        return;
    }
    if (onShootTrailMaterial.shader.id == 0) {
        // Texture2D texture = LoadTexture("assets/wing-trail.png");
        // SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
        onShootTrailMaterial = LoadMaterialDefault();
        // SetMaterialTexture(&onShootTrailMaterial, MATERIAL_MAP_DIFFUSE, texture);
    }
    Vector3 forward = SceneGraph_getWorldForward(psg.sceneGraph, shootingComponent->spawnPoint);
    SceneObjectId bullet = SceneGraph_createObject(graph, "bullet");
    float angle = atan2f(forward.x, forward.z) * RAD2DEG;
    SceneGraph_setLocalPosition(psg.sceneGraph, bullet, SceneGraph_getWorldPosition(psg.sceneGraph, shootingComponent->spawnPoint));
    SceneGraph_setLocalRotation(psg.sceneGraph, bullet, (Vector3) { 0, angle, 0 });
    // SceneComponentId trail = AddTrailRendererComponent(bullet, 40, 0.125f, Vector3Zero(), 20, onShootTrailMaterial, 0);
    // TrailRendererComponent* trailComponent;
    // if (SceneGraph_getComponent(graph, trail, (void**)&trailComponent))
    // {
    //     trailComponent->widthDecayRate = 0.5f;
    //     TrailRendererComponent_addTrailWidth(trailComponent, 0.0f, 0.0f);
    //     TrailRendererComponent_addTrailWidth(trailComponent, 0.07f, 0.23f);
    //     TrailRendererComponent_addTrailWidth(trailComponent, 0.03f, 0.5f);
    //     TrailRendererComponent_addTrailWidth(trailComponent, 0.001f, 0.8f);
    // }
    AddMeshRendererComponent(bullet, psg.meshPlayerBullet, 1.0f);

    forward = Vector3Scale(forward, shootingConfig->bulletSpeed);
    forward.y = 0;
    AddLinearVelocityComponent(bullet, forward, Vector3Zero(), Vector3Zero());

    AddAutoDestroyComponent(bullet, shootingConfig->bulletLifetime);
    SceneGraph_addComponent(graph, bullet, psg.bulletComponentId,
        &(BulletComponent) {
            .radius = 0.01f,
            .colliderFlags = shootingConfig->bulletMask,
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
    if (component) {
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
    SceneGraph_setLocalPosition(psg.sceneGraph, propeller, (Vector3) { x, y, z });
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
        .bulletMask = 1,
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

static void PropellerRotator(SceneGraph* graph, SceneObjectId objectId, SceneComponentId componentId, float dt, struct UpdateCallbackComponent* component)
{
    float speed = component->floatValue;
    Vector3 rotation = SceneGraph_getLocalRotation(graph, objectId);
    SceneGraph_setLocalRotation(graph, objectId, (Vector3) { 0, 0, rotation.z + speed * dt });
}

static Material _hitEffectMaterial = { 0 };
void SpawnHitEffect(SceneGraph *g, Vector3 position, Vector3 initialVelocity)
{
    if (_hitEffectMaterial.shader.id == 0) {
        Texture2D texture = LoadTexture("assets/spark.png");
        SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
        _hitEffectMaterial = LoadMaterialDefault();
        SetMaterialTexture(&_hitEffectMaterial, MATERIAL_MAP_DIFFUSE, texture);
        _hitEffectMaterial.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    }
    float maxIniV = fabsf(initialVelocity.x);
    if (fabsf(initialVelocity.y) > maxIniV) {
        maxIniV = fabsf(initialVelocity.y);
    }
    if (fabsf(initialVelocity.z) > maxIniV) {
        maxIniV = fabsf(initialVelocity.z);
    }
    for (int i=0;i<5;i+=1)
    {
        Vector3 rndVel = (Vector3) { GetRandomFloat(-1, 1), GetRandomFloat(-1, 1), GetRandomFloat(-1, 1) };
        SceneObjectId hitEffect = SceneGraph_createObject(g, "hit-effect");
        SceneGraph_setLocalPosition(g, hitEffect, position);
        SceneComponentId trailId = AddTrailRendererComponent(hitEffect, 20, 0.5f, Vector3Zero(), 20, _hitEffectMaterial, 0);
        
        TrailRendererComponent* trail = NULL;
        SceneGraph_getComponent(g, trailId, (void**)&trail);
        TrailRendererComponent_addTrailWidth(trail, 0.0f, 0.0f);
        TrailRendererComponent_addTrailWidth(trail, 0.25f, 0.1f);
        TrailRendererComponent_addTrailWidth(trail, 0.02f, 0.5f);
        TrailRendererComponent_addTrailWidth(trail, 0.001f, 0.8f);
        trail->widthDecayRate = 2.0f;
        AddLinearVelocityComponent(hitEffect, Vector3Add(initialVelocity, Vector3Scale(rndVel, maxIniV * .5f)), Vector3Zero(), Vector3Zero());
        AddAutoDestroyComponent(hitEffect, 0.5f);
    }
}

int OnEnemyHit(SceneGraph* g, SceneObjectId target, SceneObjectId bullet)
{
    HealthComponent* health;
    SceneGraph_getComponentByType(g, target, psg.healthComponentId, (void**)&health, 0);
    if (health == NULL) {
        return 0;
    }
    health->health -= 1;
    if (health->health <= 0) {
        SceneGraph_destroyObject(g, target);

    }
    else {
        SpawnHitEffect(g, SceneGraph_getWorldPosition(g, bullet), Vector3Scale(GetVelocity(bullet), -.5f));
    }
    SceneGraph_destroyObject(g, bullet);
    return 1;
}

static void SpawnEnemy(SceneGraph* g, float x, float y, EnemyBehaviorComponent behavior)
{
    SceneObjectId enemy = SceneGraph_createObject(g, "enemy");
    SceneGraph_setLocalPosition(g, enemy, (Vector3) { x, 0, y });
    SceneGraph_setLocalRotation(g, enemy, (Vector3) { 0, 180, 0 });
    AddLinearVelocityComponent(enemy, Vector3Zero(), Vector3Zero(), Vector3Zero());
    SceneGraph_addComponent(g, enemy, psg.enemyBehaviorComponentId, &behavior);
    SceneObjectId propeller = add_propeller(enemy, 0, 0.08f, 0.6f);
    SceneGraph_addComponent(g, propeller, psg.updateCallbackComponentId,
        &(UpdateCallbackComponent) {
            .update = PropellerRotator,
            .floatValue = 3000.0f });
    AddMeshRendererComponent(enemy, psg.meshPlane2, 1.0f);
    AddHealthComponent(enemy, 3, 3);
    SceneGraph_addComponent(g, enemy, psg.targetComponentId,
        &(TargetComponent) {
            .colliderMask = 1,
            .radius = 0.5f,
            .onHit = OnEnemyHit });
    AddWingTrail(enemy, 1.0f, 0.3f, 0, 2.0f);
    AddWingTrail(enemy, -1.0f, 0.3f, 0, 2.0f);

    ShootingConfig shootingConfig = {
        .shotInterval = 1.5f,
        .bulletSpeed = 10.0f,
        .bulletLifetime = 2.0f,
        .bulletMask = 2,
        .onShoot = (OnShootFn)onShoot,
    };

    SceneGraph_addComponent(psg.sceneGraph, enemy, psg.shootingComponentId,
        &(ShootingComponent) {
            .spawnPoint = enemy,
            .shooting = 2,
            .config = shootingConfig,
        });
}

static void level1_e1(SceneGraph* g, struct LevelEvent* event)
{
    float x = event->levelEventData.x;
    float y = event->levelEventData.y;
    int n = event->levelEventData.n;
    float xx;
    Vector2* points = event->levelEventData.points;
    for (int i = 0; i < n; i += 1) {
        xx = x + i * event->levelEventData.formationStep;
        SpawnEnemy(g, xx, y, (EnemyBehaviorComponent) { .agility = 2.0f, .initialized = 1, .behaviorType = 0, .autoDestroy = 1, .points[0] = (Vector3) { points[0].x + xx, 0, points[0].y + y }, .points[1] = (Vector3) { points[1].x + xx, 0, points[1].y + y }, .points[2] = (Vector3) { points[2].x + xx, 0, points[2].y + y }, .points[3] = (Vector3) { points[3].x + xx, 0, points[3].y + y }, .pointCount = event->levelEventData.pointCount, .velocity = 3.50f });
    }
}

int GameStateLevel_Init()
{
    SceneGraph_clear(psg.sceneGraph);
    SceneObjectId systemsId = SceneGraph_createObject(psg.sceneGraph, "systems");
    // SceneGraph_addComponent(psg.sceneGraph, systemsId, psg.targetSpawnSystemId, NULL);
    SceneGraph_addComponent(psg.sceneGraph, systemsId, psg.cloudSystemId, NULL);
    LevelEvent events[128] = {
        { .time = 1.0f, .onTrigger = level1_e1, .levelEventData = (LevelEventData) { 
            .formationStep = 2.5f, .n = 2, .x = 7, .y = 15,
            .pointCount = 3, 
            .points[0]= (Vector2) { 0, -5 }, 
            .points[1] = (Vector2){-10, -10},
            .points[2] = (Vector2){-20, 10},
        } },
        { 0 }
    };
    for (int i=0;i<127;i++)
    {
        LevelEventData eventData = { 0 };
        eventData.x = GetRandomValue(-8, 8);
        eventData.y = GetRandomValue(15, 20);
        eventData.formationStep = 2.25f + GetRandomValue(0, 5) * .25f;
        eventData.n = GetRandomValue(1, 3);
        eventData.pointCount = 3;
        eventData.points[0] = (Vector2) { 0, -5 };
        eventData.points[1] = (Vector2) { (eventData.x > 0 ? -1 : 1) * GetRandomValue(0,4), GetRandomValue(-15,-9) };
        eventData.points[2] = (Vector2) { GetRandomValue(-8,8), 8 };
        events[i] = (LevelEvent) { .time = 1.5f * i + GetRandomValue(0,10)*.2f, .onTrigger = level1_e1, .levelEventData = eventData };
    }
    events[127] = (LevelEvent) { 0 };
    SceneGraph_addComponent(psg.sceneGraph, systemsId, psg.levelSystemId, &(LevelSystem) { .time = 0.0f, .events = events });
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