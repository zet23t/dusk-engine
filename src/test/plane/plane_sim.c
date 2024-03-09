#include "config.h"
#include "math.h"
#include "plane_sim_g.h"
#include "shared/scene_graph/scene_graph.h"
#include <raymath.h>
#include <stdlib.h>

#include <string.h>

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

struct MeshMapping {
    const char* name;
    union {
        Mesh** mesh;
        struct {
            Mesh*** meshList;
            int* count;
        };
    };
    int isList;
};

int loadMeshes()
{
    const char* gltfMeshFile = "assets/meshes.glb";

    psg.model = LoadModel(gltfMeshFile);
    psg.meshPlane = NULL;
    struct MeshMapping meshMappings[] = {
        { "fighter-plane-1", { .mesh = &psg.meshPlane }, 0 },
        { "fighter-plane-2", { .mesh = &psg.meshPlane2 }, 0 },
        { "propeller", { .mesh = &psg.meshPropellerPin }, 0 },
        { "propeller-blade-1", { .mesh = &psg.meshPropellerBlade }, 0 },
        { "player-bullet-1", { .mesh = &psg.meshPlayerBullet }, 0 },
        { "target-1", { .mesh = &psg.meshTarget }, 0 },
        { "hit-particle-1", { .mesh = &psg.meshHitParticle1 }, 0 },
        { "leaftree-", { .meshList = &psg.leafTreeList, .count = &psg.leafTreeCount }, 1 },
        { "cloud.", { .meshList = &psg.cloudList, .count = &psg.cloudCount }, 1 },
        { "ui-border", { .mesh = &psg.meshUiBorder }, 0 },
        { 0 }
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
            if (meshMappings[j].isList) {
                if (strncmp(meshName, meshMappings[j].name, strlen(meshMappings[j].name)) == 0) {
                    Mesh*** list = meshMappings[j].meshList;
                    int count = *meshMappings[j].count;
                    *list = (Mesh**)realloc(*list, sizeof(Mesh*) * (count + 1));
                    (*list)[count] = &psg.model.meshes[i];
                    *meshMappings[j].count = count + 1;
                    TraceLog(LOG_INFO, "  Added mesh to list: %s", meshName);
                }
            } else if (strcmp(meshName, meshMappings[j].name) == 0) {
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

    return 0;
}

static char* previousText = NULL;

static int levelConfigLoad(int force)
{
    char* levelConfigText = LoadFileText("assets/level.json");
    if (levelConfigText == NULL) {
        TraceLog(LOG_ERROR, "Failed to load level config");
        if (psg.levelConfig == NULL) {
            psg.levelConfig = cJSON_CreateObject();
        }
        return 0;
    }
    if (previousText != NULL && strcmp(levelConfigText, previousText) == 0 && !force) {
        UnloadFileText(levelConfigText);
        return 0;
    }
    if (previousText) {
        UnloadFileText(previousText);
    }
    cJSON* loaded = cJSON_Parse(levelConfigText);
    previousText = levelConfigText;
    if (loaded == NULL) {
        TraceLog(LOG_ERROR, "Failed to parse level config");
        if (psg.levelConfig == NULL) {
            psg.levelConfig = cJSON_CreateObject();
        }
        return 0;
    }

    if (psg.levelConfig) {
        cJSON_Delete(psg.levelConfig);
    }
    psg.levelConfig = loaded;
    return 1;
}

static Shader shader;
static char* loadedTxVs = NULL;
static char* loadedTxFs = NULL;

static void shaderLoad(int isReload)
{
    if (isReload) {
        char* txVs = LoadFileText(TextFormat("assets/shaders/glsl%i/lighting.vs", GLSL_VERSION));
        char* txFs = LoadFileText(TextFormat("assets/shaders/glsl%i/fog.fs", GLSL_VERSION));
        if (txVs == NULL || txFs == NULL) {
            TraceLog(LOG_ERROR, "Failed to load shader files");
            return;
        }
        if (loadedTxFs != NULL && strcmp(txVs, loadedTxVs) == 0 && strcmp(txFs, loadedTxFs) == 0 && 1) {
            UnloadFileText(txVs);
            UnloadFileText(txFs);
            return;
        }
        if (loadedTxFs) {
            UnloadFileText(loadedTxVs);
            UnloadFileText(loadedTxFs);
        }
        loadedTxVs = txVs;
        loadedTxFs = txFs;
    }
    if (shader.id != 0 && isReload) {
        UnloadShader(shader);
    }

    // Load shader and set up some uniforms
    shader = LoadShader(TextFormat("assets/shaders/glsl%i/lighting.vs", GLSL_VERSION),
        TextFormat("assets/shaders/glsl%i/fog.fs", GLSL_VERSION));
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    // Ambient light level
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]) { 0.2f, 0.2f, 0.2f, 1.0f }, SHADER_UNIFORM_VEC4);

    float fogDensity = 0.15f;
    int fogDensityLoc = GetShaderLocation(shader, "fogDensity");
    SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);

    psg.litAmountIndex = GetShaderLocation(shader, "litAmount");

    for (int i = 0; i < psg.model.materialCount; i++) {
        psg.model.materials[i].shader = shader;
        for (int j = 0; j < MAX_MATERIAL_MAPS; j++) {
            if (psg.model.materials[i].maps[j].texture.id == 0) {
                continue;
            }
            SetTextureFilter(psg.model.materials[i].maps[j].texture, TEXTURE_FILTER_BILINEAR);
        }
    }
    if (!isReload) {
        // CreateLight(LIGHT_POINT, (Vector3){ 0, 2, 6 }, Vector3Zero(), WHITE, shader);
    }
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

