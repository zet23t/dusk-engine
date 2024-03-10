#include "../plane_sim_g.h"
#include <math.h>

#include "external/stb_perlin.h" // Required for: stb_perlin_fbm_noise3

#include "../util/util_math.h"

#define COLUMN_COUNT 6
#define ROW_COUNT 8
#define TILE_SIZE 8.0f
#define TILE_TYPE_COUNT 2
const char groundTypes[TILE_TYPE_COUNT] = { 'g', 'w' };

static Vector3 groundTileOffset = { -TILE_SIZE * (COLUMN_COUNT - 1) * .5f, -50, 0 };
static float offset = 0;
static float waterLineLevel = 0.5f;
static SceneObjectId groundTiles[ROW_COUNT * COLUMN_COUNT];

static MeshTileConfig FindMatchingTile(uint32_t cornersToMatch, int* rotation)
{
    MeshTileConfig matches[psg.meshTileCount];
    int rotations[psg.meshTileCount];
    int matchCount = 0;
    for (int i = 0; i < psg.meshTileCount; i++) {
        MeshTileConfig config = psg.meshTiles[i];
        for (int j = 0; j < 4; j++) {
            if (config.cornerConfigs[j] == cornersToMatch) {
                rotations[matchCount] = j;
                matches[matchCount++] = config;
            }
        }
    }

    if (matchCount == 0) {
        return (MeshTileConfig) { 0 };
    }

    int index = GetRandomValue(0, matchCount - 1);
    *rotation = rotations[index];
    return matches[index];
}

static float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

static float bilinearInterpolate(float topRight, float bottomRight, float bottomLeft, float topLeft, float x, float y)
{
    float bottom = lerp(bottomLeft, bottomRight, x);
    float top = lerp(topLeft, topRight, x);
    return lerp(bottom, top, y);
}

static SceneObjectId spawnTile(int x, int y)
{
    float freq = 0.25f;
    SceneObjectId id = SceneGraph_createObject(psg.sceneGraph, "GroundTile");
    float p1 = stb_perlin_turbulence_noise3(x * freq, (y - 1) * freq, 0, 0.5f, 0.5f, 3);
    float p2 = stb_perlin_turbulence_noise3(x * freq, y * freq, 0, 0.5f, 0.5f, 3);
    float p3 = stb_perlin_turbulence_noise3((x - 1) * freq, (y)*freq, 0, 0.5f, 0.5f, 3);
    float p4 = stb_perlin_turbulence_noise3((x - 1) * freq, (y - 1) * freq, 0, 0.5f, 0.5f, 3);
    char corners[4];
    corners[0] = p1 > waterLineLevel ? 'g' : 'w';
    corners[1] = p2 > waterLineLevel ? 'g' : 'w';
    corners[2] = p3 > waterLineLevel ? 'g' : 'w';
    corners[3] = p4 > waterLineLevel ? 'g' : 'w';
    uint32_t cornerConfig = *(uint32_t*)corners;
    int rot = 0;
    MeshTileConfig config = FindMatchingTile(cornerConfig, &rot);
    Vector3 rotation = { 0, 90 * rot, 0 };
    SceneGraph_setLocalRotation(psg.sceneGraph, id, rotation);

    if (config.mesh == NULL) {
        return id;
    }
    AddMeshRendererComponent(id, config.mesh, 1.0f);

    // scatter trees on ground

    for (int i = 0; i < 64; i++) {
        float rx = GetRandomFloat(-1.0f / 2.0f, 1.0f / 2.0f);
        float ry = GetRandomFloat(-1.0f / 2.0f, 1.0f / 2.0f);
        Vector3 position = {
            (i % 8 + rx) / 8.0f + 0.125f * .5f, // GetRandomFloat(0, 1),
            0,
            (i / 8 + ry) / 8.0f + 0.125f * .5f // GetRandomFloat(0, 1),
        };

        float px = (x + position.x) * TILE_SIZE;
        float pz = (y + position.z) * TILE_SIZE;

        float variation = stb_perlin_turbulence_noise3(px * .2f, (pz - 1) * .2f, 4.25f, 0.5f, 0.5f, 3) * 1.0f - .35f + GetRandomFloat(0, .25f);


        if (GetRandomValue(0, 100) > 50 + (int)(80.0f * (variation - .5f))) {
            continue;
        }

        char* grid = &config.mesh->name[2];
        float v = bilinearInterpolate(
            grid[1] == 'g', grid[0] == 'g', grid[3] == 'g', grid[2] == 'g',
            position.x, position.z);
        float scale = v * 2.0f - 1.0f;
        Vector3 worldPos = SceneGraph_localToWorld(psg.sceneGraph, id, (Vector3) { px, 0, pz });
        px = worldPos.x;
        pz = worldPos.z;
        scale += stb_perlin_turbulence_noise3(px * 1.2f, (pz - 1) * 1.2f, 2.25f, 0.5f, 0.5f, 3) * 1.0f - .35f + GetRandomFloat(0, .25f);
        if (scale < 0.5f) {
            continue;
        }
        scale += GetRandomFloat(-.25f, .25f);
        float type = stb_perlin_turbulence_noise3(px * 1.2f, (pz - 1) * 1.2f, 3.25f, 0.5f, 0.5f, 3);
        int treeType = (int)(type * psg.leafTreeCount);
        if (treeType >= psg.leafTreeCount) {
            treeType = psg.leafTreeCount - 1;
        }
        SceneObjectId tree = SceneGraph_createObject(psg.sceneGraph, "Tree");
        SceneGraph_setParent(psg.sceneGraph, tree, id);
        position.x = (position.x - .5f) * TILE_SIZE;
        position.z = (position.z - .5f) * TILE_SIZE;
        SceneGraph_setLocalScale(psg.sceneGraph, tree, (Vector3) { scale, scale, scale });
        SceneGraph_setLocalPosition(psg.sceneGraph, tree, position);
        Vector3 rotate = { GetRandomFloat(-10, 10), GetRandomFloat(0, 360), GetRandomFloat(-10, 10) };
        SceneGraph_setLocalRotation(psg.sceneGraph, tree, rotate);
        AddMeshRendererComponent(tree, psg.leafTreeList[treeType], 0.0f);
    }
    return id;
}

