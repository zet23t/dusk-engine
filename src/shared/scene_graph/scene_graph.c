#include "scene_graph.h"

#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SceneObject* SceneGraph_getObject(SceneGraph* graph, SceneObjectId id);

SceneGraph* SceneGraph_create()
{
    SceneGraph* graph = malloc(sizeof(SceneGraph));
    memset(graph, 0, sizeof(SceneGraph));
    return graph;
}

int SceneGraph_countLiveObjects(SceneGraph* graph)
{
    int count = 0;
    for (int i = 0; i < graph->objects_count; i++) {
        if (graph->objects[i].id.version != 0) {
            count++;
        }
    }
    return count;
}

void SceneGraph_destroy(SceneGraph* graph)
{
    for (int i = 0; i < graph->objects_count; i++) {
        SceneGraph_destroyObject(graph, graph->objects[i].id);
    }
    for (int i = 0; i < graph->componentTypes_count; i++) {
        SceneComponentType* type = &graph->componentTypes[i];
        free(type->name);
        free(type->componentData);
    }
    free(graph->componentTypes);
    free(graph->objects);
    free(graph);
}

void SceneGraph_clear(SceneGraph* graph)
{
    for (int i = 0; i < graph->objects_count; i++) {
        SceneGraph_destroyObject(graph, graph->objects[i].id);
    }
}

STRUCT_LIST_ACQUIRE_FN(SceneGraph, SceneObject, objects)
STRUCT_LIST_ACQUIRE_FN(SceneGraph, SceneComponentType, componentTypes)
STRUCT_LIST_ACQUIRE_FN(SceneComponentType, SceneComponent, components)
STRUCT_LIST_ACQUIRE_FN(SceneObject, SceneComponentId, components)
STRUCT_LIST_ACQUIRE_FN(SceneObject, SceneObjectId, children)

SceneComponentTypeId SceneGraph_registerComponentType(SceneGraph* graph, const char* name,
    size_t dataSize, SceneComponentTypeMethods methods)
{
    for (int i = 0; i < graph->componentTypes_count; i += 1) {
        if (strcmp(graph->componentTypes[i].name, name) == 0) {
            TraceLog(LOG_WARNING, "SceneGraph_registerComponentType(%s): component type already registered", name);
            if (graph->componentTypes[i].dataSize != dataSize) {
                TraceLog(LOG_ERROR, "SceneGraph_registerComponentType(%s): component type already registered with different data size; invalidating existing components", name);
                for (int j = 0; j < graph->componentTypes[i].components_count; j += 1) {
                    SceneComponent* component = &graph->componentTypes[i].components[j];
                    component->id.version = 0;
                }
                graph->componentTypes[i].dataSize = dataSize;
                free(graph->componentTypes[i].componentData);
                graph->componentTypes[i].componentData = NULL;
                graph->componentTypes[i].componentData_capacity = 0;
                graph->componentTypes[i].componentData_count = 0;
            }
            graph->componentTypes[i].methods = methods;
            return graph->componentTypes[i].id;
        }
    }

    SceneComponentType* type = SceneGraph_acquire_componentTypes(graph);
    type->id.id = graph->componentTypes_count - 1;
    type->id.version = ++graph->versionCounter;
    type->name = strdup(name);
    type->dataSize = dataSize;
    type->methods = methods;
    type->componentData = NULL;
    type->componentData_capacity = 0;
    type->componentData_count = 0;
    return type->id;
}

Matrix SceneObject_getLocalMatrix(SceneObject* object)
{
    if (object->flags & SCENE_OBJECT_FLAG_LOCAL_MATRIX_DIRTY) {
        // optimize this later
        object->transform.localMatrix = MatrixIdentity();
        object->transform.localMatrix = MatrixMultiply(object->transform.localMatrix, MatrixScale(object->transform.scale.x, object->transform.scale.y, object->transform.scale.z));
        object->transform.localMatrix = MatrixMultiply(object->transform.localMatrix, MatrixRotateXYZ((Vector3) { DEG2RAD * object->transform.eulerRotationDegrees.x, DEG2RAD * object->transform.eulerRotationDegrees.y, DEG2RAD * object->transform.eulerRotationDegrees.z }));
        object->transform.localMatrix = MatrixMultiply(object->transform.localMatrix, MatrixTranslate(object->transform.position.x, object->transform.position.y, object->transform.position.z));
        object->flags &= ~SCENE_OBJECT_FLAG_LOCAL_MATRIX_DIRTY;
        object->flags |= SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY;
        object->transform.worldMatrixVersion++;
    }

    return object->transform.localMatrix;
}

