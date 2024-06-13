#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

static int _activeAllocationCount = 0;
static int _activeAllocationSize = 0;
static int _totalAllocationCount = 0;
static int _totalAllocationSize = 0;

void *tracked_malloc(size_t size, const char *file, int line) {
    printf("  Allocating %lld bytes from %s:%d\n", size, file, line);
    _totalAllocationCount++;
    _totalAllocationSize += size;
    _activeAllocationCount++;
    _activeAllocationSize += size;
    void *allocation = malloc(size + sizeof(size_t));
    *((size_t *)allocation) = size;
    return (void *)((size_t *)allocation + 1);
}

void tracked_free(void *ptr, const char *file, int line) {
    printf("  Freeing %p from %s:%d\n", ptr, file, line);
    if (ptr == NULL)
    {
        return;
    }
    size_t *size = (((size_t *)ptr) - 1);
    _activeAllocationCount--;
    _activeAllocationSize -= *size;
    free(size);
}

void* tracked_realloc(void *ptr, size_t size, const char *file, int line) {
    if (ptr == NULL)
    {
        return tracked_malloc(size, file, line);
    }
    printf("  Reallocating %p to %lld bytes from %s:%d\n", ptr, size, file, line);
    size_t *oldSize = (((size_t *)ptr) - 1);
    _activeAllocationSize -= *oldSize;
    _activeAllocationSize += size;
    _totalAllocationSize += size - *oldSize;
    void *allocation = realloc(oldSize, size + sizeof(size_t));
    *((size_t *)allocation) = size;
    return (void *)((size_t *)allocation + 1);
}

char *tracked_strdup(const char *str, const char *file, int line) {
    size_t len = strlen(str) + 1;
    char *newStr = (char *)tracked_malloc(len, file, line);
    memcpy(newStr, str, len);
    return newStr;
}

#define RL_MALLOC(sz) tracked_malloc(sz, __FILE__, __LINE__)
#define RL_REALLOC(ptr, sz) tracked_realloc(ptr, sz, __FILE__, __LINE__)
#define RL_FREE(ptr) tracked_free(ptr, __FILE__, __LINE__)
#define RL_STRDUP(str) tracked_strdup(str, __FILE__, __LINE__)

#include "raylib.h"

#include "shared/scene_graph/scene_graph.c"



int main() {
    printf("================= Running tests =================\n");
    printf("Testing scene graph...\n");
    SceneGraph* sceneGraph = SceneGraph_create();
    printf("Scene graph created\n");

    SceneObjectId rootId = SceneGraph_createObject(sceneGraph, "root");
    SceneObjectId child1Id = SceneGraph_createObject(sceneGraph, "child1");
    SceneObjectId child11Id = SceneGraph_createObject(sceneGraph, "child1-1");
    SceneObjectId child12Id = SceneGraph_createObject(sceneGraph, "child1-2");
    SceneObjectId child2Id = SceneGraph_createObject(sceneGraph, "child2");
    SceneObjectId child3Id = SceneGraph_createObject(sceneGraph, "child3");
    SceneGraph_setParent(sceneGraph, child1Id, rootId);
    SceneGraph_setParent(sceneGraph, child2Id, rootId);
    SceneGraph_setParent(sceneGraph, child3Id, rootId);
    SceneGraph_setParent(sceneGraph, child11Id, child1Id);
    SceneGraph_setParent(sceneGraph, child12Id, child1Id);

    SceneGraph_destroy(sceneGraph);
    printf("Scene graph test passed\n");
    printf("allocation count: %d, allocation size: %d\n", _activeAllocationCount, _activeAllocationSize);
    printf("total allocation count: %d, total allocation size: %d\n", _totalAllocationCount, _totalAllocationSize);
    return 0;
}