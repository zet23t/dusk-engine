typedef void (*InitializeGameCodeFn)(void* storedState);
typedef void* (*UnloadGameCodeFn)();
typedef void (*DrawGameCodeFn)();
typedef void (*UpdateGameCodeFn)(float dt);

static DrawGameCodeFn dlDraw;
static UpdateGameCodeFn dlUpdate;
void TraceLog(int logType, const char *text, ...);

#if PLATFORM_DESKTOP
#include <stdio.h>
#include <windows.h>


static HMODULE GameCodeDLL;

static char dllPath[MAX_PATH];
double GetTime(void);


void Host_InitializeGameCode(void* storedState)
{
#if !defined(DEBUG)
    if (dllPath[0] == 0)
    {
        float testStart = GetTime();
        HMODULE test = LoadLibraryA("game.dll");
        if (!test) {
            printf("Failed to load DLL: %lu\n", GetLastError());
            return;
        }
        // Get the full path of the loaded DLL
        if (GetModuleFileNameA(test, dllPath, sizeof(dllPath)) == 0) {
            printf("Failed to get DLL path: %lu\n", GetLastError());
            FreeLibrary(test);
            return;
        } else {
            printf("Loaded DLL from: %s\n", dllPath);
        }
        FreeLibrary(test);
        float testStartDt = GetTime() - testStart;
        printf("Found DLL path in %.2fms\n", testStartDt * 1000.0f);
    }

    float compileStart = GetTime();
    // compile the DLL
#if defined(DEBUG)
    int compileResult = system("make dll BUILD=debug");
#else
    int compileResult = system("make dll");
#endif
    float compileDt = GetTime() - compileStart;
    printf("Compiled DLL in %.2fms\n", compileDt * 1000.0f);

    if (compileResult != 0) {
        printf("Failed to compile DLL: %d\n", compileResult);
        return;
    }

    float copyStart = GetTime();
    if (!CopyFileA(dllPath, "game_copy.dll", FALSE)) {
        printf("Failed to copy DLL: %lu\n", GetLastError());
        return;
    }
    float copyDt = GetTime() - copyStart;
    printf("Copied DLL in %.2fms\n", copyDt * 1000.0f);

    float loadStart = GetTime();
    GameCodeDLL = LoadLibraryA("game_copy.dll");
    float loadDt = GetTime() - loadStart;
    printf("Loaded DLL in %.2fms\n", loadDt * 1000.0f);
#else
    GameCodeDLL = LoadLibraryA("game.dll");
#endif
    if (GameCodeDLL) {
        float initStart = GetTime();
        InitializeGameCodeFn init = (InitializeGameCodeFn)GetProcAddress(GameCodeDLL, "InitializeGameCode");
        if (init) {
            init(storedState);
            printf("InitializeGameCode successfully\n");
        } else {
            printf("Failed to get InitializeGameCode: %lu\n", GetLastError());
        }
        dlDraw = (DrawGameCodeFn)GetProcAddress(GameCodeDLL, "GameCodeDraw");
        dlUpdate = (UpdateGameCodeFn)GetProcAddress(GameCodeDLL, "GameCodeUpdate");
        float initDt = GetTime() - initStart;
        printf("Initialized game code in %.2fms\n", initDt * 1000.0f);
    }
}

void* Host_UnloadGameCode()
{
    if (!GameCodeDLL) {
        return NULL;
    }
    UnloadGameCodeFn unload = (UnloadGameCodeFn)GetProcAddress(GameCodeDLL, "UnloadGameCode");
    void* storedState = NULL;
    if (unload) {
        storedState = unload();
    }

    FreeLibrary(GameCodeDLL);

    return storedState;
}
#else


void InitializeGameCode(void*);
void GameCodeDraw();
void GameCodeUpdate(float dt);

void Host_InitializeGameCode(void* storedState)
{
    InitializeGameCode(0);
    dlDraw = GameCodeDraw;
    dlUpdate = GameCodeUpdate;
}

void* Host_UnloadGameCode()
{
    return 0;
}
#endif

void Host_GameCodeDraw()
{
    if (!dlDraw) {
        return;
    }
    dlDraw();
}

void Host_GameCodeUpdate(float dt)
{
    if (!dlUpdate) {
        return;
    }
    dlUpdate(dt);
}