Matrix SceneObject_getWorldMatrix(SceneObject* object)
{
    SceneObject* parent = SceneGraph_getObject(object->graph, object->parent);
    if (parent != NULL) {
        // this is annoying and I don't know if this can be improved: while there's a cache of the world matrix including a version identifier
        // of the parent matrix, if the parent matrix is outdated and not queried, the children will not be updated
        SceneObject_getWorldMatrix(parent);
    }

    if (object->flags & SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY || (parent != NULL && parent->transform.worldMatrixVersion != object->parentWorldMatrixVersion)) {
        if (parent != NULL) {
            object->transform.worldMatrix = MatrixMultiply(SceneObject_getLocalMatrix(object), SceneObject_getWorldMatrix(parent));
        } else {
            object->transform.worldMatrix = SceneObject_getLocalMatrix(object);
        }
        object->flags &= ~SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY;
        object->transform.worldMatrixVersion++;
        object->parentWorldMatrixVersion = parent != NULL ? parent->transform.worldMatrixVersion : 0;
    }

    return object->transform.worldMatrix;
}

Vector3 SceneGraph_getWorldForward(SceneGraph* graph, SceneObjectId id)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return (Vector3) { 0, 0, 0 };
    }
    Matrix m = SceneObject_getWorldMatrix(object);
    return (Vector3) {
        m.m8,
        m.m9,
        m.m10,
    };
}

Vector3 SceneGraph_getWorldUp(SceneGraph* graph, SceneObjectId id)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return (Vector3) { 0, 0, 0 };
    }
    Matrix m = SceneObject_getWorldMatrix(object);
    return (Vector3) {
        m.m4,
        m.m5,
        m.m6,
    };
}

Vector3 SceneGraph_getWorldRight(SceneGraph* graph, SceneObjectId id)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return (Vector3) { 0, 0, 0 };
    }
    Matrix m = SceneObject_getWorldMatrix(object);
    return (Vector3) {
        m.m0,
        m.m1,
        m.m2,
    };
}

Vector3 SceneGraph_getWorldPosition(SceneGraph* graph, SceneObjectId id)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return (Vector3) { 0, 0, 0 };
    }
    Matrix m = SceneObject_getWorldMatrix(object);
    return (Vector3) {
        m.m12,
        m.m13,
        m.m14,
    };
}

Vector3 SceneGraph_getLocalPosition(SceneGraph* graph, SceneObjectId id)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return (Vector3) { 0, 0, 0 };
    }
    return object->transform.position;
}

Vector3 SceneGraph_getLocalRotation(SceneGraph* graph, SceneObjectId id)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return (Vector3) { 0, 0, 0 };
    }
    return object->transform.eulerRotationDegrees;
}

Vector3 SceneGraph_getLocalScale(SceneGraph* graph, SceneObjectId id)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return (Vector3) { 1, 1, 1 };
    }
    return object->transform.scale;
}

void SceneGraph_setLocalPosition(SceneGraph* graph, SceneObjectId id, Vector3 position)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return;
    }
    object->transform.position = position;
    object->flags |= SCENE_OBJECT_FLAG_LOCAL_MATRIX_DIRTY | SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY;
}

void SceneGraph_setLocalRotation(SceneGraph* graph, SceneObjectId id, Vector3 rotation)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return;
    }
    object->transform.eulerRotationDegrees = rotation;
    object->flags |= SCENE_OBJECT_FLAG_LOCAL_MATRIX_DIRTY | SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY;
}

void SceneGraph_setLocalScale(SceneGraph* graph, SceneObjectId id, Vector3 scale)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return;
    }
    object->transform.scale = scale;
    object->flags |= SCENE_OBJECT_FLAG_LOCAL_MATRIX_DIRTY | SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY;
}

void SceneGraph_setLocalTransform(SceneGraph* graph, SceneObjectId id, Vector3 position,
    Vector3 rotation, Vector3 scale)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return;
    }
    object->transform.position = position;
    object->transform.eulerRotationDegrees = rotation;
    object->transform.scale = scale;
    object->flags |= SCENE_OBJECT_FLAG_LOCAL_MATRIX_DIRTY | SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY;
}

