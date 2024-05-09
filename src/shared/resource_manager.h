#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include <raylib.h>
#include <inttypes.h>

extern const char* ResourceAssetPath;

typedef struct Resource {
    char* path;
    char* filePath;
    int32_t hash;
    void* data;
    void (*freeData)(void*);
} Resource;

typedef struct ResourceManager{
    Resource* resources;
    const char *projectPath;
    int count;
    int capacity;
} ResourceManager;

void ResourceManager_init(ResourceManager *resourceManager, const char* projectPath);
Model ResourceManager_loadModel(ResourceManager *resourceManager, const char* path);
Texture2D ResourceManager_loadTexture(ResourceManager *resourceManager, const char* path, int filter);
Font ResourceManager_loadFont(ResourceManager *ResourceManager, const char *path);

#endif