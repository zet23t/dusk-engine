#include "scene_graph.h"

#include <stdlib.h>
#include <string.h>
#include <raymath.h>

SceneGraph *SceneGraph_create()
{
    SceneGraph *graph = malloc(sizeof(SceneGraph));
    memset(graph, 0, sizeof(SceneGraph));
    return graph;
}

void SceneGraph_destroy(SceneGraph *graph)
{
    for (int i = 0; i < graph->objects_count; i++)
    {
        SceneGraph_destroyObject(graph, graph->objects[i].id);
    }
    free(graph);
}

void SceneGraph_updateTick(SceneGraph *graph, float delta)
{
    for (int i = 0; i < graph->objects_count; i++)
    {
        SceneObject *object = &graph->objects[i];
        if (object->id.version == 0)
        {
            continue;
        }
        if (object->flags & SCENE_OBJECT_FLAG_LOCAL_MATRIX_DIRTY)
        {
            object->transform.localMatrix = MatrixIdentity();
            object->transform.localMatrix = MatrixMultiply(object->transform.localMatrix, MatrixScale(object->transform.scale.x, object->transform.scale.y, object->transform.scale.z));
            object->transform.localMatrix = MatrixMultiply(object->transform.localMatrix, MatrixRotateXYZ((Vector3){DEG2RAD * object->transform.eulerRotationDegrees.x, DEG2RAD * object->transform.eulerRotationDegrees.y, DEG2RAD * object->transform.eulerRotationDegrees.z}));
            object->transform.localMatrix = MatrixMultiply(object->transform.localMatrix, MatrixTranslate(object->transform.position.x, object->transform.position.y, object->transform.position.z));
            object->flags &= ~SCENE_OBJECT_FLAG_LOCAL_MATRIX_DIRTY;
        }
    }
}

STRUCT_LIST_ACQUIRE_FN(SceneGraph, SceneObject, objects)

SceneObjectId SceneGraph_createObject(SceneGraph *graph, const char *name)
{
    SceneObject *object;
    for (int i=0;i<graph->objects_count;i++)
    {
        object = &graph->objects[i];
        if (object->id.version == 0)
        {
            break;
        }
        object = NULL;
    }
    if (object == NULL)
    {
        object = SceneGraph_acquire_objects(graph);
        object->id.id = graph->objects_count - 1;
    }

    object->id.version = ++graph->versionCounter;
    object->name = strdup(name);
    object->flags = SCENE_OBJECT_FLAG_ENABLED;
    object->transform.scale = (Vector3){1, 1, 1};
    object->transform.localMatrix = MatrixIdentity();
    object->transform.worldMatrix = MatrixIdentity();

    return object->id;
}

SceneObject *SceneGraph_getObject(SceneGraph *graph, SceneObjectId id)
{
    if (id.id < 0 || id.id >= graph->objects_count)
    {
        return NULL;
    }
    SceneObject *object = &graph->objects[id.id];
    if (object->id.version != id.version)
    {
        return NULL;
    }
    return object;
}

void SceneGraph_destroyObject(SceneGraph *graph, SceneObjectId id)
{
    SceneObject *object = SceneGraph_getObject(graph, id);
    if (object == NULL)
    {
        return;
    }

    for (int i=0;i<object->children_count;i++)
    {
        SceneGraph_destroyObject(graph, object->children[i]);
    }

    for (int i=0;i<object->components_count;i++)
    {
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

STRUCT_LIST_ACQUIRE_FN(SceneGraph, SceneComponent, components)

SceneComponentId SceneGraph_createComponent(SceneGraph *graph, SceneObjectId object)
{
    SceneObject *sceneObject = SceneGraph_getObject(graph, object);
    if (sceneObject == NULL)
    {
        return (SceneComponentId){0, 0};
    }

    SceneComponent *component = SceneGraph_acquire_components(graph);
    component->object = object;
    return component->id;
}

SceneComponent *SceneGraph_getComponent(SceneGraph *graph, SceneComponentId id)
{
    if (id.id < 0 || id.id >= graph->components_count)
    {
        return NULL;
    }
    SceneComponent *component = &graph->components[id.id];
    if (component->id.version != id.version)
    {
        return NULL;
    }
    return component;
}

void SceneGraph_destroyComponent(SceneGraph *graph, SceneComponentId id)
{
    SceneComponent *component = SceneGraph_getComponent(graph, id);
    if (component == NULL)
    {
        return;
    }
    component->id.version = 0;
}