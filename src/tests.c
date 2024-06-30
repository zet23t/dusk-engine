#define TESTS

#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SENTINEL_SIZE 32
typedef struct Allocation {
    size_t size;
    const char* file;
    int line;
    int allocationIndex;
} Allocation;

static int _activeAllocationCount = 0;
static int _activeAllocationSize = 0;
static int _totalAllocationCount = 0;
static int _totalAllocationSize = 0;
static Allocation** _allocationTable = NULL;
static int _allocationTableCount = 0;
static int _allocationTableCapacity = 0;

static int _allocationCapacity = 0;
static Allocation** _allocations = NULL;


void tracked_sentinelCheck(void* ptr, const char* file, int line)
{
    Allocation* allocation = ((Allocation*)ptr) - 1;
    unsigned char *sentinel = (unsigned char*)ptr + allocation->size;
    for (int i = 0; i < SENTINEL_SIZE; i++) {
        if (sentinel[i] != 0b10101010) {
            printf("Memory corruption detected at %p (%s:%d), checked at %s:%d\n", ptr, 
                allocation->file, allocation->line, file, line);
            exit(1);
        }
    }
}

void tracked_sentinelCheckAll(const char *file, int line)
{
    for (int i = 0; i < _allocationTableCount; i++) {
        tracked_sentinelCheck(_allocationTable[i] + 1, file, line);
    }
}

void tracked_addAllocation(Allocation* allocation)
{
    if (_allocationTableCount == _allocationTableCapacity) {
        _allocationTableCapacity = _allocationTableCapacity == 0 ? 256 : _allocationTableCapacity * 2;
        _allocationTable = realloc(_allocationTable, _allocationTableCapacity * sizeof(Allocation));
    }
    _allocationTable[_allocationTableCount++] = allocation;
}

void tracked_removeAllocation(Allocation* allocation)
{
    for (int i = 0; i < _allocationTableCount; i++) {
        if (_allocationTable[i] == allocation) {
            _allocationTable[i] = _allocationTable[--_allocationTableCount];
            return;
        }
    }
}

int tracked_getAllocationIndex(Allocation* allocation)
{
    for (int i = 0; i < _allocationTableCount; i++) {
        if (_allocationTable[i] == allocation) {
            return i;
        }
    }
    return -1;
}

void* tracked_malloc(size_t size, const char* file, int line)
{
    // printf("  Allocating %lld bytes from %s:%d\n", size, file, line);
    _totalAllocationCount++;
    _totalAllocationSize += size;
    _activeAllocationCount++;
    _activeAllocationSize += size;
    Allocation* allocation = malloc(size + sizeof(Allocation) + SENTINEL_SIZE);
    tracked_addAllocation(allocation);
    memset(allocation + 1, 0b10101010, size + SENTINEL_SIZE);
    allocation->size = size;
    allocation->file = file;
    allocation->line = line;

    int freeIndex = -1;
    for (int i = 0; i < _allocationCapacity; i++) {
        if (_allocations[i] == NULL) {
            freeIndex = i;
            break;
        }
    }
    if (freeIndex == -1) {
        freeIndex = _allocationCapacity;
        int previousCapacity = _allocationCapacity;
        _allocationCapacity = _allocationCapacity == 0 ? 256 : _allocationCapacity * 2;
        _allocations = realloc(_allocations, _allocationCapacity * sizeof(Allocation*));
        memset(&_allocations[previousCapacity], 0, (_allocationCapacity - previousCapacity) * sizeof(Allocation*));
    }
    allocation->allocationIndex = freeIndex;
    _allocations[freeIndex] = allocation;

    return (void*)(allocation + 1);
}

void tracked_free(void* ptr, const char* file, int line)
{
    if (ptr == NULL) {
        return;
    }
    tracked_sentinelCheck(ptr, file, line);
    // printf("  Freeing %p from %s:%d\n", ptr, file, line);
    Allocation* allocation = ((Allocation*)ptr) - 1;
    tracked_removeAllocation(allocation);
    _allocations[allocation->allocationIndex] = NULL;
    _activeAllocationCount--;
    _activeAllocationSize -= allocation->size;
    free(allocation);
}