void SceneGraph_setParent(SceneGraph* graph, SceneObjectId id, SceneObjectId parentId)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return;
    }

    SceneObject* oldParent = SceneGraph_getObject(graph, object->parent);
    if (oldParent != NULL) {
        for (int i = 0; i < oldParent->children_count; i++) {
            if (oldParent->children[i].id == id.id) {
                for (int j = i; j < oldParent->children_count - 1; j++) {
                    oldParent->children[j] = oldParent->children[j + 1];
                }
                oldParent->children_count--;
                break;
            }
        }
    }

    object->parent = parentId;
    object->flags |= SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY;
    SceneObject* parent = SceneGraph_getObject(graph, parentId);
    if (parent == NULL) {
        return;
    }

    SceneObjectId* childEntry = SceneObject_acquire_children(parent);
    *childEntry = id;
}

void SceneGraph_updateTick(SceneGraph* graph, float delta)
{
    for (int i = 0; i < graph->componentTypes_count; i++) {
        SceneComponentType type = graph->componentTypes[i];
        if (type.methods.updateTick == NULL) {
            continue;
        }

        for (int j = 0; j < type.components_count; j++) {
            SceneComponent* component = &type.components[j];
            if (component->id.version == 0 || (component->flags & SCENE_COMPONENT_FLAG_ENABLED) == 0) {
                continue;
            }

            SceneObject* object = SceneGraph_getObject(graph, component->objectId);
            if (object == NULL) {
                continue;
            }

            void* data = &type.componentData[j * type.dataSize];
            type.methods.updateTick(object, component->id, delta, data);
        }
    }
}

void SceneGraph_sequentialDraw(SceneGraph* graph, Camera3D camera, void* userdata)
{
    int marker = ++graph->markerCounter;
    int maxDepth = 0;
    // setting object markers to 0 should not be needed...
    // for (int i=0;i<graph->objects_count;i++)
    // {
    //     SceneObject* object = &graph->objects[i];
    //     if (object->id.version == 0 || (object->flags & SCENE_OBJECT_FLAG_ENABLED) == 0)
    //     {
    //         continue;
    //     }
    //     object->marker = 0;
    // }

    // Phase 1: Mark all objects (and parents) with components that have a sequentialDraw method
    for (int i = 0; i < graph->componentTypes_count; i++) {
        SceneComponentType type = graph->componentTypes[i];
        if (type.methods.sequentialDraw == NULL) {
            continue;
        }

        for (int j = 0; j < type.components_count; j++) {
            SceneComponent* component = &type.components[j];
            if (component->id.version == 0 || (component->flags & SCENE_COMPONENT_FLAG_ENABLED) == 0) {
                continue;
            }

            SceneObject* object = SceneGraph_getObject(graph, component->objectId);
            if (object == NULL || object->marker == marker || (object->flags & SCENE_OBJECT_FLAG_ENABLED) == 0) {
                continue;
            }

            SceneObjectId parentId = object->parent;
            int isActive = 1;
            while (parentId.version != 0) {
                SceneObject* parent = SceneGraph_getObject(graph, parentId);
                if (parent == NULL) {
                    break;
                }
                if (parent->marker == marker) {
                    break;
                }
                parentId = parent->parent;
                if ((parent->flags & SCENE_OBJECT_FLAG_ENABLED) == 0) {
                    isActive = 0;
                    break;
                }
            }

            if (!isActive) {
                continue;
            }

            object->marker = marker;
            int depth = 1;
            while (object->parent.version != 0) {
                object = SceneGraph_getObject(graph, object->parent);
                if (object == NULL) {
                    break;
                }
                object->marker = marker;
                depth++;
            }

            if (depth > maxDepth) {
                maxDepth = depth;
            }

            // void* data = &type.componentData[j * type.dataSize];
            // type.methods.sequentialDraw(camera, object, component->id, data, userdata);
        }

        // Phase 2: Draw all objects (and parents) with components that have a sequentialDraw method
        int iterationStack[maxDepth];
        SceneObject* objectStack[maxDepth];
        memset(iterationStack, 0, sizeof(iterationStack));
        for (int i = 0; i < graph->objects_count; i++) {
            // find a root object
            SceneObject* object = &graph->objects[i];
            if (object->id.version == 0 || object->marker != marker || object->parent.version != 0) {
                continue;
            }

            // iterate all children, recursion free
            int depth = 0;
            objectStack[depth] = object;
            iterationStack[depth] = 0;
            while (depth >= 0) {
                SceneObject* o = objectStack[depth];
                if (iterationStack[depth] == 0) {
                    for (int j = 0; j < o->components_count; j += 1) {
                        SceneComponent* component = SceneGraph_getComponent(graph, o->components[j], NULL);
                        if (component == NULL || (component->flags & SCENE_COMPONENT_FLAG_ENABLED) == 0) {
                            continue;
                        }
                        SceneComponentType* type = SceneGraph_getComponentType(graph, component->typeId);
                        if (type == NULL) {
                            continue;
                        }
                        if (type->methods.sequentialDraw == NULL) {
                            continue;
                        }
                        void* data = &type->componentData[component->id.id * type->dataSize];
                        type->methods.sequentialDraw(camera, o, component->id, data, userdata);
                    }
                }

                if (iterationStack[depth] >= o->children_count) {
                    depth--;
                    continue;
                }

                SceneObject* child = SceneGraph_getObject(graph, o->children[iterationStack[depth]++]);
                if (child == NULL) {
                    continue;
                }

                if (child->marker == marker) {
                    depth++;
                    objectStack[depth] = child;
                    iterationStack[depth] = 0;
                    continue;
                }
            }
        }
    }
}

