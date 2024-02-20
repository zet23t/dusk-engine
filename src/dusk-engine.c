#include <raylib.h>

#include "shared/scene_graph/scene_graph.h"

SceneGraph *sceneGraph;

void init()
{
    sceneGraph = SceneGraph_create();
    SceneObjectId obj = SceneGraph_createObject(sceneGraph, "element");
}

int main(void)
{
    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Hello Raylib");

    SetTargetFPS(60);

    init();

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update

        // Draw
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("Hello, Raylib!", 10, 10, 20, DARKGRAY);

        SceneGraph_updateTick(sceneGraph, GetFrameTime());

        EndDrawing();
    }

    // Cleanup
    CloseWindow();

    return 0;
}
