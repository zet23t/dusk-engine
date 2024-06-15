#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include <raylib.h>
#include <inttypes.h>

extern const char* ResourceAssetPath;

#define RESOURCE_TYPE_MODEL 0
#define RESOURCE_TYPE_TEXTURE 1
#define RESOURCE_TYPE_FONT 2
#define RESOURCE_TYPE_TEXT 3

typedef struct Resource {
    int32_t resourceType;
    int32_t hash;
    char* path;
    char* filePath;
    void* data;
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
Font ResourceManager_loadFont(ResourceManager *resourceManager, const char *path);
char* ResourceManager_loadText(ResourceManager *resourceManager, const char *path);
void ResourceManager_reloadAll(ResourceManager *resourceManager);
int ResourceManager_getModHash(ResourceManager *resourceManager, const char *path);

#endif