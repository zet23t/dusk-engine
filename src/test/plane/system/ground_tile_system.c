#include "../plane_sim_g.h"
#include <math.h>

#include "external/stb_perlin.h" // Required for: stb_perlin_fbm_noise3

#define COLUMN_COUNT 6
#define ROW_COUNT 8
#define TILE_SIZE 8
#define TILE_TYPE_COUNT 2
const char groundTypes[TILE_TYPE_COUNT] = { 'g', 'w' };

static Vector3 groundTileOffset = { -TILE_SIZE * (COLUMN_COUNT - 1) * .5f, -50, 0 };
static float offset = 0;
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

static SceneObjectId spawnTile(int x, int y)
{
    SceneObjectId id = SceneGraph_createObject(psg.sceneGraph, "GroundTile");
    float p1 = stb_perlin_fbm_noise3(x * .2f, (y - 1) * .2f, 0, 0.5f, 0.5f, 3);
    float p2 = stb_perlin_fbm_noise3(x * .2f, y * .2f, 0, 0.5f, 0.5f, 3);
    float p3 = stb_perlin_fbm_noise3((x - 1) * .2f, (y) * .2f, 0, 0.5f, 0.5f, 3);
    float p4 = stb_perlin_fbm_noise3((x - 1) * .2f, (y - 1) * .2f, 0, 0.5f, 0.5f, 3);
    // float p1 = sinf(x) + cosf(y - 1); // stb_perlin_fbm_noise3(column * .2f, offset * .2f, 0, 0.5f, 0.5f, 3);
    // float p2 = sinf(x) + cosf(y); // stb_perlin_fbm_noise3(column * .2f, (offset - 1) * .2f, 0, 0.5f, 0.5f, 3);
    // float p3 = sinf(x - 1) + cosf(y); // stb_perlin_fbm_noise3((column - 1) * .2f, (offset - 1) * .2f, 0, 0.5f, 0.5f, 3);
    // float p4 = sinf(x - 1) + cosf(y - 1); // stb_perlin_fbm_noise3((column - 1) * .2f, offset * .2f, 0, 0.5f, 0.5f, 3);
    // TraceLog(LOG_INFO, "%d %d p1: %f, p2: %f, p3: %f, p4: %f", x, y, p1, p2, p3, p4);
    char corners[4];
    corners[0] = p1 > .0f ? 'g' : 'w';
    corners[1] = p2 > .0f ? 'g' : 'w';
    corners[2] = p3 > .0f ? 'g' : 'w';
    corners[3] = p4 > .0f ? 'g' : 'w';
    uint32_t cornerConfig = *(uint32_t*)corners;
    int rot;
    MeshTileConfig config = FindMatchingTile(cornerConfig, &rot);
    Vector3 rotation = { 0, 90 * rot, 0 };
    SceneGraph_setLocalRotation(psg.sceneGraph, id, rotation);

    if (config.mesh == NULL) {
        return id;
    }
    SceneGraph_addComponent(psg.sceneGraph, id, psg.meshRendererComponentId,
        &(MeshRendererComponent) {
            .material = psg.model.materials[1],
            .mesh = config.mesh,
        });
    return id;
}

int step = 0;
void UpdateGroundTileSystem()
{
    offset += psg.deltaTime * 1.125f;
    if (offset > 1.0f) {
        offset -= 1.0f;
        step++;
        for (int x = 0; x < COLUMN_COUNT; x++) {
            SceneGraph_destroyObject(psg.sceneGraph, groundTiles[x]);
            for (int y = 0; y < ROW_COUNT - 1; y++) {
                groundTiles[y * COLUMN_COUNT + x] = groundTiles[(y + 1) * COLUMN_COUNT + x];
            }
            groundTiles[(ROW_COUNT - 1) * COLUMN_COUNT + x] = spawnTile(x, step + ROW_COUNT - 1);
        }
    }
    for (int x = 0; x < COLUMN_COUNT; x++) {
        for (int y = 0; y < ROW_COUNT; y++) {
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