void SceneGraph_draw2D(SceneGraph* graph, Camera2D camera, void* userdata)
{
    for (int i = 0; i < graph->componentTypes_count; i++) {
        SceneComponentType type = graph->componentTypes[i];
        if (type.methods.draw2D == NULL) {
            continue;
        }
        for (int j = 0; j < type.components_count; j++) {
            SceneComponent* component = &type.components[j];
            if (component->id.version == 0 || (component->flags & SCENE_COMPONENT_FLAG_ENABLED) == 0) {
                continue;
            }

            SceneObject* object = SceneGraph_getObject(graph, component->objectId);
            if (object == NULL) {
                continue;
            }

            void* data = &type.componentData[j * type.dataSize];
            type.methods.draw2D(camera, object, component->id, data, userdata);
        }
    }
}
void SceneGraph_draw(SceneGraph* graph, Camera3D camera, void* userdata)
{
    for (int i = 0; i < graph->componentTypes_count; i++) {
        SceneComponentType type = graph->componentTypes[i];
        if (type.methods.draw == NULL) {
            continue;
        }
        for (int j = 0; j < type.components_count; j++) {
            SceneComponent* component = &type.components[j];
            if (component->id.version == 0 || (component->flags & SCENE_COMPONENT_FLAG_ENABLED) == 0) {
                continue;
            }

            SceneObject* object = SceneGraph_getObject(graph, component->objectId);
            if (object == NULL) {
                continue;
            }

            void* data = &type.componentData[j * type.dataSize];
            type.methods.draw(camera, object, component->id, data, userdata);
        }
    }
}

Vector3 SceneGraph_localToWorld(SceneGraph* graph, SceneObjectId id, Vector3 local)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return local;
    }
    return Vector3Transform(local, SceneObject_getWorldMatrix(object));
}

SceneObjectId SceneGraph_createObject(SceneGraph* graph, const char* name)
{
    SceneObject* object = NULL;
    for (int i = 0; i < graph->objects_count; i++) {
        object = &graph->objects[i];
        if (object->id.version == 0) {
            break;
        }
        object = NULL;
    }
    if (object == NULL) {
        object = SceneGraph_acquire_objects(graph);
        object->id.id = graph->objects_count - 1;
        object->graph = graph;
    }

    object->id.version = ++graph->versionCounter;
    object->name = strdup(name);
    object->flags = SCENE_OBJECT_FLAG_ENABLED | SCENE_OBJECT_FLAG_LOCAL_MATRIX_DIRTY | SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY;
    object->parent = (SceneObjectId) { 0 };
    object->transform.position = (Vector3) { 0, 0, 0 };
    object->transform.eulerRotationDegrees = (Vector3) { 0, 0, 0 };
    object->transform.scale = (Vector3) { 1, 1, 1 };
    object->transform.localMatrix = MatrixIdentity();
    object->transform.worldMatrix = MatrixIdentity();

    return object->id;
}

