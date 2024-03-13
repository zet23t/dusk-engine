#include <windows.h>
#include <stdio.h>

typedef void (*InitializeGameCodeFn)(void *storedState);
typedef void* (*UnloadGameCodeFn)();

static HMODULE GameCodeDLL;

void InitializeGameCode(void *storedState)
{
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
    int compileResult = system("make dll");
    if (compileResult != 0) {
        printf("Failed to compile DLL: %d\n", compileResult);
        return;
    }

    if (!CopyFileA(dllPath, "game_copy.dll", FALSE)) {
        printf("Failed to copy DLL: %lu\n", GetLastError());
        return;
    }

    GameCodeDLL = LoadLibraryA("game_copy.dll");
    if (GameCodeDLL)
    {
        InitializeGameCodeFn init = (InitializeGameCodeFn)GetProcAddress(GameCodeDLL, "InitializeGameCode");
        if (init)
        {
            init(storedState);
        }
    }
}

void* UnloadGameCode()
{
    if (!GameCodeDLL)
    {
        return NULL;
    }
    UnloadGameCodeFn unload = (UnloadGameCodeFn)GetProcAddress(GameCodeDLL, "UnloadGameCode");
    void *storedState = NULL;
    if (unload)
    {
        storedState = unload();
    }

    FreeLibrary(GameCodeDLL);

    return storedState;
}