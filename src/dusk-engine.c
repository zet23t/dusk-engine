#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "shared/scene_graph/scene_graph.h"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

typedef struct Cube {
    Vector3 size;
    Color color;
} Cube;

typedef struct WaveComponent {
    float amplitude;
    float frequency;
    float phase;
    float speed;
} WaveComponent;

#define TEST_SCENE_GRAPH 1

#if TEST_SCENE_GRAPH
const char *systemTest = "sceneGraph";
SceneGraph* sceneGraph;

void Cube_draw(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
    void* componentData, void* userdata)
{
    Matrix m = SceneObject_getWorldMatrix(sceneObject);
    Cube* cube = (Cube*)componentData;
    rlPushMatrix();
    rlLoadIdentity();
    rlMultMatrixf(MatrixToFloat(m));
    DrawCubeV(Vector3Zero(), cube->size, cube->color);
    rlPopMatrix();
}

void Wave_updateTick(SceneObject* sceneObject, SceneComponentId sceneComponent,
    float delta, void* componentData)
{
    WaveComponent* wave = (WaveComponent*)componentData;
    wave->phase += delta * wave->speed;
    SceneGraph_setLocalPosition(sceneObject->graph, sceneObject->id, (Vector3) { 
        sceneObject->transform.position.x, sinf(wave->phase * wave->frequency) * wave->amplitude, sceneObject->transform.position.z });
}

void init()
{
    sceneGraph = SceneGraph_create();
    SceneComponentTypeId cubeCompoent = SceneGraph_registerComponentType(sceneGraph, "cube", sizeof(Cube),
        (SceneComponentTypeMethods) {
            .updateTick = NULL,
            .draw = Cube_draw });

    SceneComponentTypeId waveCompoent = SceneGraph_registerComponentType(sceneGraph, "wave", sizeof(WaveComponent),
        (SceneComponentTypeMethods) {
            .updateTick = Wave_updateTick,
        });

    const int count = 128;
    int n = 0;
    for (int j = 0; j < count; j++)
        for (int i = 0; i < count; i++) {
            SceneObjectId obj = SceneGraph_createObject(sceneGraph, "element");
            SceneGraph_addComponent(sceneGraph, obj, cubeCompoent, &(Cube) { .size = (Vector3) { .1f, .1f, .1f }, .color = (Color) { i, j, 0, 255 } });
            float x = (i / (float)count - .5f) * 100.0f;
            float y = (j / (float)count - .5f) * 100.0f;
            SceneGraph_setLocalPosition(sceneGraph, obj, (Vector3) { x * .25f, 0, y * .25f });

            SceneGraph_addComponent(sceneGraph, obj, waveCompoent, &(WaveComponent) { .amplitude = .5f, .frequency = 2.0f, .phase = n++ * .1f, .speed = 2.0f });
        }
}

void update()
{
    SceneGraph_updateTick(sceneGraph, GetFrameTime());
}

void draw(Camera3D camera)
{
    SceneGraph_draw(sceneGraph, camera, NULL);
}
#else
const char *systemTest = "ecs";
// test pico_ecs

#include "shared/ecs/pico_ecs.h"

static ecs_t* ecs;
static ecs_id_t cubeComponent;
static ecs_id_t waveComponent;
static ecs_id_t transformComponent;
static ecs_id_t drawSystem;
static ecs_id_t waveSystem;

static void cube_init(ecs_t* ecs, ecs_id_t entity_id, void* ptr, void* args)
{
    Cube* cube = (Cube*)ptr;
    Cube* argsCube = (Cube*)args;
    *cube = *argsCube;
}

static void wave_init(ecs_t* ecs, ecs_id_t entity_id, void* ptr, void* args)
{
    WaveComponent* wave = (WaveComponent*)ptr;
    WaveComponent* argsWave = (WaveComponent*)args;
    *wave = *argsWave;
}

static void transform_init(ecs_t* ecs, ecs_id_t entity_id, void* ptr, void* args)
{
    SceneObjectTransform* transform = (SceneObjectTransform*)ptr;
    SceneObjectTransform* argsTransform = (SceneObjectTransform*)args;
    *transform = *argsTransform;
}

ecs_ret_t wave_system(ecs_t* ecs,
    ecs_id_t* entities,
    int entity_count,
    ecs_dt_t dt,
    void* udata)
{
    for (int i = 0; i < entity_count; i++) {
        WaveComponent* wave = ecs_get(ecs, entities[i], waveComponent);
        SceneObjectTransform* transform = ecs_get(ecs, entities[i], transformComponent);
        wave->phase += dt * wave->speed;
        transform->position.y = sinf(wave->phase * wave->frequency) * wave->amplitude;
    }
    return 0;
}

ecs_ret_t draw_system(ecs_t* ecs,
    ecs_id_t* entities,
    int entity_count,
    ecs_dt_t dt,
    void* udata)
{
    for (int i = 0; i < entity_count; i++) {
        Cube* cube = ecs_get(ecs, entities[i], cubeComponent);
        SceneObjectTransform* transform = ecs_get(ecs, entities[i], transformComponent);

        transform->localMatrix = MatrixIdentity();
        transform->localMatrix = MatrixMultiply(transform->localMatrix, MatrixScale(transform->scale.x, transform->scale.y, transform->scale.z));
        transform->localMatrix = MatrixMultiply(transform->localMatrix, MatrixRotateXYZ((Vector3) { DEG2RAD * transform->eulerRotationDegrees.x, DEG2RAD * transform->eulerRotationDegrees.y, DEG2RAD * transform->eulerRotationDegrees.z }));
        transform->localMatrix = MatrixMultiply(transform->localMatrix, MatrixTranslate(transform->position.x, transform->position.y, transform->position.z));
        transform->worldMatrix = transform->localMatrix;

        rlPushMatrix();
        rlLoadIdentity();
        rlMultMatrixf(MatrixToFloat(transform->worldMatrix));
        DrawCubeV(Vector3Zero(), cube->size, cube->color);
        rlPopMatrix();
    }
    return 0;
}