static int step = 0;
static int moveSpeedSetting = 0;
static float baseSpeed = 0.125f;
static cJSON* parsedGroundTileCfg;

static MappedVariable mappedGroundVariables[] = {
    { "baseSpeed", VALUE_TYPE_FLOAT, .floatValue = &baseSpeed },
    { "waterLineLevel", VALUE_TYPE_FLOAT, .floatValue = &waterLineLevel},
    { 0 },
};

void readParsedCfg()
{
    if (parsedGroundTileCfg == psg.levelConfig) {
        return;
    }

    parsedGroundTileCfg = psg.levelConfig;
    cJSON* groundTileCfg = cJSON_GetObjectItem(parsedGroundTileCfg, "groundTileCfg");
    if (groundTileCfg == NULL) {
        TraceLog(LOG_ERROR, "groundTileCfg not found in level config");
        return;
    }
    offset = 0;
    ReadMappedVariables(groundTileCfg, mappedGroundVariables);
}

void UpdateGroundTileSystem(SceneObject *object, SceneComponentId componentId, float dt, void *userdata)
{
    readParsedCfg();
    offset += psg.deltaTime * baseSpeed * (moveSpeedSetting == 0 ? 1.0f : 8.0f);
    if (IsKeyPressed(KEY_Q)) {
        moveSpeedSetting = !moveSpeedSetting;
    }
    while (offset > 1.0f) {
        offset -= 1.0f;
        step++;
        for (int x = 0; x < COLUMN_COUNT; x += 1) {
            SceneGraph_destroyObject(psg.sceneGraph, groundTiles[x]);
            for (int y = 0; y < ROW_COUNT - 1; y += 1) {
                groundTiles[y * COLUMN_COUNT + x] = groundTiles[(y + 1) * COLUMN_COUNT + x];
            }
            groundTiles[(ROW_COUNT - 1) * COLUMN_COUNT + x] = spawnTile(x, step + ROW_COUNT - 1);
        }
    }
    for (int x = 0; x < COLUMN_COUNT; x += 1) {
        for (int y = 0; y < ROW_COUNT; y += 1) {
            SceneObjectId id = groundTiles[y * COLUMN_COUNT + x];
            SceneObject* tile = SceneGraph_getObject(psg.sceneGraph, id);
            Vector3 position = {
                TILE_SIZE * x + groundTileOffset.x,
                groundTileOffset.y,
                TILE_SIZE * y - offset * TILE_SIZE,
            };
            if (tile == NULL) {
                id = spawnTile(x, y);
                groundTiles[y * COLUMN_COUNT + x] = id;
            }
            SceneGraph_setLocalPosition(psg.sceneGraph, id, position);
        }
    }
}

void GroundTileSystemRegister()
{
    psg.groundTileSystemId = SceneGraph_registerComponentType(psg.sceneGraph, "GroundTileSystem", 0,
        (SceneComponentTypeMethods) {
            .updateTick = UpdateGroundTileSystem
        });
}