void* tracked_realloc(void* ptr, size_t size, const char* file, int line)
{
    if (ptr == NULL) {
        return tracked_malloc(size, file, line);
    }
    tracked_sentinelCheck(ptr, file, line);

    // printf("  Reallocating %p to %lld bytes from %s:%d\n", ptr, size, file, line);
    Allocation* allocation = ((Allocation*)ptr) - 1;
    int allocationIndex = tracked_getAllocationIndex(allocation);
    if (allocationIndex == -1) {
        printf("Failed to find allocation index for allocation %s:%d\n", allocation->file, allocation->line);
        exit(1);
    }
    _activeAllocationSize -= allocation->size;
    _activeAllocationSize += size;
    _totalAllocationSize += size - allocation->size;
    allocation = realloc(allocation, size + sizeof(Allocation) + SENTINEL_SIZE);
    _allocationTable[allocationIndex] = allocation;
    allocation->size = size;
    allocation->file = file;
    allocation->line = line;
    ptr = (void*)(allocation + 1);
    for (int i = 0; i < SENTINEL_SIZE; i++) {
        ((unsigned char*)ptr)[size + i] = 0b10101010;
    }
    return ptr;
}

char* tracked_strdup(const char* str, const char* file, int line)
{
    size_t len = strlen(str) + 1;
    char* newStr = (char*)tracked_malloc(len, file, line);
    memcpy(newStr, str, len);
    return newStr;
}

#define RL_MALLOC(sz) tracked_malloc(sz, __FILE__, __LINE__)
#define RL_REALLOC(ptr, sz) tracked_realloc(ptr, sz, __FILE__, __LINE__)
#define RL_FREE(ptr) tracked_free(ptr, __FILE__, __LINE__)
#define RL_STRDUP(str) tracked_strdup(str, __FILE__, __LINE__)
#define STRUCT_MALLOC(type, count) (type*)tracked_malloc(sizeof(type) * (count), __FILE__, __LINE__)
#define STRUCT_REALLOC(ptr, type, count) (type*)tracked_realloc(ptr, sizeof(type) * (count), __FILE__, __LINE__)

#include "raylib.h"
#include "shared/serialization/serializable_structs.h"

#include "shared/scene_graph/scene_graph.c"
#include "shared/serialization/reflection.c"

void checkMemLeaks()
{
    printf("allocation count: %d, allocation size: %d\n", _activeAllocationCount, _activeAllocationSize);
    if (_activeAllocationCount > 0) {
        printf("Active allocations:\n");
        for (int i = 0; i < _allocationCapacity; i++) {
            Allocation* allocation = _allocations[i];
            if (allocation != NULL) {
                printf("  %p: %lld bytes from %s:%d\n", allocation + 1, allocation->size, allocation->file, allocation->line);
            }
        }
        exit(1);
    }
    printf("total allocation count: %d, total allocation size: %d\n", _totalAllocationCount, _totalAllocationSize);
}

