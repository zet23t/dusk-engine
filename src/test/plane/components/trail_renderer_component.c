#include "../plane_sim_g.h"
#include <memory.h>
#include <string.h>
void TrailRendererComponent_onInitialize(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void* initArg)
{
    TrailRendererComponent* trailRendererComponent = (TrailRendererComponent*)componentData;
    trailRendererComponent->lastPosition = SceneGraph_getWorldPosition(sceneObject->graph, sceneObject->id);

    ComponentInitializer* initializer = (ComponentInitializer*)initArg;
    if (initializer->memData != NULL) {
        memcpy(trailRendererComponent, initializer->memData, sizeof(TrailRendererComponent));
    } else {
        memset(trailRendererComponent, 0, sizeof(TrailRendererComponent));
        trailRendererComponent->emitterRate = 10.0f;
        trailRendererComponent->emitterVelocity = (Vector3) { 0, 0, 0 };
        trailRendererComponent->maxLifeTime = 1.0f;
        cJSON* cfg = initializer->config;
        MappedVariable mappedVariables[] = {
            { .name = "maxLifeTime", .type = VALUE_TYPE_FLOAT, .floatValue = &trailRendererComponent->maxLifeTime },
            { .name = "emitterRate", .type = VALUE_TYPE_FLOAT, .floatValue = &trailRendererComponent->emitterRate },
            { .name = "emitterVelocity", .type = VALUE_TYPE_VEC3, .vec3Value = &trailRendererComponent->emitterVelocity },
            { 0 }
        };
        ReadMappedVariables(cfg, mappedVariables);
        cJSON* widths = cJSON_GetObjectItemCaseSensitive(cfg, "trailWidths");
        if (cJSON_IsArray(widths)) {
            int n = cJSON_GetArraySize(widths);
            if (n % 2 != 0) {
                TraceLog(LOG_ERROR, "trailWidths must have an even number of elements");
                return;
            }
            trailRendererComponent->trailWidthCount = n / 2;
            trailRendererComponent->trailWidths = (TrailWidthStep*)malloc(trailRendererComponent->trailWidthCount * sizeof(TrailWidthStep));
            for (int i = 0; i < trailRendererComponent->trailWidthCount; i += 2) {
                cJSON* percent = cJSON_GetArrayItem(widths, i);
                cJSON* width = cJSON_GetArrayItem(widths, i + 1);
                if (cJSON_IsNumber(width) && cJSON_IsNumber(percent)) {
                    trailRendererComponent->trailWidths[i].width = (float)width->valuedouble;
                    trailRendererComponent->trailWidths[i].percent = (float)percent->valuedouble;
                } else {
                    TraceLog(LOG_ERROR, "element at %d or %d aren't numbers", i, i + 1);
                    free(trailRendererComponent->trailWidths);
                    trailRendererComponent->trailWidths = NULL;
                    trailRendererComponent->trailWidthCount = 0;
                    return;
                }
            }
            for (int i = 0; i < trailRendererComponent->trailWidthCount; i++) {
                float percent = trailRendererComponent->trailWidths[i].percent;
                if (percent < 0 || percent > 1) {
                    TraceLog(LOG_WARNING, "percent at index %d is not between 0 and 1 but is %f", i, percent);
                }
            }

            for (int i = 1; i < trailRendererComponent->trailWidthCount; i++) {
                float a = trailRendererComponent->trailWidths[i].percent;
                float b = trailRendererComponent->trailWidths[i - 1].percent;
                if (a < b) {
                    TraceLog(LOG_WARNING, "index %d percent (%f) less than at index %d (%f)", i, a, i - 1, b);
                }
            }
        }
    }
}

void TrailRendererComponent_onDestroy(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData)
{
    TrailRendererComponent* trailRendererComponent = (TrailRendererComponent*)componentData;
    if (trailRendererComponent->nodes) {
        free(trailRendererComponent->nodes);
    }
}

