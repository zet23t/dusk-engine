#include "resource_manager.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <sys/stat.h>

const char* ResourceAssetPath;

static int32_t fileModHash(const char *filePath)
{
    int32_t hash = 0;
    for (int i = 0; filePath[i] != '\0'; i++)
    {
        hash = hash * 31 + filePath[i];
    }

    struct stat attr;
    if (stat(filePath, &attr) == 0)
    {
        hash = hash * 31 + (int32_t) attr.st_mtime;
    }

    return hash;
}

static Resource* findEntry(ResourceManager *resourceManager, const char* path)
{
    for (int i = 0; i < resourceManager->count; i++)
    {
        if (strcmp(resourceManager->resources[i].path, path) == 0)
        {
            int hash = fileModHash(path);
            if (hash != resourceManager->resources[i].hash)
            {
                printf("Unloading resource due to hash change: %s\n", path);
                if (resourceManager->resources[i].freeData != NULL)
                {
                    resourceManager->resources[i].freeData(resourceManager->resources[i].data);
                }
                resourceManager->resources[i] = resourceManager->resources[resourceManager->count - 1];
                resourceManager->count--;
                continue;
            }

            return &resourceManager->resources[i];
        }
    }

    return NULL;
}

static Resource* addEntry(ResourceManager *resourceManager, const char* path)
{
    if (resourceManager->count >= resourceManager->capacity)
    {
        resourceManager->capacity = resourceManager->capacity == 0 ? 16 : resourceManager->capacity * 2;
        resourceManager->resources = realloc(resourceManager->resources, resourceManager->capacity * sizeof(Resource));
    }

    Resource *resource = &resourceManager->resources[resourceManager->count++];
    resource->path = path;
    resource->data = NULL;
    resource->hash = fileModHash(path);

    return resource;
}

void freeModel (void *data)
{
    UnloadModel(*(Model*)data);
    free(data);
}

Model ResourceManager_loadModel(ResourceManager *resourceManager, const char* path)
{
    Resource *resource = findEntry(resourceManager, path);
    if (resource == NULL)
    {
        printf("Loading model: %s\n", path);
        resource = addEntry(resourceManager, path);
        Model model = LoadModel(path);
        resource->data = malloc(sizeof(Model));
        memcpy(resource->data, &model, sizeof(Model));
        resource->freeData = freeModel;
    }

    return *(Model*)resource->data;
}

void freeTexture (void *data)
{
    UnloadTexture(*(Texture2D*)data);
    free(data);
}

Texture2D ResourceManager_loadTexture(ResourceManager *resourceManager, const char* path, int filter)
{
    Resource *resource = findEntry(resourceManager, path);
    if (resource == NULL)
    {
        printf("Loading texture: %s\n", path);
        resource = addEntry(resourceManager, path);
        Texture2D texture = LoadTexture(path);
        resource->data = malloc(sizeof(Texture2D));
        memcpy(resource->data, &texture, sizeof(Texture2D));
        resource->freeData = freeTexture;
        SetTextureFilter(texture, filter);
    }

    return *(Texture2D*)resource->data;
}

static void freeFont(void *data)
{
    UnloadFont(*(Font*)data);
    free(data);
}

Font ResourceManager_loadFont(ResourceManager *ResourceManager, const char *path)
{
    Resource *resource = findEntry(ResourceManager, path);
    if (resource == NULL)
    {
        printf("Loading font: %s\n", path);
        resource = addEntry(ResourceManager, path);
        Font font = LoadFont(path);
        resource->data = malloc(sizeof(Font));
        memcpy(resource->data, &font, sizeof(Font));
        resource->freeData = freeFont;
    }

    return *(Font*)resource->data;
}