SceneObject* SceneGraph_getObject(SceneGraph* graph, SceneObjectId id)
{
    if (id.id < 0 || id.id >= graph->objects_count || id.version == 0) {
        return NULL;
    }
    SceneObject* object = &graph->objects[id.id];
    if (object->id.version != id.version) {
        return NULL;
    }
    return object;
}

const char* SceneGraph_getObjectName(SceneGraph* graph, SceneObjectId id)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return NULL;
    }
    return object->name;
}

void SceneGraph_destroyObject(SceneGraph* graph, SceneObjectId id)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return;
    }

    for (int i = 0; i < object->components_count; i++) {
        SceneGraph_destroyComponent(graph, object->components[i]);
    }

    for (int i = 0; i < object->children_count; i++) {
        SceneGraph_destroyObject(graph, object->children[i]);
    }

    free(object->children);
    object->children = NULL;

    free(object->components);
    object->components = NULL;

    free(object->name);
    object->name = NULL;
    object->id.version = 0;
    object->children_count = 0;
    object->children_capacity = 0;
    object->components_count = 0;
    object->components_capacity = 0;
}

SceneComponentId SceneGraph_addComponent(SceneGraph* graph, SceneObjectId id, SceneComponentTypeId componentType,
    void* componentData)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return (SceneComponentId) { 0 };
    }

    SceneComponentType* type = SceneGraph_getComponentType(graph, componentType);
    if (type == NULL) {
        TraceLog(LOG_ERROR, "SceneGraph_addComponent: component type not found");
        return (SceneComponentId) { 0 };
    }

    SceneComponent* component = NULL;
    for (int i = 0; i < type->components_count; i++) {
        component = &type->components[i];
        if (component->id.version == 0) {
            break;
        }
        component = NULL;
    }
    if (component == NULL) {
        component = SceneComponentType_acquire_components(type);
        component->id.id = type->components_count - 1;
    }
    component->id.version = ++graph->versionCounter;
    component->id.typeId = componentType;
    component->objectId = id;
    component->typeId = componentType;
    component->flags = SCENE_COMPONENT_FLAG_ENABLED;
    component->name = NULL;

    int dataIndex = component->id.id;
    if (dataIndex >= type->componentData_capacity && type->dataSize > 0) {
        type->componentData_capacity = type->componentData_capacity == 0 ? 1 : type->componentData_capacity * 2;
        type->componentData = realloc(type->componentData, type->componentData_capacity * type->dataSize);
    }

    if (type->methods.onInitialize != NULL) {
        type->methods.onInitialize(object, component->id, &type->componentData[dataIndex * type->dataSize], componentData);
        object = SceneGraph_getObject(graph, id);
        if (object == NULL) {
            component->id.version = 0;
            return (SceneComponentId) { 0 };
        }
    } else if (componentData != NULL && type->dataSize > 0)
        memcpy(&type->componentData[dataIndex * type->dataSize], componentData, type->dataSize);
    else if (type->dataSize > 0)
        memset(&type->componentData[dataIndex * type->dataSize], 0, type->dataSize);

    for (int i = 0; i < object->components_count; i++) {
        if (object->components[i].version == 0) {
            object->components[i] = component->id;
            return component->id;
        }
    }

    SceneComponentId* componentIdRef = SceneObject_acquire_components(object);
    *componentIdRef = component->id;

    return component->id;
}

SceneComponentType* SceneGraph_getComponentType(SceneGraph* graph, SceneComponentTypeId componentType)
{
    if (componentType.id < 0 || componentType.id >= graph->componentTypes_count) {
        return NULL;
    }
    SceneComponentType* type = &graph->componentTypes[componentType.id];
    if (type->id.version != componentType.version) {
        return NULL;
    }
    return type;
}

SceneComponentTypeId SceneGraph_getComponentTypeId(SceneGraph* graph, const char* name)
{
    for (int i = 0; i < graph->componentTypes_count; i++) {
        SceneComponentType* type = &graph->componentTypes[i];
        if (type->id.version == 0) {
            continue;
        }
        if (strcmp(type->name, name) == 0) {
            return type->id;
        }
    }
    return (SceneComponentTypeId) { 0 };
}

