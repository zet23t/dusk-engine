#define TESTS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

static int _activeAllocationCount = 0;
static int _activeAllocationSize = 0;
static int _totalAllocationCount = 0;
static int _totalAllocationSize = 0;

typedef struct Allocation {
    size_t size;
    const char *file;
    int line;
    int allocationIndex;
} Allocation;
static int _allocationCapacity = 0;
static Allocation **_allocations = NULL;

void *tracked_malloc(size_t size, const char *file, int line) {
    // printf("  Allocating %lld bytes from %s:%d\n", size, file, line);
    _totalAllocationCount++;
    _totalAllocationSize += size;
    _activeAllocationCount++;
    _activeAllocationSize += size;
    Allocation *allocation = malloc(size + sizeof(Allocation));
    allocation->size = size;
    allocation->file = file;
    allocation->line = line;

    int freeIndex = -1;
    for (int i = 0; i < _allocationCapacity; i++)
    {
        if (_allocations[i] == NULL)
        {
            freeIndex = i;
            break;
        }
    }
    if (freeIndex == -1)
    {
        freeIndex = _allocationCapacity;
        int previousCapacity = _allocationCapacity;
        _allocationCapacity = _allocationCapacity == 0 ? 256 : _allocationCapacity * 2;
        _allocations = realloc(_allocations, _allocationCapacity * sizeof(Allocation*));
        memset(&_allocations[previousCapacity], 0, (_allocationCapacity - previousCapacity) * sizeof(Allocation*));
    }
    allocation->allocationIndex = freeIndex;
    _allocations[freeIndex] = allocation;

    return (void *)(allocation + 1);
}

void tracked_free(void *ptr, const char *file, int line) {
    if (ptr == NULL)
    {
        return;
    }
    // printf("  Freeing %p from %s:%d\n", ptr, file, line);
    Allocation *allocation = ((Allocation *)ptr) - 1;
    _allocations[allocation->allocationIndex] = NULL;
    _activeAllocationCount--;
    _activeAllocationSize -= allocation->size;
    free(allocation);
}

