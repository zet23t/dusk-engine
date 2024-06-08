typedef void (*InitializeGameCodeFn)(void* context);
typedef void (*UnloadGameCodeFn)();
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
    if (dllPath[0]== 0)
    {
        #if defined(DEBUG)
        sprintf(dllPath, "%s/_build/debug/platform_desktop/game.dll", projectPath);
        #else
        sprintf(dllPath, "%s/_build/release/platform_desktop/game.dll", projectPath);
        #endif
    }
    // if (dllPath[0] == 0)
    // {
    //     float testStart = GetTime();
    //     HMODULE test = LoadLibraryA("game.dll");
    //     if (!test) {
    //         printf("Failed to load DLL: %lu\n", GetLastError());
    //         return "Failed to load DLL";
    //     }
    //     // Get the full path of the loaded DLL
    //     if (GetModuleFileNameA(test, dllPath, sizeof(dllPath)) == 0) {
    //         printf("Failed to get DLL path: %lu\n", GetLastError());
    //         FreeLibrary(test);
    //         return "Failed to get DLL path";
    //     } else {
    //         printf("Loaded DLL from: %s\n", dllPath);
    //     }
    //     FreeLibrary(test);
    //     float testStartDt = GetTime() - testStart;
    //     printf("Found DLL path in %.2fms\n", testStartDt * 1000.0f);
    // }

    float compileStart = GetTime();
    // compile the DLL
    char command[1024];
    char commandResultBuffer[0x1000];
    static char *commandResult = NULL;
    if (commandResult) {
        free(commandResult);
        commandResult = NULL;
    }

#if defined(DEBUG)
    snprintf(command, sizeof(command), "make BUILD=debug dll PROJECTDIR=%s 2>&1", projectPath);
#else
    snprintf(command, sizeof(command), "make dll PROJECTDIR=%s 2>&1", projectPath);
#endif
    FILE* pipe = _popen(command, "r");
    if (!pipe) {
        return "Failed to open pipe";
    }
    while (!feof(pipe)) {
        if (fgets(commandResultBuffer, sizeof(commandResultBuffer), pipe) == NULL) {
            break;
        }
        int oldLen = commandResult!=NULL ? strlen(commandResult) : 0;
        int len = oldLen + strlen(commandResultBuffer) + 1;
        commandResult = realloc(commandResult, len);
        memset(commandResult + oldLen, 0, len - oldLen);
        strcat(commandResult, commandResultBuffer);
    }

    int compileResult = _pclose(pipe);

    float compileDt = GetTime() - compileStart;
    printf("Compiled DLL in %.2fms\n", compileDt * 1000.0f);

    if (compileResult != 0) {
        printf("Failed to compile DLL: %d\n%s", compileResult, commandResult);

        return commandResult;
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

void Host_UnloadGameCode()
{
    if (!GameCodeDLL) {
        return;
    }
    UnloadGameCodeFn unload = (UnloadGameCodeFn)GetProcAddress(GameCodeDLL, "UnloadGameCode");
    if (unload) {
        unload();
    }

    FreeLibrary(GameCodeDLL);
}
#else


void InitializeGameCode(void*);
void GameCodeDraw();
void GameCodeUpdate(float dt);

const char* Host_InitializeGameCode(void* storedState, const char *projectPath)
{
    InitializeGameCode(storedState);
    dlDraw = GameCodeDraw;
    dlUpdate = GameCodeUpdate;
    return 0;
}

void Host_UnloadGameCode()
{
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
