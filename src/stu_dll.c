#include <stdlib.h>
#include <stdio.h>

void InitializeGameCode(void *storedState)
{
    printf("InitializeGameCode successfully\n");
}

void* UnloadGameCode()
{
    printf("UnloadGameCode successfully\n");
    return NULL;
}