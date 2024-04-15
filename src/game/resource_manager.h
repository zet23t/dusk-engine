#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include <raylib.h>
#include <inttypes.h>

typedef struct Resource {
    const char* path;
    int32_t hash;
    void* data;
    void (*freeData)(void*);
} Resource;

typedef struct ResourceManager{
    Resource* resources;
    int count;
    int capacity;
} ResourceManager;

Model ResourceManager_loadModel(ResourceManager *resourceManager, const char* path);
Texture2D ResourceManager_loadTexture(ResourceManager *resourceManager, const char* path, int filter);
Font ResourceManager_loadFont(ResourceManager *ResourceManager, const char *path);

#endif