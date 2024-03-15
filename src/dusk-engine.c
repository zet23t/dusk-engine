#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#if PLATFORM_WEB
#include <emscripten.h>
#endif

void Host_InitializeGameCode(void *storedState);
void* Host_UnloadGameCode();

void Host_GameCodeDraw();
void Host_GameCodeUpdate(float dt);

int main(void)
{
#if PLATFORM_WEB

    emscripten_run_script(
        "window.addEventListener('keydown', function(e) {"
        "    if([32, 37, 38, 39, 40].indexOf(e.keyCode) > -1) {"
        "        e.preventDefault();"
        "    }"
        "}, false);");

#endif
#ifndef DEBUG
    SetTraceLogLevel(LOG_WARNING);
#endif
    // Initialization
    const int screenWidth = 640;
    const int screenHeight = 480;

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Scene graph testing");
    SetTargetFPS(1000);

    // init();

    // Camera3D camera = { 0 };
    // camera.position = (Vector3) { 30.0f, 30.0f, 10.0f };
    // camera.target = (Vector3) { 0.0f, 0.0f, 0.0f };
    // camera.up = (Vector3) { 0.0f, 1.0f, 0.0f };
    // camera.fovy = 45.0f;
    // camera.near = 0.1f;
    // camera.far = 100.0f;

    TraceLog(LOG_INFO, "Loop start\n");

    const int trackCount = 300;
    float drawTimes[trackCount];
    float updateTimes[trackCount];
    unsigned int trackIndex = 0;
    memset(drawTimes, 0, sizeof(drawTimes));
    memset(updateTimes, 0, sizeof(updateTimes));

    Host_InitializeGameCode(NULL);

#if PLATFORM_WEB
    DisableCursor();
#endif

    int showInfo = 0;
    int isPaused = 0;
    int isSlowmo = 0;
    int step = 0;
    // Main game loop
    while (!WindowShouldClose()) {
        // Update

        float t = GetTime();
        float dt = GetFrameTime();
        // assume 10fps is the minimum, after that we slow the game
        if (dt > 0.1f) dt = 0.1f;
        Host_GameCodeUpdate(isPaused && !step ? 0.0f : dt * (isSlowmo ? .1f : 1.0f));
        float updateDt = GetTime() - t;
        updateTimes[trackIndex % trackCount] = updateDt;

        // Draw
        BeginDrawing();
        ClearBackground((Color) { 120, 140, 160, 255 });
        t = GetTime();
        Host_GameCodeDraw();
        float drawDt = GetTime() - t;
        drawTimes[trackIndex % trackCount] = drawDt;

        if (IsKeyPressed(KEY_P)) {
            isPaused = !isPaused;
        }
        if (IsKeyPressed(KEY_O)) {
            isSlowmo = !isSlowmo;
        }
        step = 0;
        if (IsKeyPressed(KEY_N)) {
            step = 1;
        }

        if (IsKeyPressed(KEY_F7))
        {
            float reloadStart = GetTime();
            TraceLog(LOG_WARNING, "Unloading game code\n");
            void *storedState = Host_UnloadGameCode();
            float unloadTime = GetTime();
            TraceLog(LOG_WARNING, "Reloading game code\n");
            Host_InitializeGameCode(storedState);
            float loadedTime = GetTime();

            TraceLog(LOG_WARNING, "Reloaded game code in %.2fms (unloaded in %.2fms)\n", (loadedTime - reloadStart) * 1000.0f, (unloadTime - reloadStart) * 1000.0f);
        }

        // update();

        // BeginMode3D(camera);

        // draw(camera);
        // EndMode3D();

        trackIndex++;
        float minUpdate = updateTimes[0];
        float maxUpdate = updateTimes[0];
        float avgUpdate = updateTimes[0];
        for (int i = 1; i < trackCount; i++) {
            if (updateTimes[i] < minUpdate)
                minUpdate = updateTimes[i];
            if (updateTimes[i] > maxUpdate)
                maxUpdate = updateTimes[i];
            avgUpdate += updateTimes[i];
        }
        avgUpdate /= trackCount;

        float minDraw = drawTimes[0];
        float maxDraw = drawTimes[0];
        float avgDraw = drawTimes[0];
        for (int i = 1; i < trackCount; i++) {
            if (drawTimes[i] < minDraw)
                minDraw = drawTimes[i];
            if (drawTimes[i] > maxDraw)
                maxDraw = drawTimes[i];
            avgDraw += drawTimes[i];
        }
        avgDraw /= trackCount;

        char buffer[200];
        sprintf(buffer, "FPS: %d\n\nupdate: %.2f / %.2f / %.2fms\n\ndraw: %.2f / %.2f / %.2fms",
            GetFPS(),
            minUpdate * 1000.0f, avgUpdate * 1000.0f, maxUpdate * 1000.0f,
            minDraw * 1000.0f, avgDraw * 1000.0f, maxDraw * 1000.0f);

        if (trackIndex % trackCount == 0 && trackIndex > trackCount * 2) {
            // TraceLog(LOG_INFO, "%s\n", buffer);
        }
        if (IsKeyPressed(KEY_I)) {
            showInfo = !showInfo;
        }
        if (showInfo) {
            DrawText(buffer, 12, 12, 30, BLACK);
            DrawText(buffer, 10, 10, 30, WHITE);
        }

        EndDrawing();
    }

    // Cleanup
    CloseWindow();

    return 0;
}
