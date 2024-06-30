#include "resource_manager.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <sys/stat.h>

const char* ResourceAssetPath;

static int32_t fileModHash(const char *filePath, const char *projectPath)
{
    int32_t hash = 0;
    for (int i = 0; filePath[i] != '\0'; i++)
    {
        hash = hash * 31 + filePath[i];
    }

    struct stat attr;
    char projectFilePath[(projectPath ? strlen(projectPath) : 0) + strlen(filePath) + 2];
    if (projectPath)
    {
        strcpy(projectFilePath, projectPath);
        strcat(projectFilePath, "/");
    }
    strcat(projectFilePath, filePath);
    
    if (stat(projectFilePath, &attr) == 0)
    {
        hash = hash * 31 + (int32_t) attr.st_mtime;
    }
    else if (stat(filePath, &attr) == 0)
    {
        hash = hash * 31 + (int32_t) attr.st_mtime;
    }
    

    return hash;
}


static void freeFont(void *data)
{
    UnloadFont(*(Font*)data);
    free(data);
}

void freeModel (void *data)
{
    UnloadModel(*(Model*)data);
    free(data);
}

void freeTexture (void *data)
{
    UnloadTexture(*(Texture2D*)data);
    free(data);
}


void ResourceManager_unload(ResourceManager *resourceManager, Resource *resource)
{
    switch (resource->resourceType)
    {
    case RESOURCE_TYPE_MODEL:
        freeModel(resource->data);
        break;
    case RESOURCE_TYPE_TEXTURE:
        freeTexture(resource->data);
        break;
    case RESOURCE_TYPE_TEXT:
        UnloadFileText(resource->data);
        break;
    case RESOURCE_TYPE_FONT:
        freeFont(resource->data);
        break;
    default:
        TraceLog(LOG_WARNING, "Unknown resource type: %d (%s)", resource->resourceType, resource->path);
        break;
    }

    resource->data = NULL;
}

static Resource* findEntry(ResourceManager *resourceManager, const char* path)
{
    for (int i = 0; i < resourceManager->count; i++)
    {
        if (strcmp(resourceManager->resources[i].path, path) == 0)
        {
            int hash = fileModHash(path, resourceManager->projectPath);
            if (hash != resourceManager->resources[i].hash)
            {
                printf("Unloading resource due to hash change: %s\n", path);
                ResourceManager_unload(resourceManager, &resourceManager->resources[i]);
                resourceManager->resources[i] = resourceManager->resources[resourceManager->count - 1];
                resourceManager->count--;
                continue;
            }

            // printf("Resource found: %s (%8x)\n", path, hash);

            return &resourceManager->resources[i];
        }
    }

    return NULL;
}

void ResourceManager_init(ResourceManager *resourceManager, const char* projectPath)
{
    TraceLog(LOG_INFO, "Initializing resource manager - project path: %s", projectPath ? projectPath : "NULL");
    resourceManager->resources = NULL;
    resourceManager->count = 0;
    resourceManager->capacity = 0;
    resourceManager->projectPath = projectPath;
}

static Resource* addEntry(ResourceManager *resourceManager, const char* path)
{
    // char projectPath = 
    if (resourceManager->count >= resourceManager->capacity)
    {
        resourceManager->capacity = resourceManager->capacity == 0 ? 16 : resourceManager->capacity * 2;
        resourceManager->resources = realloc(resourceManager->resources, resourceManager->capacity * sizeof(Resource));
    }

    Resource *resource = &resourceManager->resources[resourceManager->count++];
    resource->path = strdup(path);
    char filePath[(resourceManager->projectPath ? strlen(resourceManager->projectPath) : 0) + strlen(path) + 2];
    if (resourceManager->projectPath)
    {
        strcpy(filePath, resourceManager->projectPath);
        strcat(filePath, "/");
    }
    strcat(filePath, path);
    TraceLog(LOG_INFO, "check resource: %s or %s", filePath, path);
    if (FileExists(filePath))
    {
        resource->filePath = strdup(filePath);
    }
    else if (FileExists(path))
    {
        resource->filePath = strdup(path);
    }
    else
    {
        char engineAssetsPath[strlen(path) + strlen("engine/") + 1];
        strcpy(engineAssetsPath, "engine/");
        strcat(engineAssetsPath, path);
        resource->filePath = strdup(engineAssetsPath);
    }
    resource->data = NULL;
    resource->hash = fileModHash(path, resourceManager->projectPath);

    return resource;
}