SceneComponent* SceneGraph_getComponent(SceneGraph* graph, SceneComponentId id, void** componentData)
{
    SceneComponentTypeId typeId = id.typeId;
    SceneComponentType* type = SceneGraph_getComponentType(graph, typeId);
    if (type == NULL) {
        if (componentData != NULL)
            *componentData = NULL;
        return NULL;
    }
    SceneComponent* component = &type->components[id.id];
    if (component->id.version != id.version) {
        if (componentData != NULL)
            componentData = NULL;
        return NULL;
    }
    if (componentData != NULL)
        *componentData = &type->componentData[id.id * type->dataSize];
    return component;
}

SceneComponent* SceneGraph_getComponentByType(SceneGraph* graph, SceneObjectId id, SceneComponentTypeId typeId, void** componentData, int atIndex)
{
    if (id.version == 0) {
        SceneComponentType* type = SceneGraph_getComponentType(graph, typeId);
        if (type == NULL) {
            if (componentData != NULL)
                *componentData = NULL;
            return NULL;
        }
        for (int i = 0; i < type->components_count; i++) {
            SceneComponent* component = &type->components[i];
            if (component->id.version != 0 && --atIndex < 0) {
                if (componentData != NULL)
                    *componentData = &type->componentData[i * type->dataSize];
                return component;
            }
        }
    }

    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        if (componentData != NULL)
            *componentData = NULL;
        return NULL;
    }

    for (int i = 0; i < object->components_count; i++) {
        SceneComponent* component = SceneGraph_getComponent(graph, object->components[i], componentData);
        if (component == NULL) {
            continue;
        }
        if (component->typeId.id == typeId.id && component->typeId.version == typeId.version && --atIndex < 0) {
            return component;
        }
    }

    if (componentData != NULL)
        *componentData = NULL;

    return NULL;
}

SceneComponent* SceneGraph_getComponentOrFindByType(SceneGraph* graph, SceneObjectId id, SceneComponentId* componentId, SceneComponentTypeId typeId, void** componentData)
{
    SceneComponent* result;

    if (componentId != NULL) {
        result = SceneGraph_getComponent(graph, *componentId, componentData);
        if (result != NULL) {
            return result;
        }
    }

    result = SceneGraph_getComponentByType(graph, id, typeId, componentData, 0);
    if (result != NULL && componentId != NULL) {
        *componentId = result->id;
    }

    return result;
}

SceneObject* SceneGraph_findObjectByName(SceneGraph* graph, const char* name, int includeDisabled)
{
    for (int i = 0; i < graph->objects_count; i++) {
        SceneObject* object = &graph->objects[i];
        if (object->id.version == 0 || ((object->flags & SCENE_OBJECT_FLAG_ENABLED) == 0 && !includeDisabled)) {
            continue;
        }
        SceneObject* parent = SceneGraph_getObject(graph, object->parent);
        while (parent) {
            if ((parent->flags & SCENE_OBJECT_FLAG_ENABLED) == 0) {
                break;
            }
            parent = SceneGraph_getObject(graph, parent->parent);
        }

        if (parent == NULL && object->name && strcmp(object->name, name) == 0) {
            return object;
        }
    }
    return NULL;
}

SceneComponent* SceneGraph_findComponentByName(SceneGraph* graph, const char* name, int includeDisabled)
{
    for (int i = 0; i < graph->componentTypes_count; i++) {
        SceneComponentType* type = &graph->componentTypes[i];
        if (type->id.version == 0) {
            continue;
        }
        for (int j = 0; j < type->components_count; j++) {
            SceneComponent* component = &type->components[j];
            if (component->id.version == 0 || ((component->flags & SCENE_COMPONENT_FLAG_ENABLED) == 0 && !includeDisabled)) {
                continue;
            }
            if (!includeDisabled) {
                SceneObject* object = SceneGraph_getObject(graph, component->objectId);
                if (object == NULL) {
                    continue;
                }
                while (object) {
                    if ((object->flags & SCENE_OBJECT_FLAG_ENABLED) == 0) {
                        break;
                    }
                    object = SceneGraph_getObject(graph, object->parent);
                }
                if (object) {
                    continue;
                }
            }
            if (component->name && strcmp(component->name, name) == 0) {
                return component;
            }
        }
    }
    return NULL;
}

