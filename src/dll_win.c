#include <stdio.h>
#include <windows.h>

typedef void (*InitializeGameCodeFn)(void* storedState);
typedef void* (*UnloadGameCodeFn)();
typedef void (*DrawGameCodeFn)();
typedef void (*UpdateGameCodeFn)(float dt);

static HMODULE GameCodeDLL;
static DrawGameCodeFn dlDraw;
static UpdateGameCodeFn dlUpdate;

void InitializeGameCode(void* storedState)
{
#if !defined(DEBUG)
    HMODULE test = LoadLibraryA("game.dll");
    if (!test) {
        printf("Failed to load DLL: %lu\n", GetLastError());
        return;
    }
    // Get the full path of the loaded DLL
    char dllPath[MAX_PATH];
    if (GetModuleFileNameA(test, dllPath, sizeof(dllPath)) == 0) {
        printf("Failed to get DLL path: %lu\n", GetLastError());
        FreeLibrary(test);
        return;
    } else {
        printf("Loaded DLL from: %s\n", dllPath);
    }
    FreeLibrary(test);

// compile the DLL
#if defined(DEBUG)
    int compileResult = system("make dll BUILD=debug");
#else
    int compileResult = system("make dll");
#endif
    if (compileResult != 0) {
        printf("Failed to compile DLL: %d\n", compileResult);
        return;
    }

    if (!CopyFileA(dllPath, "game_copy.dll", FALSE)) {
        printf("Failed to copy DLL: %lu\n", GetLastError());
        return;
    }

    GameCodeDLL = LoadLibraryA("game_copy.dll");
#else
    GameCodeDLL = LoadLibraryA("game.dll");
#endif
    if (GameCodeDLL) {
        InitializeGameCodeFn init = (InitializeGameCodeFn)GetProcAddress(GameCodeDLL, "InitializeGameCode");
        if (init) {
            init(storedState);
            printf("InitializeGameCode successfully\n");
        } else {
            printf("Failed to get InitializeGameCode: %lu\n", GetLastError());
        }
        dlDraw = (DrawGameCodeFn)GetProcAddress(GameCodeDLL, "GameCodeDraw");
        dlUpdate = (UpdateGameCodeFn)GetProcAddress(GameCodeDLL, "GameCodeUpdate");
    }
}

void* UnloadGameCode()
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

void GameCodeDraw()
{
    if (!dlDraw) {
        return;
    }
    dlDraw();
}

void GameCodeUpdate(float dt)
{
    if (!dlUpdate) {
        return;
    }
    dlUpdate(dt);
}