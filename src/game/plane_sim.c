#include "config.h"
#include "game_state_level.h"
#include "math.h"
#include "plane_sim_g.h"
#include "shared/touch_util.h"
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #if defined(PLATFORM_DESKTOP)
// #define GLSL_VERSION 330
// #else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
// #endif

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

    psg.model = ResourceManager_loadModel(&psg.resourceManager, gltfMeshFile);
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
            printf("  Added mesh tile: %s\n", meshName);
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
}

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
void EnemyBehaviorComponentRegister();

void RegisterTargetSpawnSystem();
void CloudSystemRegister();
void LevelSystemRegister();
void PlayerInputHandlerRegister();
void GroundTileSystemRegister();
void GameUiSystemRegister();

#define COMPONENT(t) void t##Register();
#include "component_list.h"
#undef COMPONENT

int plane_sim_init()
{
    if (loadMeshes()) {
        return 1;
    }
    MessageHub_init();
    shaderLoad(0);
    levelConfigLoad(1);

    int isReload = psg.sceneGraph != NULL;
    if (!isReload) {
        psg.sceneGraph = SceneGraph_create();
    }

    RegisterTargetSpawnSystem();
    CloudSystemRegister();
    LevelSystemRegister();
    PlayerInputHandlerRegister();
    GroundTileSystemRegister();

#define COMPONENT(t) t##Register();
#include "component_list.h"
#undef COMPONENT
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
    EnemyBehaviorComponentRegister();

    // Game ui should run last
    GameUiSystemRegister();

    if (!isReload) {
        GameStateLevel_Init();
    }

    return 1;
}

static int textIndex = 0;
static int autoreload = 0;
static int reloadTimer = 0;
void plane_sim_draw()
{

#if !defined(PLATFORM_WEB)
    if (IsKeyPressed(KEY_F1)) {
        SceneGraph_print(psg.sceneGraph, 1, 10);
    }
    if (IsKeyPressed(KEY_F6)) {
        autoreload = !autoreload;
        TraceLog(LOG_INFO, "autoreload: %d", autoreload);
    }
    if (IsKeyPressed(KEY_F5) || (autoreload && reloadTimer++ > 30)) {
        reloadTimer = 0;
        SetTraceLogLevel(LOG_WARNING);
        shaderLoad(1);
        if (levelConfigLoad(IsKeyPressed(KEY_F5))) {
            GameStateLevel_Init();
        }
#if defined(DEBUG)
        SetTraceLogLevel(LOG_INFO);
#endif
    }
#endif

    Camera3D camera = CameraComponent_getCamera3D(psg.sceneGraph, psg.camera);
    SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &camera.position.x, SHADER_UNIFORM_VEC3);

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

void plane_sim_update(float dt)
{
    ProcessTouches();

    Vector2 mouseDelta = GetMouseDelta();
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
        psg.hasMouse = 1;
    }
    if (IsKeyPressed(KEY_R)) {
        psg.disableDrawMesh = !psg.disableDrawMesh;
    }
    psg.time += dt;
    psg.deltaTime = dt;
    SceneGraph_updateTick(psg.sceneGraph, dt);
    MessageHub_process();
}