static void TrailRendererComponent_newNode(TrailRendererComponent* trailRendererComponent, Vector3 spawnPoint, Vector3 velocity)
{
    if (trailRendererComponent->nodeCount == trailRendererComponent->nodeCapacity) {
        trailRendererComponent->nodeCapacity = trailRendererComponent->nodeCapacity == 0
            ? 4
            : trailRendererComponent->nodeCapacity * 2;
        trailRendererComponent->nodes = realloc(
            trailRendererComponent->nodes, trailRendererComponent->nodeCapacity * sizeof(TrailNode));
    }
    memmove(trailRendererComponent->nodes + 1, trailRendererComponent->nodes, trailRendererComponent->nodeCount * sizeof(TrailNode));
    TrailNode* node = &trailRendererComponent->nodes[0];
    node->position = spawnPoint;
    node->velocity = velocity;
    node->time = 0;

    trailRendererComponent->nodeCount++;
}

static void TrailRendererComponent_setWidth(Vector3 vup, Vector3 prev, Vector3 next, float width, float* vertices)
{
    Vector3 dir = Vector3Normalize(Vector3Subtract(next, prev));
    Vector3 right = Vector3CrossProduct(dir, vup);
    Vector3 up = Vector3CrossProduct(right, dir);
    Vector3 scale = Vector3Scale(up, width);
    *((Vector3*)&vertices[0]) = Vector3Add(next, scale);
    *((Vector3*)&vertices[3]) = Vector3Subtract(next, scale);
}

void TrailRendererComponent_onUpdate(SceneObject* sceneObject, SceneComponentId sceneComponentId, float deltaTime, void* componentData)
{
    TrailRendererComponent* trailRendererComponent = (TrailRendererComponent*)componentData;
    if (trailRendererComponent->emitterRate <= 0 || trailRendererComponent->trailWidthCount == 0) {
        return;
    }

    for (int i=0;i<trailRendererComponent->nodeCount;i++)
    {
        TrailNode* node = &trailRendererComponent->nodes[i];
        node->time += deltaTime;
        if (node->time > trailRendererComponent->maxLifeTime) {
            trailRendererComponent->nodeCount = i;
            break;
        }
        else {
            node->position = Vector3Add(node->position, Vector3Scale(node->velocity, deltaTime));
        }
    }

    Vector3 worldPosition = SceneGraph_getWorldPosition(sceneObject->graph, sceneObject->id);
    Vector3 prevPosition = trailRendererComponent->lastPosition;
    float emitterInterval = 1.0f / trailRendererComponent->emitterRate;
    trailRendererComponent->time += deltaTime;
    while (trailRendererComponent->lastEmitTime < trailRendererComponent->time) {
        trailRendererComponent->lastEmitTime += emitterInterval;
        float tRemaining = trailRendererComponent->time - trailRendererComponent->lastEmitTime;
        Vector3 spawnPoint = worldPosition;
        if (tRemaining > 0.0f) {
            float t = 1.0f - tRemaining / deltaTime;
            if (t < 0.0f) t = 0.0f;
            spawnPoint = Vector3Lerp(prevPosition, worldPosition, t);
        }
        TrailRendererComponent_newNode(trailRendererComponent, spawnPoint, trailRendererComponent->emitterVelocity);
    }
    trailRendererComponent->lastPosition = worldPosition;
    
    // trail always starts at the current position and goes over its nodes - hence the +1
    int trailElements = trailRendererComponent->nodeCount + 1;
    if (trailElements < 2) {
        return;
    }

    Mesh *mesh = &trailRendererComponent->mesh;
    int vertexCount = trailElements * 2;
    if (vertexCount > trailRendererComponent->maxVertexCount) {
        trailRendererComponent->maxVertexCount *= 2;
        if (trailRendererComponent->maxVertexCount < vertexCount) {
            trailRendererComponent->maxVertexCount = vertexCount;
        }
        mesh->vertices = realloc(mesh->vertices, trailRendererComponent->maxVertexCount * sizeof(float) * 3);
        mesh->texcoords = realloc(mesh->texcoords, trailRendererComponent->maxVertexCount * sizeof(float) * 2);
        mesh->indices = realloc(mesh->indices, trailRendererComponent->maxVertexCount * sizeof(uint16_t) * 3 * 2);

        uint16_t index = 0;
        for (int i=0;i < trailRendererComponent->maxVertexCount * 3 * 2;i+=6)
        {
            mesh->indices[i] = index++; // 0
            mesh->indices[i + 1] = index++; // 1
            mesh->indices[i + 2] = index++; // 2

            mesh->indices[i + 3] = index - 1; // 2
            mesh->indices[i + 4] = index - 2; // 1
            mesh->indices[i + 5] = index++;
        }
    }
    mesh->vertexCount = vertexCount;
    mesh->triangleCount = trailElements * 2 - 2;
    float* vertices = mesh->vertices;
    float* texcoords = mesh->texcoords;
    
    vertices[0] = worldPosition.x;
    vertices[1] = worldPosition.y;
    vertices[2] = worldPosition.z;

    vertices[3] = worldPosition.x;
    vertices[4] = worldPosition.y;
    vertices[5] = worldPosition.z;
    
    texcoords[0] = 0.0f;
    texcoords[1] = 0.0f;

    texcoords[2] = 1.0f;
    texcoords[3] = 0.0f;

    for (int i=0;i<trailRendererComponent->nodeCount;i++)
    {
        TrailNode* node = &trailRendererComponent->nodes[i];
        vertices[(i + 1) * 6] = node->position.x;
        vertices[(i + 1) * 6 + 1] = node->position.y;
        vertices[(i + 1) * 6 + 2] = node->position.z;

        vertices[(i + 1) * 6 + 3] = node->position.x;
        vertices[(i + 1) * 6 + 4] = node->position.y;
        vertices[(i + 1) * 6 + 5] = node->position.z;

        texcoords[(i + 1) * 4] = 0.0f;
        texcoords[(i + 1) * 4 + 1] = 0.0f;

        texcoords[(i + 1) * 4 + 2] = 1.0f;
        texcoords[(i + 1) * 4 + 3] = 0.0f;
    }

    // the vertices are set, now update the widths
    Vector3 prev = worldPosition;
    Vector3 up = (Vector3) { 0, 1, 0 };
    TrailRendererComponent_setWidth(up, worldPosition, *((Vector3*)&vertices[6]), 1.0f, vertices);
    for (int i=0;i<trailRendererComponent->nodeCount - 1;i++)
    {
        Vector3 pos = *((Vector3*)&vertices[(i + 1) * 6]);
        Vector3 next = *((Vector3*)&vertices[(i + 3) * 6]);
        TrailRendererComponent_setWidth(up, prev, next, 1.0f, &vertices[(i + 1) * 6]);
        prev = pos;
    }
}