SceneObject* SceneGraph_findChildByName(SceneGraph* graph, SceneObjectId parentId, const char* name, int includeDisabled)
{
    SceneObject* object = SceneGraph_getObject(graph, parentId);
    if (object == NULL || ((object->flags & SCENE_OBJECT_FLAG_ENABLED) == 0 && !includeDisabled)) {
        return NULL;
    }

    if (strcmp(object->name, name) == 0) {
        return object;
    }

    for (int i = 0; i < object->children_count; i++) {
        SceneObject* child = SceneGraph_findChildByName(graph, object->children[i], name, includeDisabled);
        if (child != NULL) {
            return child;
        }
    }

    return NULL;
}
SceneComponent* SceneGraph_findChildComponentByName(SceneGraph* graph, SceneObjectId parentId, const char* name, int includeDisabled)
{
    SceneObject* object = SceneGraph_getObject(graph, parentId);
    if (object == NULL || ((object->flags & SCENE_OBJECT_FLAG_ENABLED) == 0 && !includeDisabled)) {
        return NULL;
    }

    for (int i = 0; i < object->components_count; i++) {
        SceneComponent* component = SceneGraph_getComponent(graph, object->components[i], NULL);
        if (component == NULL || ((component->flags & SCENE_COMPONENT_FLAG_ENABLED) == 0 && !includeDisabled)) {
            continue;
        }
        if (component->name && strcmp(component->name, name) == 0) {
            return component;
        }
    }

    for (int i = 0; i < object->children_count; i++) {
        SceneComponent* component = SceneGraph_findChildComponentByName(graph, object->children[i], name, includeDisabled);
        if (component != NULL) {
            return component;
        }
    }

    return NULL;
}

void SceneGraph_destroyComponent(SceneGraph* graph, SceneComponentId id)
{
    SceneComponent* component = SceneGraph_getComponent(graph, id, NULL);
    if (component == NULL) {
        return;
    }
    SceneComponentType* type = SceneGraph_getComponentType(graph, id.typeId);
    if (type != NULL && type->methods.onDestroy != NULL) {
        void* componentData = type->dataSize > 0 ? &type->componentData[id.id * type->dataSize] : NULL;
        type->methods.onDestroy(SceneGraph_getObject(graph, component->objectId), id, componentData);
    }
    component->id.version = 0;
    if (component->name) {
        free(component->name);
        component->name = NULL;
    }
}

int SceneGraph_setComponentName(SceneGraph* graph, SceneComponentId id, const char* name)
{
    SceneComponent* component = SceneGraph_getComponent(graph, id, NULL);
    if (component == NULL) {
        return 0;
    }
    if (component->name) {
        free(component->name);
    }
    component->name = strdup(name);
    return 1;
}

void SceneGraph_printObject(SceneObject* object, const char* indent, int printComponents)
{
    int indentLength = strlen(indent);
    char newIndent[indentLength + 3];
    for (int i = 0; i < indentLength + 2; i++) {
        newIndent[i] = ' ';
    }
    newIndent[indentLength + 2] = '\0';

    Vector3 position = object->transform.position;
    Vector3 rotation = object->transform.eulerRotationDegrees;
    Vector3 worldPos = SceneGraph_getWorldPosition(object->graph, object->id);

    printf("%sObject %3d [%s]: %s (%.2f, %.2f, %.2f) @ (%.2f, %.2f, %.2f) | (%.2f, %.2f, %.2f)\n", indent, object->id.id,
        object->flags & SCENE_OBJECT_FLAG_ENABLED ? "E" : " ",
        object->name,
        position.x, position.y, position.z,
        rotation.x, rotation.y, rotation.z,
        worldPos.x, worldPos.y, worldPos.z);
    if (printComponents) {
        for (int i = 0; i < object->components_count; i++) {
            SceneComponent* component = SceneGraph_getComponent(object->graph, object->components[i], NULL);
            if (component == NULL) {
                printf("%s+Invalid component %3d\n", newIndent, object->components[i].id);
                continue;
            }
            SceneComponentType* type = SceneGraph_getComponentType(object->graph, component->typeId);
            if (type == NULL) {
                printf("%s+Invalid type %3d\n", newIndent, component->typeId.id);
                continue;
            }
            printf("%s+Component %3d [%s]: %s\n", newIndent, component->id.id,
                component->flags & SCENE_COMPONENT_FLAG_ENABLED ? "E" : " ",
                type->name);
        }
    }

    for (int i = 0; i < object->children_count; i++) {
        SceneObject* child = SceneGraph_getObject(object->graph, object->children[i]);
        if (child->parent.id != object->id.id || child->parent.version != object->id.version) {
            printf("%sInvalid child %3d: %s\n", newIndent, object->children[i].id, child->name);
            continue;
        }
        if (child == NULL) {
            continue;
        }
        SceneGraph_printObject(child, newIndent, printComponents);
    }
}