int ResourceManager_getModHash(ResourceManager *resourceManager, const char *path)
{
    Resource *resource = findEntry(resourceManager, path);
    if (resource == NULL)
    {
        return 0;
    }

    return fileModHash(resource->filePath, resourceManager->projectPath);
}

Model ResourceManager_loadModel(ResourceManager *resourceManager, const char* path)
{
    Resource *resource = findEntry(resourceManager, path);
    if (resource == NULL)
    {
        printf("Loading model: %s\n", path);
        resource = addEntry(resourceManager, path);
        resource->resourceType = RESOURCE_TYPE_MODEL;
        Model model = LoadModel(resource->filePath);
        resource->data = malloc(sizeof(Model));
        memcpy(resource->data, &model, sizeof(Model));
    }
    else if (resource->data == NULL)
    {
        Model model = LoadModel(resource->filePath);
        resource->data = malloc(sizeof(Model));
        memcpy(resource->data, &model, sizeof(Model));
    }

    return *(Model*)resource->data;
}

char* ResourceManager_loadText(ResourceManager *resourceManager, const char *path)
{
    Resource *resource = findEntry(resourceManager, path);
    if (resource == NULL)
    {
        printf("Loading text: %s\n", path);
        resource = addEntry(resourceManager, path);
        resource->resourceType = RESOURCE_TYPE_TEXT;
        char *text = LoadFileText(resource->filePath);
        resource->data = text;
    }
    else if (resource->data == NULL)
    {
        char *text = LoadFileText(resource->filePath);
        resource->data = text;
    }

    return (char*)resource->data;
}

Texture2D ResourceManager_loadTexture(ResourceManager *resourceManager, const char* path, int filter)
{
    Resource *resource = findEntry(resourceManager, path);
    if (resource == NULL)
    {
        printf("Loading texture: %s\n", path);
        resource = addEntry(resourceManager, path);
        resource->resourceType = RESOURCE_TYPE_TEXTURE;
        Texture2D texture = LoadTexture(resource->filePath);
        resource->data = malloc(sizeof(Texture2D));
        memcpy(resource->data, &texture, sizeof(Texture2D));
        SetTextureFilter(texture, filter);
    }
    else if (resource->data == NULL)
    {
        Texture2D texture = LoadTexture(resource->filePath);
        resource->data = malloc(sizeof(Texture2D));
        memcpy(resource->data, &texture, sizeof(Texture2D));
        SetTextureFilter(texture, filter);
    }

    return *(Texture2D*)resource->data;
}

Font ResourceManager_loadFont(ResourceManager *ResourceManager, const char *path)
{
    Resource *resource = findEntry(ResourceManager, path);
    if (resource == NULL)
    {
        printf("Loading font: %s\n", path);
        resource = addEntry(ResourceManager, path);
        resource->resourceType = RESOURCE_TYPE_FONT;
        Font font = LoadFont(resource->filePath);
        resource->data = malloc(sizeof(Font));
        memcpy(resource->data, &font, sizeof(Font));
    }
    else if (resource->data == NULL)
    {
        Font font = LoadFont(resource->filePath);
        resource->data = malloc(sizeof(Font));
        memcpy(resource->data, &font, sizeof(Font));
    }

    return *(Font*)resource->data;
}

void ResourceManager_reloadAll(ResourceManager *resourceManager)
{
    printf("Reloading all resources\n");
    for (int i = 0; i < resourceManager->count; i++)
    {
        Resource *resource = &resourceManager->resources[i];
        switch (resource->resourceType)
        {
        case RESOURCE_TYPE_MODEL:
            ResourceManager_unload(resourceManager, resource);
            ResourceManager_loadModel(resourceManager, resource->path);
            break;
        case RESOURCE_TYPE_TEXTURE:
            ResourceManager_unload(resourceManager, resource);
            ResourceManager_loadTexture(resourceManager, resource->path, 0);
            break;
        case RESOURCE_TYPE_FONT:
            ResourceManager_unload(resourceManager, resource);
            ResourceManager_loadFont(resourceManager, resource->path);
            break;
        case RESOURCE_TYPE_TEXT:
            ResourceManager_unload(resourceManager, resource);
            ResourceManager_loadText(resourceManager, resource->path);
            break;
        default:
            break;
        }
    }
}