SceneComponentId AddTrailRendererComponent(SceneObjectId objectId, Mesh mesh, float emitterRate, float maxLifeTime, Vector3 emitterVelocity, int maxVertexCount)
{
    SceneComponentId componentId = SceneGraph_addComponent(psg.sceneGraph, objectId, psg.trailRendererComponentTypeId, &(ComponentInitializer) {
        .memData = &(TrailRendererComponent) {
            .mesh = mesh,
            .emitterRate = emitterRate,
            .maxLifeTime = maxLifeTime,
            .emitterVelocity = emitterVelocity,
            .maxVertexCount = maxVertexCount,
            .nodeCount = 0,
            .nodeCapacity = 0,
            .nodes = NULL,
            .trailWidths = NULL,
            .trailWidthCount = 0,
            .time = 0,
            .lastEmitTime = 0,
            .lastPosition = (Vector3) { 0, 0, 0 }
        }
    });
    return componentId;
}

void TrailRendererComponentRegister()
{
    psg.trailRendererComponentTypeId = SceneGraph_registerComponentType(psg.sceneGraph, "TrailRendererComponent", sizeof(TrailRendererComponent),
        (SceneComponentTypeMethods) {
            .onInitialize = TrailRendererComponent_onInitialize,
            .onDestroy = TrailRendererComponent_onDestroy,
            .updateTick = TrailRendererComponent_onUpdate,
        });
}