void* tracked_realloc(void *ptr, size_t size, const char *file, int line) {
    if (ptr == NULL)
    {
        return tracked_malloc(size, file, line);
    }
    // printf("  Reallocating %p to %lld bytes from %s:%d\n", ptr, size, file, line);
    Allocation *allocation = ((Allocation *)ptr) - 1;
    
    _activeAllocationSize -= allocation->size;
    _activeAllocationSize += size;
    _totalAllocationSize += size - allocation->size;
    allocation = realloc(allocation, size + sizeof(Allocation));
    allocation->size = size;
    return (void *)(allocation + 1);
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
#include "shared/serialization/serializable_structs.h"

#include "shared/scene_graph/scene_graph.c"
#include "shared/serialization/reflection.c"

typedef struct TestComponent {
    int a;
    float b;
} TestComponent;

void checkMemLeaks()
{
    printf("allocation count: %d, allocation size: %d\n", _activeAllocationCount, _activeAllocationSize);
    if (_activeAllocationCount > 0)
    {
        printf("Active allocations:\n");
        for (int i = 0; i < _allocationCapacity; i++)
        {
            Allocation *allocation = _allocations[i];
            if (allocation != NULL)
            {
                printf("  %p: %lld bytes from %s:%d\n", allocation + 1, allocation->size, allocation->file, allocation->line);
            }
        }
        exit(1);
    }
    printf("total allocation count: %d, total allocation size: %d\n", _totalAllocationCount, _totalAllocationSize);
}

int main() {
    printf("================= Running tests =================\n");

    printf("Testing scene graph...\n");
    SceneGraph* sceneGraph = SceneGraph_create();
    SceneComponentTypeId testComponent = SceneGraph_registerComponentType(sceneGraph, "TestComponent", sizeof(TestComponent), (SceneComponentTypeMethods){0});
    printf("Scene graph created\n");

    for (int i=0;i<_allocationCapacity;i++)
    {
        if (_allocations[i] != NULL)
        {
            printf("  %p: %lld bytes from %s:%d\n", _allocations[i], _allocations[i]->size, _allocations[i]->file, _allocations[i]->line);
        }
    }

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

    SceneComponentId testComponentId = SceneGraph_addComponent(sceneGraph, child12Id, testComponent, &(TestComponent){1, 2.0f});

    SceneComponentId foundComponent = SceneGraph_getSceneComponentIdByPath(sceneGraph, (SceneObjectId){0}, "root/child1/child1-2#TestComponent");
    if (foundComponent.id != testComponentId.id || foundComponent.version != testComponentId.version)
    {
        printf("Failed to find component by path\n");
        exit(1);
    }
    printf("  Found component by path via root\n");

    foundComponent = SceneGraph_getSceneComponentIdByPath(sceneGraph, child1Id, "child1-2#TestComponent");
    if (foundComponent.id != testComponentId.id || foundComponent.version != testComponentId.version)
    {
        printf("Failed to find component by path\n");
        exit(1);
    }
    printf("  Found component by path via child1\n");


    SceneGraph_destroy(sceneGraph);
    printf("Scene graph test passed\n");
    checkMemLeaks();

    Position pos = {1.0f, 2.0f, 3.0f};
    float *x;
    size_t resultSize;
    const char *resultType;
    int reflectError = Reflect_Position_retrieve("y", &pos, (void **)&x, &resultSize, &resultType);

    if (reflectError != 0)
    {
        printf("Failed to retrieve x: %d\n", reflectError);
        exit(1);
    }

    if (*x != 2.0f)
    {
        printf("Failed to retrieve x\n");
        exit(1);
    }

    float *z;
    TRS trs = {{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}, {7.0f, 8.0f, 9.0f}};
    reflectError = Reflect_TRS_retrieve("position.z", &trs, (void **)&z, &resultSize, &resultType);
    if (reflectError == REFLECT_OK)
    {
        printf("  result: %f\n", *z);
        printf("  resultSize: %lld\n", resultSize);
        printf("  resultType: %s\n", resultType);

        if (*z != 3.0f)
        {
            printf("Failed to retrieve position.z: %f\n", *z);
            exit(1);
        }
    }
    if (reflectError != 0)
    {
        printf("Failed to retrieve position.z: %d\n", reflectError);
        exit(1);
    }

    M44 m44 = {{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f}};
    float *m44_5;
    reflectError = Reflect_M44_retrieve("m[5]", &m44, (void **)&m44_5, &resultSize, &resultType);
    if (reflectError == REFLECT_OK)
    {
        printf("  result: %f\n", *m44_5);
        printf("  resultSize: %lld\n", resultSize);
        printf("  resultType: %s\n", resultType);
    }
    if (reflectError != 0)
    {
        printf("Failed to retrieve m[5]: %d\n", reflectError);
        exit(1);
    }
    reflectError = Reflect_M44_retrieve("m[16]", &m44, (void **)&m44_5, &resultSize, &resultType);
    assert(reflectError == REFLECT_INDEX_OUT_OF_BOUNDS);

    FixedArrayTest fixedArrayTest = {{{1.0f, 2.0f, 3.0f}, {4.0f, 5.0f, 6.0f}, {7.0f, 8.0f, 9.0f}}};
    float *fixedArrayTest_1_2;
    reflectError = Reflect_FixedArrayTest_retrieve("positions[1].z", &fixedArrayTest, (void **)&fixedArrayTest_1_2, &resultSize, &resultType);
    if (reflectError == REFLECT_OK)
    {
        printf("  result: %f\n", *fixedArrayTest_1_2);
        printf("  resultSize: %lld\n", resultSize);
        printf("  resultType: %s\n", resultType);

        if (*fixedArrayTest_1_2 != 6.0f)
        {
            printf("Failed to retrieve positions[1].z: %f\n", *fixedArrayTest_1_2);
            exit(1);
        }
    }

    printf("All tests passed\n");

    return 0;
}