static void AddWingTrail(SceneObjectId parent, float speed)
{
    if (_trailMaterial.shader.id == 0) {
        Texture2D texture = LoadTexture("assets/wing-trail.png");
        _trailMaterial = LoadMaterialDefault();
        SetMaterialTexture(&_trailMaterial, MATERIAL_MAP_DIFFUSE, texture);
    }
    SceneComponentId trailId = AddTrailRendererComponent(parent, 40.0f, 0.25f, (Vector3) { 0, 0, -speed }, 20, _trailMaterial, 0);
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

SceneObjectId plane_instantiate(Vector3 position)
{
    SceneObjectId plane = SceneGraph_createObject(psg.sceneGraph, "plane");
    SceneGraph_setLocalPosition(psg.sceneGraph, plane, position);
    AddMeshRendererComponent(plane, psg.meshPlane, 0.0f);

    SceneObjectId wingTrailLeft = SceneGraph_createObject(psg.sceneGraph, "wing-trail-left");
    SceneGraph_setParent(psg.sceneGraph, wingTrailLeft, plane);
    SceneGraph_setLocalPosition(psg.sceneGraph, wingTrailLeft, (Vector3) { 0.95f, 0, 0.55f });
    AddWingTrail(wingTrailLeft, 30);
    SceneObjectId wingTrailRight = SceneGraph_createObject(psg.sceneGraph, "wing-trail-right");
    SceneGraph_setParent(psg.sceneGraph, wingTrailRight, plane);
    SceneGraph_setLocalPosition(psg.sceneGraph, wingTrailRight, (Vector3) { -0.95f, 0, 0.55f });
    AddWingTrail(wingTrailRight, 30);

    SceneObjectId propeller = SceneGraph_createObject(psg.sceneGraph, "propeller");
    SceneGraph_setParent(psg.sceneGraph, propeller, plane);
    SceneGraph_setLocalPosition(psg.sceneGraph, propeller, (Vector3) { 0, 0.062696f, 0.795618f });
    AddMeshRendererComponent(propeller, psg.meshPropellerPin, 0.0f);

    for (int i = 0; i < 3; i += 1) {
        SceneObjectId propellerBlade = SceneGraph_createObject(psg.sceneGraph, "propeller-blade");
        SceneGraph_setParent(psg.sceneGraph, propellerBlade, propeller);
        SceneGraph_setLocalPosition(psg.sceneGraph, propellerBlade, (Vector3) { 0, 0, 0 });
        SceneGraph_setLocalRotation(psg.sceneGraph, propellerBlade, (Vector3) { 0, 0, 120 * i });
        AddMeshRendererComponent(propellerBlade, psg.meshPropellerBlade, 0.0f);
        // SceneObjectId propellerBladeTrail = SceneGraph_createObject(psg.sceneGraph, "propeller-blade-trail");
        // SceneGraph_setParent(psg.sceneGraph, propellerBladeTrail, propellerBlade);
        // SceneGraph_setLocalPosition(psg.sceneGraph, propellerBladeTrail, (Vector3) { 0.25f, 0, 0 });
        // AddWingTrail(propellerBladeTrail, 0);
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

void MeshRendererComponentRegister();
void PlaneBehaviorComponentRegister();
void LinearVelocityComponentRegister();
void ShootingComponentRegister();
void AutoDestroyComponentRegister();
void BulletComponentRegister();
void TargetComponentRegister();
void HealthComponentRegister();
void UpdateCallbackComponentRegister();
void EnemyPlaneBehaviorComponentRegister();
void MovementPatternComponentRegister();
void CameraComponentRegister();
void TargetHandlerComponentRegister();
void TextComponentRegister();
void ActionComponentRegister();
void TimerComponentRegister();
void TrailRendererComponentRegister();

void RegisterTargetSpawnSystem();
void CloudSystemRegister();
void LevelSystemRegister();

#include <stdio.h>

static int initScene()
{
    SceneGraph_clear(psg.sceneGraph);
    SceneObjectId systemsId = SceneGraph_createObject(psg.sceneGraph, "systems");
    // SceneGraph_addComponent(psg.sceneGraph, systemsId, psg.targetSpawnSystemId, NULL);
    SceneGraph_addComponent(psg.sceneGraph, systemsId, psg.cloudSystemId, NULL);
    SceneGraph_addComponent(psg.sceneGraph, systemsId, psg.levelSystemId, NULL);

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

int plane_sim_init()
{
    if (loadMeshes()) {
        return 1;
    }
    shaderLoad(0);
    levelConfigLoad(1);

    psg.sceneGraph = SceneGraph_create();

    RegisterTargetSpawnSystem();
    CloudSystemRegister();
    LevelSystemRegister();

    MeshRendererComponentRegister();
    PlaneBehaviorComponentRegister();
    LinearVelocityComponentRegister();
    ShootingComponentRegister();
    AutoDestroyComponentRegister();
    BulletComponentRegister();
    TargetComponentRegister();
    HealthComponentRegister();
    UpdateCallbackComponentRegister();
    EnemyPlaneBehaviorComponentRegister();
    MovementPatternComponentRegister();
    CameraComponentRegister();
    TargetHandlerComponentRegister();
    TextComponentRegister();
    ActionComponentRegister();
    TimerComponentRegister();
    TrailRendererComponentRegister();

    return initScene();
}

static int textIndex = 0;
static int autoreload = 0;
static int reloadTimer = 0;
void plane_sim_draw()
{

#if !defined(PLATFORM_WEB)
    if (IsKeyPressed(KEY_F6)) {
        autoreload = !autoreload;
        TraceLog(LOG_INFO, "autoreload: %d", autoreload);
    }
    if (IsKeyPressed(KEY_F5) || (autoreload && reloadTimer++ > 30)) {
        reloadTimer = 0;
        SetTraceLogLevel(LOG_WARNING);
        shaderLoad(1);
        if (levelConfigLoad(IsKeyPressed(KEY_F5))) {
            initScene();
        }
        #if defined(DEBUG)
        SetTraceLogLevel(LOG_INFO);
        #endif
    }
#endif

    Camera3D camera = CameraComponent_getCamera3D(psg.sceneGraph, psg.camera);

    camera.position = SceneGraph_getWorldPosition(psg.sceneGraph, psg.camera);
    Vector3 forward = SceneGraph_getWorldForward(psg.sceneGraph, psg.camera);
    Vector3 up = SceneGraph_getWorldUp(psg.sceneGraph, psg.camera);
    camera.target = Vector3Add(camera.position, forward);
    camera.up = up;
    camera.fovy = 10;
    camera.far = 256.0f;
    camera.near = 64.0f;

    BeginMode3D(camera);
    SceneGraph_draw(psg.sceneGraph, camera, NULL);
    SceneGraph_sequentialDraw(psg.sceneGraph, camera, NULL);
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
void UpdateGroundTileSystem();

void plane_sim_update(float dt)
{
    if (IsKeyPressed(KEY_R)) {
        psg.disableDrawMesh = !psg.disableDrawMesh;
    }
    psg.time += dt;
    psg.deltaTime = dt;
    SceneGraph_updateTick(psg.sceneGraph, dt);
    HandlePlayerInputUpdate();
    UpdateGroundTileSystem();
}