#ifndef __SCENE_GRAPH_H__
#define __SCENE_GRAPH_H__

#include <inttypes.h>
#include <raylib.h>
#include <stddef.h>

#define STRUCT_LIST_ELEMENT(type, name) \
    type* name;                         \
    int name##_count;                   \
    int name##_capacity;

#define STRUCT_LIST_ACQUIRE_FN(masterType, listType, name)                 \
    static listType* masterType##_acquire_##name(masterType* master)       \
    {                                                                      \
        if (master->name##_count == master->name##_capacity) {             \
            master->name##_capacity = master->name##_capacity == 0         \
                ? 4                                                        \
                : master->name##_capacity * 2;                             \
            master->name = realloc(                                        \
                master->name, master->name##_capacity * sizeof(listType)); \
        }                                                                  \
        listType* entry = &master->name[master->name##_count++];           \
        memset(entry, 0, sizeof(listType));                                \
        return entry;                                                      \
    }

#define SCENE_COMPONENT_FLAG_ENABLED 1

#define SCENE_OBJECT_FLAG_ENABLED 1
#define SCENE_OBJECT_FLAG_INITIALIZE 2
#define SCENE_OBJECT_FLAG_LOCAL_MATRIX_DIRTY 4
#define SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY 8

typedef struct SceneObjectId {
    uint32_t id;
    uint32_t version;
} SceneObjectId;

typedef struct SceneComponentTypeId {
    uint32_t id;
    uint32_t version;
} SceneComponentTypeId;

typedef struct SceneComponentId {
    uint32_t id;
    uint32_t version;
    SceneComponentTypeId typeId;
} SceneComponentId;

typedef struct SceneGraph SceneGraph;
typedef struct SceneObject SceneObject;
typedef struct SceneComponent SceneComponent;

typedef struct SceneObjectTransform {
    Vector3 position;
    Vector3 eulerRotationDegrees;
    Vector3 scale;
    Matrix localMatrix;
    Matrix worldMatrix;
    int32_t worldMatrixVersion;
} SceneObjectTransform;

typedef struct SceneComponentTypeMethods {
    void (*onInitialize)(SceneObject* sceneObject, SceneComponentId SceneComponent, void* componentData, void *initArg);
    void (*updateTick)(SceneObject* sceneObject, SceneComponentId SceneComponent,
        float delta, void* componentData);
    void (*draw)(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
        void* componentData, void* userdata);
    void (*sequentialDraw)(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
        void* componentData, void* userdata);
    void (*onDestroy)(SceneObject* sceneObject, SceneComponentId sceneComponent, void* componentData);
    
    int (*getValue)(SceneObject* sceneObject, SceneComponent *sceneComponent, void* componentData, char *name, int bufferSize, void* buffer);
    int (*setValue)(SceneObject* sceneObject, SceneComponent *sceneComponent, void* componentData, char *name, int bufferSize, void* buffer);
} SceneComponentTypeMethods;

typedef struct SceneComponent {
    SceneComponentId id;
    SceneObjectId objectId;
    SceneComponentTypeId typeId;
    char *name;
    uint32_t flags;
} SceneComponent;

typedef struct SceneComponentType {
    SceneComponentTypeId id;
    char* name;
    size_t dataSize;
    STRUCT_LIST_ELEMENT(SceneComponent, components)
    STRUCT_LIST_ELEMENT(uint8_t, componentData)
    SceneComponentTypeMethods methods;
} SceneComponentType;

typedef struct SceneObject {
    SceneObjectId id;
    SceneGraph* graph;
    char* name;
    int32_t flags;
    int32_t marker;
    int32_t parentWorldMatrixVersion;
    SceneObjectId parent;
    SceneObjectTransform transform;

    STRUCT_LIST_ELEMENT(SceneObjectId, children)
    STRUCT_LIST_ELEMENT(SceneComponentId, components)
} SceneObject;

typedef struct SceneGraph {
    int32_t versionCounter;
    int32_t markerCounter;
    STRUCT_LIST_ELEMENT(SceneObject, objects)
    STRUCT_LIST_ELEMENT(SceneComponentType, componentTypes)
} SceneGraph;

SceneGraph* SceneGraph_create();
void SceneGraph_destroy(SceneGraph* graph);
// deletes all objects and components - but registered types remain. Use for unloading scenes
void SceneGraph_clear(SceneGraph* graph);
SceneComponentTypeId SceneGraph_registerComponentType(SceneGraph* graph, const char* name,
    size_t dataSize, SceneComponentTypeMethods methods);
SceneComponentType* SceneGraph_getComponentType(SceneGraph* graph, SceneComponentTypeId componentType);
SceneComponentTypeId SceneGraph_getComponentTypeId(SceneGraph* graph, const char* name);
int SceneGraph_countLiveObjects(SceneGraph* graph);

SceneObjectId SceneGraph_createObject(SceneGraph* graph, const char* name);
void SceneGraph_destroyObject(SceneGraph* graph, SceneObjectId id);
SceneComponentId SceneGraph_addComponent(SceneGraph* graph, SceneObjectId id, SceneComponentTypeId componentType,
    void* componentData);
SceneObject* SceneGraph_getObject(SceneGraph* graph, SceneObjectId id);
const char *SceneGraph_getObjectName(SceneGraph* graph, SceneObjectId id);
SceneComponent* SceneGraph_getComponent(SceneGraph* graph, SceneComponentId id, void** componentData);
SceneComponent* SceneGraph_getComponentByType(SceneGraph* graph, SceneObjectId id, SceneComponentTypeId typeId, void** componentData, int atIndex);
SceneComponent* SceneGraph_getComponentOrFindByType(SceneGraph* graph, SceneObjectId id, SceneComponentId* componentId, SceneComponentTypeId typeId, void** componentData);

Vector3 SceneGraph_localToWorld(SceneGraph *graph, SceneObjectId id, Vector3 local);
Vector3 SceneGraph_getWorldPosition(SceneGraph* graph, SceneObjectId id);
Vector3 SceneGraph_getWorldForward(SceneGraph* graph, SceneObjectId id);
Vector3 SceneGraph_getWorldUp(SceneGraph* graph, SceneObjectId id);
Vector3 SceneGraph_getWorldRight(SceneGraph* graph, SceneObjectId id);
Vector3 SceneGraph_getLocalPosition(SceneGraph* graph, SceneObjectId id);
Vector3 SceneGraph_getLocalRotation(SceneGraph* graph, SceneObjectId id);
Vector3 SceneGraph_getLocalScale(SceneGraph* graph, SceneObjectId id);
void SceneGraph_setLocalPosition(SceneGraph* graph, SceneObjectId id, Vector3 position);
void SceneGraph_setLocalRotation(SceneGraph* graph, SceneObjectId id, Vector3 rotation);
void SceneGraph_setLocalScale(SceneGraph* graph, SceneObjectId id, Vector3 scale);
void SceneGraph_setLocalTransform(SceneGraph* graph, SceneObjectId id, Vector3 position,
    Vector3 rotation, Vector3 scale);
Matrix SceneObject_getLocalMatrix(SceneObject* object);
Matrix SceneObject_getWorldMatrix(SceneObject* object);
void SceneGraph_setParent(SceneGraph* graph, SceneObjectId id, SceneObjectId parentId);

// calls update methods of all components in order of type registration. No particular order of objects is followed
void SceneGraph_updateTick(SceneGraph* graph, float delta);
// calls draw methods of all components in order of type registration. No particular order of objects is followed
void SceneGraph_draw(SceneGraph* graph, Camera3D camera, void* userdata);
// calls sequential draw methods of all components in order of being added to the object. No particular order of objects is followed
// for root objects, but within a tree of objects, children are drawn after their parents and in the order they were added.
// This form of iteration is most inefficient (memory wise) and should be used only for UI or other cases where draw order is important.
// Certain optimizations can help: Any parent object without any child with a sequential draw component is be skipped. Therefore keeping
// sequential draw components used only in certain tree branches can help (e.g. one UI canvas parent that has nothing to do with 
// the world object part of the scene graph).
void SceneGraph_sequentialDraw(SceneGraph* graph, Camera3D camera, void* userdata);

SceneObject* SceneGraph_findObjectByName(SceneGraph* graph, const char *name, int includeDisabled);
SceneComponent* SceneGraph_findComponentByName(SceneGraph* graph, const char *name, int includeDisabled);
SceneObject* SceneGraph_findChildByName(SceneGraph* graph, SceneObjectId parentId, const char *name, int includeDisabled);
SceneComponent* SceneGraph_findChildComponentByName(SceneGraph* graph, SceneObjectId parentId, const char *name, int includeDisabled);
void SceneGraph_destroyComponent(SceneGraph* graph, SceneComponentId id);
void SceneGraph_printObject(SceneObject *object, const char *indent);
void SceneGraph_print(SceneGraph* graph);

void SceneGraph_setComponentEnabled(SceneGraph* graph, SceneComponentId id, int enabled);
int SceneGraph_isComponentEnabled(SceneGraph* graph, SceneComponentId id);
void SceneGraph_setObjectEnabled(SceneGraph* graph, SceneObjectId id, int enabled);
int SceneGraph_isObjectEnabled(SceneGraph* graph, SceneObjectId id);

int SceneGraph_getComponentValue(SceneGraph *graph, char* name, SceneComponentId componentId, int bufferSize, void* buffer);
int SceneGraph_setComponentValue(SceneGraph *graph, char* name, SceneComponentId componentId, int bufferSize, void* buffer);

#endif