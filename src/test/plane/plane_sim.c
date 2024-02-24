#include "config.h"
#include "math.h"
#include "plane_sim_g.h"
#include "shared/scene_graph/scene_graph.h"
#include <raymath.h>
#include <stdlib.h>

#include <string.h>

struct MeshMapping {
    const char* name;
    Mesh** mesh;
};

int load_meshes()
{
    const char* gltfMeshFile = "assets/meshes.glb";

    psg.model = LoadModel(gltfMeshFile);
    psg.meshPlane = NULL;
    struct MeshMapping meshMappings[] = {
        { "fighter-plane-1", &psg.meshPlane },
        { "fighter-plane-2", &psg.meshPlane2 },
        { "propeller", &psg.meshPropellerPin },
        { "propeller-blade-1", &psg.meshPropellerBlade },
        { "player-bullet-1", &psg.meshPlayerBullet },
        { "target-1", &psg.meshTarget },
        { "hit-particle-1", &psg.meshHitParticle1 },
        { NULL, NULL }
    };

    for (int i = 0; i < psg.model.meshCount; i++) {
        char* meshName = psg.model.meshes[i].name;
        if (meshName[0] == 't' && meshName[1] == '-') {
            psg.meshTiles = realloc(psg.meshTiles, sizeof(MeshTileConfig) * (psg.meshTileCount + 1));
            uint32_t baserot = *(uint32_t*)(&meshName[2]);
            MeshTileConfig config = {
                .mesh = &psg.model.meshes[i],
                .cornerConfigs = {
                    // rotations char letter rotations of 0, 90, 180, 270 degrees
                    baserot,
                    baserot >> 8 | baserot << 24,
                    baserot >> 16 | baserot << 16,
                    baserot >> 24 | baserot << 8,
                }
            };
            psg.meshTiles[psg.meshTileCount] = config;

            psg.meshTileCount++;
            TraceLog(LOG_INFO, "  Added mesh tile: %s", meshName);
            continue;
        }

        for (int j = 0; meshMappings[j].name != NULL; j++) {
            if (strcmp(meshName, meshMappings[j].name) == 0) {
                *meshMappings[j].mesh = &psg.model.meshes[i];
            }
        }
    }

    for (int i = 0; meshMappings[i].name != NULL; i++) {
        if (*meshMappings[i].mesh == NULL) {
            TraceLog(LOG_ERROR, "Mesh not found: %s\n", meshMappings[i].name);
            return 1;
        }
    }

    for (int i = 0; i < psg.model.materialCount; i++) {
        for (int j = 0; j < MAX_MATERIAL_MAPS; j++) {
            if (psg.model.materials[i].maps[j].texture.id == 0) {
                continue;
            }
            SetTextureFilter(psg.model.materials[i].maps[j].texture, TEXTURE_FILTER_BILINEAR);
        }
    }

    return 0;
}

static void onShoot(SceneGraph* graph, SceneComponentId shooter, ShootingComponent* shootingComponent, ShootingConfig* shootingConfig)
{
    SceneComponent* shooting;
    shooting = SceneGraph_getComponent(graph, shooter, NULL);
    if (shooting == NULL) {
        return;
    }
    SceneObjectId bullet = SceneGraph_createObject(graph, "bullet");
    SceneGraph_setLocalPosition(psg.sceneGraph, bullet, SceneGraph_getWorldPosition(psg.sceneGraph, shootingComponent->spawnPoint));
    SceneGraph_addComponent(psg.sceneGraph, bullet, psg.meshRendererComponentId,
        &(MeshRendererComponent) {
            .material = psg.model.materials[1],
            .mesh = psg.meshPlayerBullet,
        });

    Vector3 forward = SceneGraph_getWorldForward(psg.sceneGraph, shootingComponent->spawnPoint);
    forward = Vector3Scale(forward, shootingConfig->bulletSpeed);
    forward.y = 0;
    SceneGraph_addComponent(graph, bullet, psg.linearVelocityComponentId,
        &(LinearVelocityComponent) {
            .velocity = forward,
            .drag = (Vector3) { 0, 0, 0 } });

    SceneGraph_addComponent(graph, bullet, psg.autoDestroyComponentId,
        &(AutoDestroyComponent) {
            .lifeTimeLeft = shootingConfig->bulletLifetime });
    SceneGraph_addComponent(graph, bullet, psg.bulletComponentId,
        &(BulletComponent) {
            .radius = 0.01f,
            .colliderFlags = 1,
            .damage = 1 });
}