void testReflect()
{
    Position pos = { 1.0f, 2.0f, 3.0f };
    float* x;
    size_t resultSize;
    const char* resultType;
    int reflectError = Reflect_Position_retrieve("y", &pos, (void**)&x, &resultSize, &resultType);

    if (reflectError != 0) {
        printf("Failed to retrieve x: %d\n", reflectError);
        exit(1);
    }

    if (*x != 2.0f) {
        printf("Failed to retrieve x\n");
        exit(1);
    }

    float* z;
    TRS trs = { { 1.0f, 2.0f, 3.0f }, { 4.0f, 5.0f, 6.0f }, { 7.0f, 8.0f, 9.0f } };
    reflectError = Reflect_TRS_retrieve("position.z", &trs, (void**)&z, &resultSize, &resultType);
    if (reflectError == REFLECT_OK) {
        printf("  result: %f\n", *z);
        printf("  resultSize: %lld\n", resultSize);
        printf("  resultType: %s\n", resultType);

        if (*z != 3.0f) {
            printf("Failed to retrieve position.z: %f\n", *z);
            exit(1);
        }
    }
    if (reflectError != 0) {
        printf("Failed to retrieve position.z: %d\n", reflectError);
        exit(1);
    }

    M44 m44 = { { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f } };
    float* floatV;
    reflectError = Reflect_M44_retrieve("m[5]", &m44, (void**)&floatV, &resultSize, &resultType);
    if (reflectError == REFLECT_OK) {
        printf("  result: %f\n", *floatV);
        printf("  resultSize: %lld\n", resultSize);
        printf("  resultType: %s\n", resultType);
    }
    if (reflectError != 0) {
        printf("Failed to retrieve m[5]: %d\n", reflectError);
        exit(1);
    }
    reflectError = Reflect_M44_retrieve("m[16]", &m44, (void**)&floatV, &resultSize, &resultType);
    assert(reflectError == REFLECT_INDEX_OUT_OF_BOUNDS);

    FixedArrayTest fixedArrayTest = { { { 1.0f, 2.0f, 3.0f }, { 4.0f, 5.0f, 6.0f }, { 7.0f, 8.0f, 9.0f } } };
    float* fixedArrayTest_1_2;
    reflectError = Reflect_FixedArrayTest_retrieve("positions[1].z", &fixedArrayTest, (void**)&fixedArrayTest_1_2, &resultSize, &resultType);
    if (reflectError == REFLECT_OK) {
        printf("  result: %f\n", *fixedArrayTest_1_2);
        printf("  resultSize: %lld\n", resultSize);
        printf("  resultType: %s\n", resultType);

        if (*fixedArrayTest_1_2 != 6.0f) {
            printf("Failed to retrieve positions[1].z: %f\n", *fixedArrayTest_1_2);
            exit(1);
        }
    }

    DynamicArrayTest dynamicArrayTest = { 0 };
    M44 m44_1 = { { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f } };
    M44* m44List;
    reflectError = Reflect_DynamicArrayTest_retrieve("matrices", &dynamicArrayTest, (void**)&m44List, &resultSize, &resultType);
    assert(reflectError == REFLECT_OK);
    reflectError = Reflect_DynamicArrayTest_retrieve("matrices[0]", &dynamicArrayTest, (void**)&m44List, &resultSize, &resultType);
    assert(reflectError == REFLECT_INDEX_OUT_OF_BOUNDS);
    dynamicArrayTest.matrices = &m44_1;
    dynamicArrayTest.matrices_count = 1;
    reflectError = Reflect_DynamicArrayTest_retrieve("matrices[0]", &dynamicArrayTest, (void**)&m44List, &resultSize, &resultType);
    assert(reflectError == REFLECT_OK);
    assert(m44List == &m44_1);
    reflectError = Reflect_DynamicArrayTest_retrieve("matrices[0].m[5]", &dynamicArrayTest, (void**)&floatV, &resultSize, &resultType);
    assert(reflectError == REFLECT_OK);
    assert(*floatV == 6.0f);

    reflectError = Reflect_retrieve("matrices[0].m[5]", &dynamicArrayTest, "DynamicArrayTest", (void**)&floatV, &resultSize, &resultType);
    assert(reflectError == REFLECT_OK);
    assert(*floatV == 6.0f);
}

// model a behavior of an animal:
// States: idle, eating, sleeping, playing, hunting, mating, breeding, dying, fleeing
// State: eating; Substates: find food, move to food, eat food
// Variables: position, energy, health, age, gender, target, speed, direction
typedef struct Animal {
    Vector2 position;
    Vector2 target;
    Vector2 speed;
    float direction;
    float energy;
    float health;
    float age;
    int gender;
} Animal;

typedef struct BehaviorMachine BehaviorMachine;
typedef struct BehaviorNode BehaviorNode;

