// Scene graph library
// This library provides a scene graph implementation with support for components and objects.
// It takes inspiration from ECS systems but focuses on components being managed as dense arrays.
// Iteration over component functions is done in the order of component type registration,
// however the fast iteration over components is done without any respective order of the object
// hierarchy. This is done to allow for efficient iteration over all components of a type. A separate
// function is provided for iterating over components in the order of object hierarchy which is
// to be used where this trait is needed.
#ifndef __SCENE_GRAPH_H__
#define __SCENE_GRAPH_H__

#include <inttypes.h>
#include <raylib.h>
#include <stddef.h>

#define STRUCT_LIST_ACQUIRE_FN(masterType, listType, name)                 \
    static listType* masterType##_acquire_##name(masterType* master)       \
    {                                                                      \
        if (master->name##_count == master->name##_capacity) {             \
            master->name##_capacity = master->name##_capacity == 0         \
                ? 4                                                        \
                : master->name##_capacity * 2;                             \
            master->name = RL_REALLOC(                                     \
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

typedef struct SceneGraph SceneGraph;
typedef struct SceneObject SceneObject;
typedef struct SceneComponent SceneComponent;

#include "shared/serialization/serializable_structs.h"

SceneGraph* SceneGraph_create();
void SceneGraph_destroy(SceneGraph* graph);
// deletes all objects and components - but registered types remain. Use for unloading scenes
void SceneGraph_clear(SceneGraph* graph);
SceneComponentTypeId SceneGraph_registerComponentType(SceneGraph* graph, const char* name,
    size_t dataSize, SceneComponentTypeMethods methods);
SceneComponentType* SceneGraph_getComponentType(SceneGraph* graph, SceneComponentTypeId componentType);
int SceneGraph_getComponentTypeCount(SceneGraph* graph, SceneComponentTypeId componentType);
SceneComponentTypeId SceneGraph_getComponentTypeId(SceneGraph* graph, const char* name);
int SceneGraph_countLiveObjects(SceneGraph* graph);

// path syntax is a forward slash separated list of object names with # separating the component type to be searched.
// an optional corner bracketed number can be added to the end to specify the index of the component of that type to be returned.
// e.g. "object1/object2#ComponentType" or "object1/object2#ComponentType[2]"
// If object id is {0}, the search starts from the root of the scene graph, otherwise it starts from the object with the given id.
// If the object id is valid but not found (due to deletion), no search is done and the function returns {0}.
SceneComponentId SceneGraph_getSceneComponentIdByPath(SceneGraph* graph, SceneObjectId objectId, const char* path);
// similar to SceneGraph_getSceneComponentIdByPath, but searches for a component of a specific type, ignoring anything after the # symbol.
// this function is used by SceneGraph_getSceneComponentIdByPath to find the component type id.
SceneComponentId SceneGraph_getSceneComponentIdByTypeAndPath(SceneGraph* graph, SceneObjectId objectId, const char* path, SceneComponentTypeId typeId, int componentIndex);

// SceneGraph_retrieve extends the getByPath functionality by using reflection to obtain a pointer to the struct's inner content. 
// The size of the struct is also returned. Important! The returned pointer is only valid until new components are created. When 
// the array is resized, the pointer may become invalid. Do not store the pointer for later use. 
// Example path for a struct with a field "int x":
//   object1/object2#ComponentType.x
int SceneGraph_retrieve(SceneGraph* graph, SceneObjectId objectId, const char *path, void **result, size_t *size, const char **typeName);

SceneObjectId SceneGraph_createObject(SceneGraph* graph, const char* name);
void SceneGraph_destroyObject(SceneGraph* graph, SceneObjectId id);
SceneComponentId SceneGraph_addComponent(SceneGraph* graph, SceneObjectId id, SceneComponentTypeId componentType,
    void* componentData);
SceneObject* SceneGraph_getObject(SceneGraph* graph, SceneObjectId id);
const char* SceneGraph_getObjectName(SceneGraph* graph, SceneObjectId id);
SceneComponent* SceneGraph_getComponent(SceneGraph* graph, SceneComponentId id, void** componentData);
SceneComponent* SceneGraph_getComponentByType(SceneGraph* graph, SceneObjectId id, SceneComponentTypeId typeId, void** componentData, int atIndex);
SceneComponent* SceneGraph_getComponentOrFindByType(SceneGraph* graph, SceneObjectId id, SceneComponentId* componentId, SceneComponentTypeId typeId, void** componentData);

Vector3 SceneGraph_localToWorld(SceneGraph* graph, SceneObjectId id, Vector3 local);
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
Matrix SceneObject_getToLocalMatrix(SceneObject* object);
void SceneGraph_setParent(SceneGraph* graph, SceneObjectId id, SceneObjectId parentId);

// calls update methods of all components in order of type registration. No particular order of objects is followed
void SceneGraph_updateTick(SceneGraph* graph, float delta);
// calls draw methods of all components in order of type registration. No particular order of objects is followed
void SceneGraph_draw(SceneGraph* graph, Camera3D camera, void* userdata);
void SceneGraph_draw2D(SceneGraph* graph, Camera2D camera, void* userdata);
// calls sequential draw methods of all components in order of being added to the object. No particular order of objects is followed
// for root objects, but within a tree of objects, children are drawn after their parents and in the order they were added.
// This form of iteration is most inefficient (memory wise) and should be used only for UI or other cases where draw order is important.
// Certain optimizations can help: Any parent object without any child with a sequential draw component is be skipped. Therefore keeping
// sequential draw components used only in certain tree branches can help (e.g. one UI canvas parent that has nothing to do with
// the world object part of the scene graph).
void SceneGraph_sequentialDraw(SceneGraph* graph, Camera3D camera, void* userdata);

SceneObject* SceneGraph_findObjectByName(SceneGraph* graph, const char* name, int includeDisabled);
SceneComponent* SceneGraph_findComponentByName(SceneGraph* graph, const char* name, int includeDisabled);
SceneObject* SceneGraph_findChildByName(SceneGraph* graph, SceneObjectId parentId, const char* name, int includeDisabled);
SceneComponent* SceneGraph_findChildComponentByName(SceneGraph* graph, SceneObjectId parentId, const char* name, int includeDisabled);
void SceneGraph_destroyComponent(SceneGraph* graph, SceneComponentId id);
int SceneGraph_setComponentName(SceneGraph* graph, SceneComponentId id, const char* name);
void SceneGraph_printObject(SceneObject* object, const char* indent, int printComponents);
void SceneGraph_print(SceneGraph* graph, int printComponents, int maxRootCount);

void SceneGraph_setComponentEnabled(SceneGraph* graph, SceneComponentId id, int enabled);
int SceneGraph_isComponentEnabled(SceneGraph* graph, SceneComponentId id);
void SceneGraph_setObjectEnabled(SceneGraph* graph, SceneObjectId id, int enabled);
int SceneGraph_isObjectEnabled(SceneGraph* graph, SceneObjectId id);

int SceneGraph_getComponentValue(SceneGraph* graph, char* name, SceneComponentId componentId, int bufferSize, void* buffer);
int SceneGraph_setComponentValue(SceneGraph* graph, char* name, SceneComponentId componentId, int bufferSize, void* buffer);
int SceneComponentIdEquals(SceneComponentId a, SceneComponentId b);
#endif