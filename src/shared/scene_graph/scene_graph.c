#include "scene_graph.h"

#include <raymath.h>
#include <stdlib.h>
#include <string.h>

SceneObject* SceneGraph_getObject(SceneGraph* graph, SceneObjectId id);

SceneGraph* SceneGraph_create()
{
    SceneGraph* graph = malloc(sizeof(SceneGraph));
    memset(graph, 0, sizeof(SceneGraph));
    return graph;
}

void SceneGraph_destroy(SceneGraph* graph)
{
    for (int i = 0; i < graph->objects_count; i++) {
        SceneGraph_destroyObject(graph, graph->objects[i].id);
    }
    free(graph);
}

STRUCT_LIST_ACQUIRE_FN(SceneGraph, SceneObject, objects)
STRUCT_LIST_ACQUIRE_FN(SceneGraph, SceneComponentType, componentTypes)
STRUCT_LIST_ACQUIRE_FN(SceneComponentType, SceneComponent, components)
STRUCT_LIST_ACQUIRE_FN(SceneObject, SceneComponentId, components)

SceneComponentTypeId SceneGraph_registerComponentType(SceneGraph* graph, const char* name,
    size_t dataSize, SceneComponentTypeMethods methods)
{
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
    }

    return object->transform.localMatrix;
}

Matrix SceneObject_getWorldMatrix(SceneObject* object)
{
    if (object->flags & SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY) {
        SceneObject* parent = SceneGraph_getObject(object->graph, object->parent);
        if (parent != NULL) {
            object->transform.worldMatrix = MatrixMultiply(SceneObject_getWorldMatrix(parent), SceneObject_getLocalMatrix(object));
        } else {
            object->transform.worldMatrix = SceneObject_getLocalMatrix(object);
        }
        object->flags &= ~SCENE_OBJECT_FLAG_WORLD_MATRIX_DIRTY;
    }

    return object->transform.worldMatrix;
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

void SceneGraph_updateTick(SceneGraph* graph, float delta)
{
    for (int i = 0; i < graph->componentTypes_count; i++) {
        SceneComponentType type = graph->componentTypes[i];
        if (type.methods.updateTick == NULL) {
            continue;
        }

        for (int j = 0; j < type.components_count; j++) {
            SceneComponent* component = &type.components[j];
            if (component->id.version == 0) {
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

void SceneGraph_draw(SceneGraph* graph, Camera3D camera, void* userdata)
{
    for (int i = 0; i < graph->componentTypes_count; i++) {
        SceneComponentType type = graph->componentTypes[i];
        if (type.methods.draw == NULL) {
            continue;
        }
        for (int j = 0; j < type.components_count; j++) {
            SceneComponent* component = &type.components[j];
            if (component->id.version == 0) {
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
    object->flags = SCENE_OBJECT_FLAG_ENABLED;
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

void SceneGraph_destroyObject(SceneGraph* graph, SceneObjectId id)
{
    SceneObject* object = SceneGraph_getObject(graph, id);
    if (object == NULL) {
        return;
    }

    for (int i = 0; i < object->children_count; i++) {
        SceneGraph_destroyObject(graph, object->children[i]);
    }

    for (int i = 0; i < object->components_count; i++) {
        SceneGraph_destroyComponent(graph, object->components[i]);
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
        return (SceneComponentId) { 0, 0 };
    }

    SceneComponentType* type = SceneGraph_getComponentType(graph, componentType);
    if (type == NULL) {
        return (SceneComponentId) { 0, 0 };
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
    component->objectId = id;
    component->typeId = componentType;

    if (type->componentData_count >= type->componentData_capacity) {
        type->componentData_capacity = type->componentData_capacity == 0 ? 1 : type->componentData_capacity * 2;
        type->componentData = realloc(type->componentData, type->componentData_capacity * type->dataSize);
    }

    if (componentData != NULL)
        memcpy(&type->componentData[type->componentData_count * type->dataSize], componentData, type->dataSize);
    type->componentData_count++;

    for (int i = 0; i < object->components_count; i++) {
        if (object->components[i].version == 0) {
            object->components[i] = component->id;
            return component->id;
        }
    }

    SceneComponentId *componentIdRef = SceneObject_acquire_components(object);
    *componentIdRef = component->id;

    return component->id;
}

SceneComponent* SceneGraph_getComponent(SceneGraph* graph, SceneComponentId id)
{
    if (id.id < 0 || id.id >= graph->components_count) {
        return NULL;
    }
    SceneComponent* component = &graph->components[id.id];
    if (component->id.version != id.version) {
        return NULL;
    }
    return component;
}

void SceneGraph_destroyComponent(SceneGraph* graph, SceneComponentId id)
{
    SceneComponent* component = SceneGraph_getComponent(graph, id);
    if (component == NULL) {
        return;
    }
    component->id.version = 0;
}