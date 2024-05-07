typedef void (*InitializeGameCodeFn)(void* storedState);
typedef void* (*UnloadGameCodeFn)();
typedef void (*DrawGameCodeFn)();
typedef void (*UpdateGameCodeFn)(float dt);

static DrawGameCodeFn dlDraw;
static UpdateGameCodeFn dlUpdate;
void TraceLog(int logType, const char *text, ...);

#if !defined(PLATFORM_WEB) && !defined(PLATFORM_DESKTOP)
#define PLATFORM_DESKTOP
#endif

#ifdef PLATFORM_DESKTOP

#include <stdio.h>
#include <windows.h>

void OSSleep(long duration)
{
    Sleep(duration);
}

static HMODULE GameCodeDLL;

static char dllPath[MAX_PATH];
double GetTime(void);


const char* Host_InitializeGameCode(void* storedState, const char *projectPath)
{
    if (dllPath[0] == 0)
    {
        float testStart = GetTime();
        HMODULE test = LoadLibraryA("game.dll");
        if (!test) {
            printf("Failed to load DLL: %lu\n", GetLastError());
            return "Failed to load DLL";
        }
        // Get the full path of the loaded DLL
        if (GetModuleFileNameA(test, dllPath, sizeof(dllPath)) == 0) {
            printf("Failed to get DLL path: %lu\n", GetLastError());
            FreeLibrary(test);
            return "Failed to get DLL path";
        } else {
            printf("Loaded DLL from: %s\n", dllPath);
        }
        FreeLibrary(test);
        float testStartDt = GetTime() - testStart;
        printf("Found DLL path in %.2fms\n", testStartDt * 1000.0f);
    }

    float compileStart = GetTime();
    // compile the DLL
    char command[1024];
#if defined(DEBUG)
    snprintf(command, sizeof(command), "make BUILD=debug dll PROJECTDIR=%s", projectPath);
    int compileResult = system(command);
#else
    snprintf(command, sizeof(command), "make dll PROJECTDIR=%s", projectPath);
    int compileResult = system(command);
    
#endif
    float compileDt = GetTime() - compileStart;
    printf("Compiled DLL in %.2fms\n", compileDt * 1000.0f);

    if (compileResult != 0) {
        printf("Failed to compile DLL: %d\n", compileResult);
        return "Failed to compile DLL";
    }

    float copyStart = GetTime();
    if (!CopyFileA(dllPath, "game_copy.dll", FALSE)) {
        printf("Failed to copy DLL: %lu\n", GetLastError());
        return "Failed to copy DLL";
    }
    float copyDt = GetTime() - copyStart;
    printf("Copied DLL in %.2fms\n", copyDt * 1000.0f);

    float loadStart = GetTime();
    GameCodeDLL = LoadLibraryA("game_copy.dll");
    float loadDt = GetTime() - loadStart;
    printf("Loaded DLL in %.2fms\n", loadDt * 1000.0f);
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
        return NULL;
    }

    return "Failed to load DLL";
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

const char* Host_InitializeGameCode(void* storedState, const char *projectPath)
{
    InitializeGameCode(0);
    dlDraw = GameCodeDraw;
    dlUpdate = GameCodeUpdate;
    return 0;
}

void* Host_UnloadGameCode()
{
    return 0;
}


void OSSleep(long duration)
{
    
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