#define BEHAVIOR_RETURN_NONE -1
#define BEHAVIOR_RETURN_SUCCESS 0
#define BEHAVIOR_RETURN_FAILURE 1
#define BEHAVIOR_RETURN_RUNNING 2

#define BEHAVIOR_NODE_TYPE_SEQUENCE 0
#define BEHAVIOR_NODE_TYPE_SELECTOR 1
#define BEHAVIOR_NODE_TYPE_PARALLEL 2
#define BEHAVIOR_NODE_TYPE_INVERTER 3

typedef struct BehaviorRuntime {
    BehaviorMachine* machine;
    BehaviorNode* currentNode;
    int8_t* behaviorReturnValues;
    void *userData;
} BehaviorRuntime;

typedef int8_t (*BehaviorUpdate)(BehaviorRuntime *runtime, int previousReturnValue);

typedef struct BehaviorNode {
    struct BehaviorMachine* machine;
    struct BehaviorNode* parent;
    char name[32];
    int id;
    struct BehaviorNode* nodes;
    int nodeCount;
    int nodeType;
    BehaviorUpdate update;
} BehaviorNode;

typedef struct BehaviorMachine {
    BehaviorNode* master;
    int idCounter;
} BehaviorMachine;

BehaviorRuntime* BehaviorRuntime_create(BehaviorMachine* machine)
{
    BehaviorRuntime* runtime = STRUCT_MALLOC(BehaviorRuntime, 1);
    runtime->machine = machine;
    runtime->currentNode = machine->master;
    runtime->behaviorReturnValues = STRUCT_MALLOC(int8_t, machine->idCounter);
    memset(runtime->behaviorReturnValues, BEHAVIOR_RETURN_NONE, machine->idCounter * sizeof(int));
    runtime->userData = NULL;
    return runtime;
}

void BehaviorRuntime_free(BehaviorRuntime* self)
{
    RL_FREE(self->behaviorReturnValues);
    RL_FREE(self);
}

void BehaviorRuntime_update(BehaviorRuntime* self)
{
    int returnValue = BEHAVIOR_RETURN_NONE;
    while (returnValue != BEHAVIOR_RETURN_RUNNING) {
        if (self->currentNode == NULL) {
            self->currentNode = self->machine->master;
            memset(self->behaviorReturnValues, BEHAVIOR_RETURN_NONE, self->machine->idCounter * sizeof(int));
        }
        returnValue = self->currentNode->update(self, returnValue);
    }
}

static int8_t BehaviorNode_defaultUpdate(BehaviorRuntime* runtime, int previousReturnValue)
{
    return BEHAVIOR_RETURN_SUCCESS;
}

void BehaviorStateClass_addState(BehaviorNode* self, const char *name, BehaviorUpdate update)
{
    self->nodes = STRUCT_REALLOC(self->nodes, BehaviorNode, self->nodeCount + 1);
    BehaviorNode* child = &self->nodes[self->nodeCount];
    strncpy(child->name, name, sizeof(child->name) - 1);
    child->parent = self;
    child->machine = self->machine;
    child->nodes = NULL;
    child->nodeCount = 0;
    child->update = update != NULL ? update : BehaviorNode_defaultUpdate;
    child->id = self->machine->idCounter++;
    self->nodeCount++;
}

void BehaviorStateClass_free(BehaviorNode* self)
{
    printf("Freeing state %s\n", self->name);
    for (int i = 0; i < self->nodeCount; i++) {
        printf("Freeing child state %s\n", self->nodes[i].name);
        BehaviorStateClass_free(&self->nodes[i]);
    }
    if (self->nodes != NULL)
        RL_FREE(self->nodes);
}

void BehaviorMachineClass_free(BehaviorMachine* self)
{
    BehaviorStateClass_free(self->master);
    RL_FREE(self->master);
}

