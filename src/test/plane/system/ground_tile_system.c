#include "../plane_sim_g.h"

#define COLUMN_COUNT 6
#define ROW_COUNT 8
#define TILE_SIZE 8
static Vector3 groundTileOffset = { -TILE_SIZE * (COLUMN_COUNT - 1) *.5f, -50, 0 };
static float offset = 0;
static SceneObjectId groundTiles[ROW_COUNT * COLUMN_COUNT];

void UpdateGroundTileSystem()
{
    for (int x = 0; x < COLUMN_COUNT && psg.meshTileCount > 0; x++) {
        for (int y = 0; y < ROW_COUNT; y++) {
            SceneObjectId id = groundTiles[y * COLUMN_COUNT + x];
            SceneObject* tile = SceneGraph_getObject(psg.sceneGraph, id);
            Vector3 position = { TILE_SIZE * x + groundTileOffset.x, groundTileOffset.y, TILE_SIZE * y };
            if (tile == NULL) {
                id = SceneGraph_createObject(psg.sceneGraph, "GroundTile");
                groundTiles[y * COLUMN_COUNT + x] = id;
                Vector3 rotation = { 0, 90 * GetRandomValue(0, 3), 0 };
                SceneGraph_setLocalRotation(psg.sceneGraph, id, rotation);
                SceneGraph_addComponent(psg.sceneGraph, id, psg.meshRendererComponentId,
                    &(MeshRendererComponent) {
                        .material = psg.model.materials[1],
                        .mesh = psg.meshTiles[GetRandomValue(0, psg.meshTileCount - 1)],
                    });
            }
            SceneGraph_setLocalPosition(psg.sceneGraph, id, position);
        }
    }
}