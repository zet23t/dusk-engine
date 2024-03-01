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
    free(graph);
}

STRUCT_LIST_ACQUIRE_FN(SceneGraph, SceneObject, objects)
STRUCT_LIST_ACQUIRE_FN(SceneGraph, SceneComponentType, componentTypes)
STRUCT_LIST_ACQUIRE_FN(SceneComponentType, SceneComponent, components)
STRUCT_LIST_ACQUIRE_FN(SceneObject, SceneComponentId, components)
STRUCT_LIST_ACQUIRE_FN(SceneObject, SceneObjectId, children)

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
    object->flags = SCENE_OBJECT_FLAG_ENABLED;
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
        return (SceneComponentId) { 0 };
    }

    SceneComponentType* type = SceneGraph_getComponentType(graph, componentType);
    if (type == NULL) {
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

    int dataIndex = component->id.id;
    if (dataIndex >= type->componentData_capacity && type->dataSize > 0) {
        type->componentData_capacity = type->componentData_capacity == 0 ? 1 : type->componentData_capacity * 2;
        type->componentData = realloc(type->componentData, type->componentData_capacity * type->dataSize);
    }

    if (type->methods.initialize != NULL)
        type->methods.initialize(object, component->id, &type->componentData[dataIndex * type->dataSize], componentData);
    else if (componentData != NULL && type->dataSize > 0)
        memcpy(&type->componentData[dataIndex * type->dataSize], componentData, type->dataSize);

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
    SceneComponent* result = SceneGraph_getComponent(graph, *componentId, componentData);
    if (result != NULL) {
        return result;
    }

    result = SceneGraph_getComponentByType(graph, id, typeId, componentData, 0);
    if (result != NULL) {
        *componentId = result->id;
    }

    return result;
}

void SceneGraph_destroyComponent(SceneGraph* graph, SceneComponentId id)
{
    SceneComponent* component = SceneGraph_getComponent(graph, id, NULL);
    if (component == NULL) {
        return;
    }
    component->id.version = 0;
}

void SceneGraph_printObject(SceneObject* object, const char* indent)
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

    printf("%sObject %3d: %s (%.2f, %.2f, %.2f) @ (%.2f, %.2f, %.2f) | (%.2f, %.2f, %.2f)\n", indent, object->id.id, object->name,
        position.x, position.y, position.z,
        rotation.x, rotation.y, rotation.z,
        worldPos.x, worldPos.y, worldPos.z);

    for (int i = 0; i < object->children_count; i++) {
        SceneObject* child = SceneGraph_getObject(object->graph, object->children[i]);
        if (child->parent.id != object->id.id || child->parent.version != object->id.version) {
            printf("%sInvalid child %3d: %s\n", newIndent, object->children[i].id, child->name);
            continue;
        }
        if (child == NULL) {
            continue;
        }
        SceneGraph_printObject(child, newIndent);
    }
}

void SceneGraph_print(SceneGraph* graph)
{
    printf("SceneGraph: %d objects\n", graph->objects_count);
    for (int i = 0; i < graph->objects_count; i++) {
        SceneObject* object = &graph->objects[i];
        if (object->id.version == 0 || SceneGraph_getObject(graph, object->parent) != NULL) {
            continue;
        }

        SceneGraph_printObject(object, "");
    }
}