SceneObjectId plane_instantiate(Vector3 position)
{
    SceneObjectId plane = SceneGraph_createObject(psg.sceneGraph, "plane");
    SceneGraph_setLocalPosition(psg.sceneGraph, plane, position);
    SceneGraph_addComponent(psg.sceneGraph, plane, psg.meshRendererComponentId,
        &(MeshRendererComponent) {
            .material = psg.model.materials[1],
            .mesh = psg.meshPlane,
        });

    SceneObjectId propeller = SceneGraph_createObject(psg.sceneGraph, "propeller");
    SceneGraph_setParent(psg.sceneGraph, propeller, plane);
    SceneGraph_setLocalPosition(psg.sceneGraph, propeller, (Vector3) { 0, 0.062696f, 0.795618f });
    SceneGraph_addComponent(psg.sceneGraph, propeller, psg.meshRendererComponentId,
        &(MeshRendererComponent) {
            .material = psg.model.materials[1],
            .mesh = psg.meshPropellerPin,
        });

    for (int i = 0; i < 3; i += 1) {
        SceneObjectId propellerBlade = SceneGraph_createObject(psg.sceneGraph, "propeller-blade");
        SceneGraph_setParent(psg.sceneGraph, propellerBlade, propeller);
        SceneGraph_setLocalPosition(psg.sceneGraph, propellerBlade, (Vector3) { 0, 0, 0 });
        SceneGraph_setLocalRotation(psg.sceneGraph, propellerBlade, (Vector3) { 0, 0, 120 * i });
        SceneGraph_addComponent(psg.sceneGraph, propellerBlade, psg.meshRendererComponentId,
            &(MeshRendererComponent) {
                .material = psg.model.materials[1],
                .mesh = psg.meshPropellerBlade,
            });
    }

    SceneGraph_addComponent(psg.sceneGraph, plane, psg.planeBehaviorComponentId,
        &(PlaneBehaviorComponent) {
            .phase = GetRandomValue(0, 1000),
            .rolling = 0,
            .time = 0,
            .prevVelocity = (Vector3) { 0, 0, 0 },
            .propeller = propeller,
        });

    const float drag = 5.0f;
    SceneGraph_addComponent(psg.sceneGraph, plane, psg.linearVelocityComponentId,
        &(LinearVelocityComponent) {
            .velocity = (Vector3) { 1, 0, 0 },
            .drag = (Vector3) { drag, drag, drag } });

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

void MeshRendererComponentRegister();
void PlaneBehaviorComponentRegister();
void LinearVelocityComponentRegister();
void ShootingComponentRegister();
void AutoDestroyComponentRegister();
void BulletComponentRegister();
void TargetComponentRegister();
void HealthComponentRegister();
void UpdateCallbackComponentRegister();

int plane_sim_init()
{
    if (load_meshes()) {
        return 1;
    }

    psg.sceneGraph = SceneGraph_create();

    MeshRendererComponentRegister();
    PlaneBehaviorComponentRegister();
    LinearVelocityComponentRegister();
    ShootingComponentRegister();
    AutoDestroyComponentRegister();
    BulletComponentRegister();
    TargetComponentRegister();
    HealthComponentRegister();
    UpdateCallbackComponentRegister();

    // for (int i = 0; i < 1000; i += 1) {
    //     plane_instantiate((Vector3) {
    //         GetRandomValue(-50, 50) * .5f,
    //         GetRandomValue(-20, 20) * .5f,
    //         GetRandomValue(-50, 50) * .5f });
    // }
    psg.playerPlane = plane_instantiate((Vector3) { 0, 0, 0 });
    // plane_instantiate((Vector3) { 0, 0, 1.5f });
    // plane_instantiate((Vector3) { 2.5f, 0, 0 });

    return 0;
}

static int textIndex = 0;
void plane_sim_draw()
{
    Camera3D camera = { 0 };
    camera.position = (Vector3) { 0, 100, -25 };
    camera.target = (Vector3) { 0, 0, 5 };
    camera.up = (Vector3) { 0, 1, 0 };
    camera.fovy = 10;
    camera.far = 1000;
    camera.near = 0.1f;

    BeginMode3D(camera);
    SceneGraph_draw(psg.sceneGraph, camera, NULL);
    EndMode3D();

    if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        int width = GetScreenWidth();
        int height = GetScreenHeight();
        const char* texts[] = {
            "Ratatatta",
            "Pew pew",
            "Bang bang",
        };
        const char* buffer = texts[textIndex % 3];
        if (GetRandomValue(0, 5) == 0) {
            textIndex++;
        }
        int textWidth = MeasureText(buffer, 40);
        int offsetX = GetRandomValue(-3, 3);
        int offsetY = GetRandomValue(-3, 3);
        int x = (width - textWidth) / 2 + offsetX;
        int y = height - 40 + offsetY;
        DrawText(buffer, x + 2, y + 2, 40, BLACK);
        DrawText(buffer, x, y, 40, WHITE);
    }
}

void HandlePlayerInputUpdate();
void HandleTargetSpawnSystem();
void UpdateGroundTileSystem();

void plane_sim_update(float dt)
{
    psg.time += dt;
    psg.deltaTime = dt;
    SceneGraph_updateTick(psg.sceneGraph, dt);
    HandlePlayerInputUpdate();
    HandleTargetSpawnSystem();
    UpdateGroundTileSystem();
}