void SceneGraph_print(SceneGraph* graph, int printComponents, int maxRootCount)
{
    printf("SceneGraph: %d objects\n", graph->objects_count);
    if (maxRootCount == 0) {
        maxRootCount = graph->objects_count;
    }
    for (int i = 0; i < graph->objects_count; i++) {
        SceneObject* object = &graph->objects[i];
        if (object->id.version == 0 || SceneGraph_getObject(graph, object->parent) != NULL) {
            continue;
        }

        SceneGraph_printObject(object, "", printComponents);
        if (maxRootCount-- <= 0) {
            break;
        }
    }
}

void SceneGraph_setComponentEnabled(SceneGraph* graph, SceneComponentId id, int enabled)
{
    SceneComponent* component = SceneGraph_getComponent(graph, id, NULL);
    if (component == NULL) {
        return;
    }
    if (enabled) {
        component->flags |= SCENE_COMPONENT_FLAG_ENABLED;
    } else {
        component->flags &= ~SCENE_COMPONENT_FLAG_ENABLED;
    }
}

int SceneGraph_isComponentEnabled(SceneGraph* graph, SceneComponentId id)
{
    SceneComponent* component = SceneGraph_getComponent(graph, id, NULL);
    if (component == NULL) {
        return 0;
    }
    return (component->flags & SCENE_COMPONENT_FLAG_ENABLED) != 0;
}

void SceneGraph_setObjectEnabled(SceneGraph* graph, SceneObjectId id, int enabled)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return;
    }
    if (enabled) {
        object->flags |= SCENE_OBJECT_FLAG_ENABLED;
    } else {
        object->flags &= ~SCENE_OBJECT_FLAG_ENABLED;
    }
}
int SceneGraph_isObjectEnabled(SceneGraph* graph, SceneObjectId id)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return 0;
    }
    return (object->flags & SCENE_OBJECT_FLAG_ENABLED) != 0;
}

int SceneGraph_getComponentValue(SceneGraph* graph, char* name, SceneComponentId componentId, int bufferSize, void* buffer)
{
    void* componentData;
    SceneComponent* component = SceneGraph_getComponent(graph, componentId, &componentData);
    if (component == NULL) {
        return 0;
    }
    SceneComponentType* type = SceneGraph_getComponentType(graph, component->typeId);
    if (type == NULL) {
        return 0;
    }
    if (type->methods.getValue == NULL) {
        return 0;
    }

    SceneObject* object = SceneGraph_getObject(graph, component->objectId);
    if (object == NULL) {
        return 0;
    }

    if (strcmp(name, "enabled") == 0) {
        if (bufferSize != sizeof(int)) {
            return 0;
        }

        int enabled = (component->flags & SCENE_COMPONENT_FLAG_ENABLED) != 0;
        memcpy(buffer, &enabled, sizeof(int));
        return 1;
    }

    return type->methods.getValue(object, component, componentData, name, bufferSize, buffer);
}

int SceneGraph_setComponentValue(SceneGraph* graph, char* name, SceneComponentId componentId, int bufferSize, void* buffer)
{
    void* componentData;
    SceneComponent* component = SceneGraph_getComponent(graph, componentId, &componentData);
    if (component == NULL) {
        return 0;
    }
    SceneComponentType* type = SceneGraph_getComponentType(graph, component->typeId);
    if (type == NULL) {
        return 0;
    }
    if (type->methods.setValue == NULL) {
        return 0;
    }

    SceneObject* object = SceneGraph_getObject(graph, component->objectId);
    if (object == NULL) {
        return 0;
    }

    if (strcmp(name, "enabled") == 0) {
        if (bufferSize != sizeof(int)) {
            return 0;
        }

        int enabled;
        memcpy(&enabled, buffer, sizeof(int));
        SceneGraph_setComponentEnabled(graph, componentId, enabled);
        return 1;
    }

    return type->methods.setValue(object, component, componentData, name, bufferSize, buffer);
}

int SceneComponentIdEquals(SceneComponentId a, SceneComponentId b)
{
    return memcmp(&a, &b, sizeof(a)) == 0;
}

#include "external/cJSON.h"

void SceneComponentType_onSerialized(const char* key, SceneComponentType* componentType, cJSON* json, void* userData)
{
}