void init()
{
    ecs = ecs_new(0xfff, NULL);
    cubeComponent = ecs_register_component(ecs, sizeof(Cube), cube_init, NULL);
    waveComponent = ecs_register_component(ecs, sizeof(WaveComponent), wave_init, NULL);
    transformComponent = ecs_register_component(ecs, sizeof(SceneObjectTransform), transform_init, NULL);

    drawSystem = ecs_register_system(ecs, draw_system, NULL, NULL, NULL);
    ecs_require_component(ecs, drawSystem, cubeComponent);
    ecs_require_component(ecs, drawSystem, transformComponent);

    waveSystem = ecs_register_system(ecs, wave_system, NULL, NULL, NULL);
    ecs_require_component(ecs, waveSystem, waveComponent);
    ecs_require_component(ecs, waveSystem, transformComponent);

    const int count = 128;
    int n = 0;
    for (int j = 0; j < count; j++)
        for (int i = 0; i < count; i++) {
            ecs_id_t obj = ecs_create(ecs);
            ecs_add(ecs, obj, cubeComponent, &(Cube) { .size = (Vector3) { .1f, .1f, .1f }, .color = (Color) { i, j, 0, 255 } });
            float x = (i / (float)count - .5f) * 100.0f;
            float y = (j / (float)count - .5f) * 100.0f;
            ecs_add(ecs, obj, transformComponent, &(SceneObjectTransform) { 
                .position = (Vector3) { x * .25f, 0, y * .25f },
                .scale = (Vector3) { 1, 1, 1 },
            });
            ecs_add(ecs, obj, waveComponent, &(WaveComponent) { .amplitude = .5f, .frequency = 2.0f, .phase = n++ * .1f, .speed = 2.0f });
        }

}

void update()
{
    ecs_update_system(ecs, waveSystem, GetFrameTime());
}

void draw(Camera3D camera)
{
    ecs_update_system(ecs, drawSystem, 0);
}

#endif

/*


INFO: ecs: FPS: 37
update: 1.03 / 1.58 / 3.64ms
draw: 19.68 / 28.74 / 65.37ms

INFO: sceneGraph: FPS: 37
update: 1.06 / 1.51 / 2.40ms
draw: 21.24 / 26.21 / 44.61ms

*/

#include "test/plane/plane_sim.h"

int main(void)
{
    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Hello Raylib");
    SetTargetFPS(1000);

    // init();

    Camera3D camera = { 0 };
    camera.position = (Vector3) { 30.0f, 30.0f, 10.0f };
    camera.target = (Vector3) { 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.near = 0.1f;
    camera.far = 100.0f;

    TraceLog(LOG_INFO, "Loop start\n");

    const int trackCount = 300;
    float drawTimes[trackCount];
    float updateTimes[trackCount];
    unsigned int trackIndex = 0;
    memset(drawTimes, 0, sizeof(drawTimes));
    memset(updateTimes, 0, sizeof(updateTimes));

    if (plane_sim_init())
    {
        return 1;
    }

    // Main game loop
    while (!WindowShouldClose()) {
        // Update

        float t = GetTime();
        plane_sim_update(GetFrameTime());
        float updateDt = GetTime() - t;
        updateTimes[trackIndex % trackCount] = updateDt;

        // Draw
        BeginDrawing();
        ClearBackground((Color) { 120, 140, 160, 255 });
        t = GetTime();
        plane_sim_draw();
        float drawDt = GetTime() - t;
        drawTimes[trackIndex % trackCount] = drawDt;

        // update();

        // BeginMode3D(camera);

        // draw(camera);
        // EndMode3D();

        trackIndex++;
        float minUpdate = updateTimes[0];
        float maxUpdate = updateTimes[0];
        float avgUpdate = updateTimes[0];
        for (int i = 1; i < trackCount; i++) {
            if (updateTimes[i] < minUpdate) minUpdate = updateTimes[i];
            if (updateTimes[i] > maxUpdate) maxUpdate = updateTimes[i];
            avgUpdate += updateTimes[i];
        }
        avgUpdate /= trackCount;

        float minDraw = drawTimes[0];
        float maxDraw = drawTimes[0];
        float avgDraw = drawTimes[0];
        for (int i = 1; i < trackCount; i++) {
            if (drawTimes[i] < minDraw) minDraw = drawTimes[i];
            if (drawTimes[i] > maxDraw) maxDraw = drawTimes[i];
            avgDraw += drawTimes[i];
        }
        avgDraw /= trackCount;

        char buffer[200];
        sprintf(buffer, "%s: FPS: %d\nupdate: %.2f / %.2f / %.2fms\ndraw: %.2f / %.2f / %.2fms\n\nUse Arrow keys to move the plane", 
            systemTest, GetFPS(),
            minUpdate * 1000.0f, avgUpdate * 1000.0f, maxUpdate * 1000.0f, 
            minDraw * 1000.0f, avgDraw * 1000.0f, maxDraw * 1000.0f);

        
        if (trackIndex % trackCount == 0 && trackIndex > trackCount * 2)
        {
            // TraceLog(LOG_INFO, "%s\n", buffer);
        }
        DrawText(buffer, 12, 12, 20, BLACK);
        DrawText(buffer, 10, 10, 20, WHITE);

        EndDrawing();
    }

    // Cleanup
    CloseWindow();

    return 0;
}