void testBehavior()
{
    BehaviorMachine* behaviorMachine = STRUCT_MALLOC(BehaviorMachine, 1);
    behaviorMachine->idCounter = 0;
    behaviorMachine->master = STRUCT_MALLOC(BehaviorNode, 1);
    BehaviorNode* master = behaviorMachine->master;
    master->machine = behaviorMachine;
    strncpy(master->name, "master", sizeof(master->name) - 1);
    master->nodeCount = 0;
    master->nodes = NULL;
    master->update = NULL;

    BehaviorStateClass_addState(master, "idle", NULL);
    BehaviorStateClass_addState(master, "eating", NULL);
    BehaviorStateClass_addState(master, "sleeping", NULL);
    BehaviorStateClass_addState(master, "playing", NULL);
    BehaviorStateClass_addState(master, "hunting", NULL);
    BehaviorStateClass_addState(master, "mating", NULL);
    BehaviorStateClass_addState(master, "breeding", NULL);
    BehaviorStateClass_addState(master, "dying", NULL);
    BehaviorStateClass_addState(master, "fleeing", NULL);


    BehaviorMachineClass_free(behaviorMachine);
    RL_FREE(behaviorMachine);
}

void testTrackedMalloc()
{
    int* intPtr = RL_MALLOC(sizeof(int));
    *intPtr = 42;
    RL_FREE(intPtr);

    intPtr = STRUCT_MALLOC(int, 1);
    *intPtr = 42;
    intPtr = STRUCT_REALLOC(intPtr, int, 2);
    intPtr[1] = 43;
    assert(intPtr[0] == 42);
    assert(intPtr[1] == 43);
    RL_FREE(intPtr);
}

int main()
{
    printf("================= Running tests =================\n");
    printf("Testing tracked malloc...\n");
    testTrackedMalloc();
    printf("Testing reflection...\n");
    testReflect();

    printf("Testing scene graph...\n");
    SceneGraph* sceneGraph = SceneGraph_create();
    SceneComponentTypeId testComponent = SceneGraph_registerComponentType(sceneGraph, "TestComponent", sizeof(TestComponent), (SceneComponentTypeMethods) { 0 });
    printf("Scene graph created\n");

    for (int i = 0; i < _allocationCapacity; i++) {
        if (_allocations[i] != NULL) {
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

    SceneComponentId testComponentId = SceneGraph_addComponent(sceneGraph, child12Id, testComponent, &(TestComponent) { .trs = {
                                                                                                                            .position = { 1.0f, 2.0f, 3.0f },
                                                                                                                        } });

    SceneComponentId foundComponent = SceneGraph_getSceneComponentIdByPath(sceneGraph, (SceneObjectId) { 0 }, "root/child1/child1-2#TestComponent");
    if (foundComponent.id != testComponentId.id || foundComponent.version != testComponentId.version) {
        printf("Failed to find component by path\n");
        exit(1);
    }
    printf("  Found component by path via root\n");

    foundComponent = SceneGraph_getSceneComponentIdByPath(sceneGraph, child1Id, "child1-2#TestComponent");
    if (foundComponent.id != testComponentId.id || foundComponent.version != testComponentId.version) {
        printf("Failed to find component by path\n");
        exit(1);
    }
    printf("  Found component by path via child1\n");

    int reflectError;
    float* resultFloat;
    size_t resultSize;
    const char* resultType;
    printf("  Retrieving position.x\n");

    reflectError = SceneGraph_retrieve(sceneGraph, child1Id,
        "child1-2#TestComponent.trs.position.x", (void**)&resultFloat,
        &resultSize, &resultType);

    if (reflectError != 0) {
        printf("Failed to retrieve position.x: %d\n", reflectError);
        exit(1);
    }
    if (*resultFloat != 1.0f) {
        printf("Failed to retrieve position.x: %f\n", *resultFloat);
        exit(1);
    }
    printf("  Retrieved position.x: %f\n", *resultFloat);

    SceneGraph_destroy(sceneGraph);
    printf("Scene graph test passed\n");
    checkMemLeaks();

    testBehavior();
    checkMemLeaks();

    printf("All tests passed\n");
    return 0;
}