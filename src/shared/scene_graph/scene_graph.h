#ifndef __SCENE_GRAPH_H__
#define __SCENE_GRAPH_H__

#include <stddef.h>
#include <inttypes.h>
#include <raylib.h>

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

#define SCENE_OBJECT_FLAG_ENABLED 1
#define SCENE_OBJECT_FLAG_INITIALIZE 2
#define SCENE_OBJECT_FLAG_LOCAL_MATRIX_DIRTY 4
#define SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY 8

typedef struct SceneObjectId {
    uint32_t id;
    uint32_t version;
} SceneObjectId;

typedef struct SceneComponentId {
    uint32_t id;
    uint32_t version;
} SceneComponentId;

typedef struct SceneComponentTypeId {
    uint32_t id;
    uint32_t version;
} SceneComponentTypeId;

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
    void (*initialize)(SceneObject* sceneObject, SceneComponentId SceneComponent, void* componentData);
    void (*updateTick)(SceneObject* sceneObject, SceneComponentId SceneComponent,
        float delta, void* componentData);
    void (*draw)(Camera3D camera, SceneObject *sceneObject, SceneComponentId sceneComponent,
        void *componentData, void *userdata);
} SceneComponentTypeMethods;

typedef struct SceneComponent {
    SceneComponentId id;
    SceneObjectId objectId;
    SceneComponentTypeId typeId;
} SceneComponent;

typedef struct SceneComponentType {
    SceneComponentTypeId id;
    const char* name;
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
    int32_t parentWorldMatrixVersion;
    SceneObjectId parent;
    SceneObjectTransform transform;

    STRUCT_LIST_ELEMENT(SceneObjectId, children)
    STRUCT_LIST_ELEMENT(SceneComponentId, components)
} SceneObject;

typedef struct SceneGraph {
    int32_t versionCounter;

    STRUCT_LIST_ELEMENT(SceneObject, objects)
    STRUCT_LIST_ELEMENT(SceneObjectId, rootElements)
    STRUCT_LIST_ELEMENT(SceneComponent, components)
    STRUCT_LIST_ELEMENT(SceneComponentType, componentTypes)
} SceneGraph;

SceneGraph* SceneGraph_create();
void SceneGraph_destroy(SceneGraph* graph);
SceneComponentTypeId SceneGraph_registerComponentType(SceneGraph* graph, const char* name,
    size_t dataSize, SceneComponentTypeMethods methods);

SceneObjectId SceneGraph_createObject(SceneGraph* graph, const char* name);
void SceneGraph_destroyObject(SceneGraph* graph, SceneObjectId id);
SceneComponentId SceneGraph_addComponent(SceneGraph* graph, SceneObjectId id, SceneComponentTypeId componentType,
    void* componentData);
void SceneGraph_setLocalPosition(SceneGraph* graph, SceneObjectId id, Vector3 position);
void SceneGraph_setLocalRotation(SceneGraph* graph, SceneObjectId id, Vector3 rotation);
void SceneGraph_setLocalScale(SceneGraph* graph, SceneObjectId id, Vector3 scale);
void SceneGraph_setLocalTransform(SceneGraph* graph, SceneObjectId id, Vector3 position,
    Vector3 rotation, Vector3 scale);
Matrix SceneObject_getLocalMatrix(SceneObject* object);
Matrix SceneObject_getWorldMatrix(SceneObject* object);
void SceneGraph_setParent(SceneGraph* graph, SceneObjectId id, SceneObjectId parentId);

void SceneGraph_updateTick(SceneGraph* graph, float delta);
void SceneGraph_draw(SceneGraph* graph, Camera3D camera, void* userdata);

void SceneGraph_destroyComponent(SceneGraph* graph, SceneComponentId id);

#endif