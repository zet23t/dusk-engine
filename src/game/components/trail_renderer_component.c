#include "../game_g.h"
#include <memory.h>
#include <string.h>
#include <stdlib.h>

void TrailRendererComponent_setMaterial(TrailRendererComponent* trailRendererComponent, Material material, int ownsMaterial)
{
    if (trailRendererComponent->ownsMaterial && trailRendererComponent->material.shader.id > 0)
    {
        UnloadMaterial(trailRendererComponent->material);
    }

    trailRendererComponent->material = material;
    trailRendererComponent->ownsMaterial = ownsMaterial;
}

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
        trailRendererComponent->currentWidthPercentage = 1.0f;
        trailRendererComponent->widthDecayRate = 0.0f;

        TrailRendererComponent_setMaterial(trailRendererComponent, LoadMaterialDefault(), 1);
        cJSON* cfg = initializer->config;
        MappedVariable mappedVariables[] = {
            { .name = "maxLifeTime", .type = VALUE_TYPE_FLOAT, .floatValue = &trailRendererComponent->maxLifeTime },
            { .name = "emitterRate", .type = VALUE_TYPE_FLOAT, .floatValue = &trailRendererComponent->emitterRate },
            { .name = "emitterVelocity", .type = VALUE_TYPE_VEC3, .vec3Value = &trailRendererComponent->emitterVelocity },
            { .name = "currentWidthPercentage", .type = VALUE_TYPE_FLOAT, .floatValue = &trailRendererComponent->currentWidthPercentage },
            { .name = "widthDecayRate", .type = VALUE_TYPE_FLOAT, .floatValue = &trailRendererComponent->widthDecayRate },
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
            for (int i = 0; i < n; i += 2) {
                cJSON* percent = cJSON_GetArrayItem(widths, i);
                cJSON* width = cJSON_GetArrayItem(widths, i + 1);
                if (cJSON_IsNumber(width) && cJSON_IsNumber(percent)) {
                    trailRendererComponent->trailWidths[i>>1].width = (float)width->valuedouble;
                    trailRendererComponent->trailWidths[i>>1].percent = (float)percent->valuedouble;
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
        else {
            TraceLog(LOG_ERROR, "a valid trailWidths array must be provided, otherwise the trail will not render");
        }
    }
}

void TrailRendererComponent_onDestroy(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData)
{
    TrailRendererComponent* trailRendererComponent = (TrailRendererComponent*)componentData;
    if (trailRendererComponent->mesh.vertices != NULL)
        UnloadMesh(trailRendererComponent->mesh);
    if (trailRendererComponent->trailWidths) {
        free(trailRendererComponent->trailWidths);
    }
    if (trailRendererComponent->nodes) {
        free(trailRendererComponent->nodes);
    }
    if (trailRendererComponent->ownsMaterial && trailRendererComponent->material.shader.id > 0)
    {
        UnloadMaterial(trailRendererComponent->material);
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
        if (trailRendererComponent->mesh.vertices != NULL)
        {
            UnloadMesh(trailRendererComponent->mesh);
            memset(&trailRendererComponent->mesh, 0, sizeof(Mesh));
        }
    }
    memmove(trailRendererComponent->nodes + 1, trailRendererComponent->nodes, trailRendererComponent->nodeCount * sizeof(TrailNode));
    TrailNode* node = &trailRendererComponent->nodes[0];
    node->position = spawnPoint;
    node->velocity = velocity;
    node->time = 0;
    node->widthPercent = trailRendererComponent->currentWidthPercentage;

    trailRendererComponent->nodeCount++;
}

static void TrailRendererComponent_setWidth(Vector3 vup, Vector3 prev, Vector3 next, float width, float* vertices)
{
    if (width < 0.0001f) {
        return;
    }
    Vector3 pos = *(Vector3*)vertices;
    float length = Vector3Length(Vector3Subtract(next, prev));
    if (length < 0.0001f) {
        return;
    }
    Vector3 dir = Vector3Scale(Vector3Subtract(next, prev), 1.0f / length);
    Vector3 right = Vector3Normalize(Vector3CrossProduct(dir, vup));
    // Vector3 up = Vector3CrossProduct(right, dir);
    Vector3 scale = Vector3Scale(right, width);
    *((Vector3*)&vertices[0]) = Vector3Subtract(pos, scale);
    *((Vector3*)&vertices[3]) = Vector3Add(pos, scale);
}

void TrailRendererComponent_onUpdate(SceneObject* sceneObject, SceneComponentId sceneComponentId, float deltaTime, void* componentData)
{
    TrailRendererComponent* trailRendererComponent = (TrailRendererComponent*)componentData;
    if (trailRendererComponent->emitterRate <= 0 || trailRendererComponent->trailWidthCount == 0) {
        return;
    }

    trailRendererComponent->meshIsDirty = 1;
    for (int i = 0; i < trailRendererComponent->nodeCount; i++) {
        TrailNode* node = &trailRendererComponent->nodes[i];
        node->time += deltaTime;
        if (node->time > trailRendererComponent->maxLifeTime) {
            trailRendererComponent->nodeCount = i;
            break;
        } else {
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
            if (t < 0.0f)
                t = 0.0f;
            spawnPoint = Vector3Lerp(prevPosition, worldPosition, t);
        }
        TrailRendererComponent_newNode(trailRendererComponent, spawnPoint, trailRendererComponent->emitterVelocity);
        trailRendererComponent->currentWidthPercentage -= emitterInterval * trailRendererComponent->widthDecayRate;
        if (trailRendererComponent->currentWidthPercentage < 0.0f) {
            trailRendererComponent->currentWidthPercentage = 0.0f;
        }
    }
    trailRendererComponent->lastPosition = worldPosition;
}

SceneComponentId AddTrailRendererComponent(SceneObjectId objectId, float emitterRate, float maxLifeTime, Vector3 emitterVelocity, int maxVertexCount, Material material, int ownsMaterial)
{
    SceneComponentId componentId = SceneGraph_addComponent(psg.sceneGraph, objectId, 
        psg.trailRendererComponentTypeId, &(ComponentInitializer) { 
            .memData = &(TrailRendererComponent) 
            {
                .mesh = {0},
                .material = material,
                .ownsMaterial = ownsMaterial,
                .emitterRate = emitterRate, 
                .maxLifeTime = maxLifeTime, 
                .emitterVelocity = emitterVelocity, 
                .nodeCount = 0, 
                .nodeCapacity = 0, 
                .nodes = NULL, 
                .trailWidths = NULL, 
                .trailWidthCount = 0, 
                .currentWidthPercentage = 1.0f,
                .time = 0, 
                .lastEmitTime = 0, 
                .lastPosition = (Vector3) { 0, 0, 0 } 
            } 
        });
    return componentId;
}

void TrailRendererComponent_onDraw(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void* userdata)
{
    TrailRendererComponent* trailRendererComponent = (TrailRendererComponent*)componentData;
    Mesh* mesh = &trailRendererComponent->mesh;
    if (trailRendererComponent->meshIsDirty) {
        trailRendererComponent->meshIsDirty = 0;

        // trail always starts at the current position and goes over its nodes - hence the +1
        int trailElements = trailRendererComponent->nodeCapacity + 1;
        if (trailElements < 2) {
            return;
        }

        int vertexCount = trailElements * 2;
        if (trailRendererComponent->mesh.vertices == NULL) {
            mesh->vertices = (float*) malloc(vertexCount * sizeof(float) * 3);
            memset(mesh->vertices, 0, vertexCount * sizeof(float) * 3);

            mesh->texcoords = (float*) malloc(vertexCount * sizeof(float) * 2);
            memset(mesh->texcoords, 0, vertexCount * sizeof(float) * 2);
            
            mesh->indices = (uint16_t*) malloc(trailElements * sizeof(uint16_t) * 3 * 2);
            memset(mesh->indices, 0, trailElements * sizeof(uint16_t) * 3 * 2);

            // generate quad strip indices
            uint16_t index = 0;
            for (int i = 0; i < trailElements * 3 * 2; i += 6) {
                mesh->indices[i] = index;
                mesh->indices[i + 1] = index + 1;
                mesh->indices[i + 2] = index + 2;

                mesh->indices[i + 3] = index + 2;
                mesh->indices[i + 4] = index + 1;
                mesh->indices[i + 5] = index + 3;
                index += 2;
            }
        }
        mesh->vertexCount = vertexCount;
    }

    if (mesh->vertices == NULL) {
        return;
    }

    mesh->triangleCount = (trailRendererComponent->nodeCapacity) * 2;
    float* vertices = mesh->vertices;
    float* texcoords = mesh->texcoords;

    Vector3 worldPosition = SceneGraph_getWorldPosition(sceneObject->graph, sceneObject->id);

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

    for (int i = 0; i < trailRendererComponent->nodeCount; i++) {
        TrailNode* node = &trailRendererComponent->nodes[i];
        vertices[(i + 1) * 6] = node->position.x;
        vertices[(i + 1) * 6 + 1] = node->position.y;
        vertices[(i + 1) * 6 + 2] = node->position.z;

        vertices[(i + 1) * 6 + 3] = node->position.x;
        vertices[(i + 1) * 6 + 4] = node->position.y;
        vertices[(i + 1) * 6 + 5] = node->position.z;
    }

    float totalDistance = 0.0f;
    float distances[trailRendererComponent->nodeCount];
    distances[0] = 0.0f;
    for (int i = 1; i < trailRendererComponent->nodeCount; i++) {
        Vector3 a = *((Vector3*)&vertices[i * 6 - 6]);
        Vector3 b = *((Vector3*)&vertices[i * 6]);
        totalDistance += Vector3Distance(a, b);
        distances[i] = totalDistance;
    }

    // the vertices are set, now update the widths
    Vector3 prev = *((Vector3*)&vertices[0]);
    Vector3 up = Vector3Subtract(camera.position, camera.target);

    int next = 6;
    // the first point is attached to the emitting point and it can happen that it's 
    // extremely close to the next point since it was just emitted; in that and for other cases
    // let's just skip the first point for the width calculation and pick the next one. 
    // sort of a smoothing for the first point
    if (trailRendererComponent->nodeCount > 2) next = 12;
    int widthIndex = 0;
    TrailWidthStep currentStep = trailRendererComponent->trailWidths[widthIndex];
    TrailWidthStep nextStep = trailRendererComponent->trailWidths[widthIndex + 1 < trailRendererComponent->trailWidthCount ? (widthIndex + 1) : widthIndex];
    
    TrailRendererComponent_setWidth(up, *((Vector3*)&vertices[0]), *((Vector3*)&vertices[next]), currentStep.width * trailRendererComponent->currentWidthPercentage, vertices);
    // there's 1 more vertex than nodes due to the attached first point
    int last = trailRendererComponent->nodeCount;
    int twcount = trailRendererComponent->trailWidthCount;
    float progressedDistance = 0;
    float uvMul = trailRendererComponent->uvDistanceFactor > 0.0f ? 1.0f / trailRendererComponent->uvDistanceFactor : 1.0f / totalDistance;
    for (int i = 1; i <= last; i++) {
        Vector3 pos = *((Vector3*)&vertices[i * 6]);
        Vector3 next = i < last ? *((Vector3*)&vertices[i * 6 + 6]) : pos;
        TrailNode* node = &trailRendererComponent->nodes[i - 1];
        float progress = node->time / trailRendererComponent->maxLifeTime;
        if (progress > nextStep.percent && widthIndex < twcount) {
            widthIndex++;
            currentStep = trailRendererComponent->trailWidths[widthIndex < twcount ? widthIndex : twcount - 1];
            nextStep = trailRendererComponent->trailWidths[widthIndex + 1 < twcount ? (widthIndex + 1) : twcount - 1];
        }
        float width = nextStep.width;
        if (progress < nextStep.percent) {
            width = Lerp(currentStep.width, nextStep.width, (progress - currentStep.percent) / (nextStep.percent - currentStep.percent));
        }
        width *= node->widthPercent;
        TrailRendererComponent_setWidth(up, prev, next, width, &vertices[i * 6]);
        prev = pos;

        progressedDistance = distances[i];
        float v = progressedDistance * uvMul;

        texcoords[(i) * 4] = 0.0f;
        texcoords[(i) * 4 + 1] = v;

        texcoords[(i) * 4 + 2] = 1.0f;
        texcoords[(i) * 4 + 3] = v;

    }
    int lastV = last * 6; 
    Vector3 lastP = *((Vector3*)&vertices[lastV - 3]);
    for (int i = lastV; i < mesh->vertexCount * 3; i += 3) {
        vertices[i] = lastP.x;
        vertices[i + 1] = lastP.y;
        vertices[i + 2] = lastP.z;
    }

    if (mesh->vaoId < 1) {
        UploadMesh(mesh, 1);
    } else {
        UpdateMeshBuffer(*mesh, 0, mesh->vertices, mesh->vertexCount * 3 * sizeof(float), 0);
        UpdateMeshBuffer(*mesh, 1, mesh->texcoords, mesh->vertexCount * 2 * sizeof(float), 0);
    }

    if (trailRendererComponent->mesh.vboId != NULL)
    {
        rlDisableDepthMask();
        DrawMesh(trailRendererComponent->mesh, trailRendererComponent->material, MatrixIdentity());
        rlEnableDepthMask();
    }

#if DEBUG
    for (int i=0;i<trailRendererComponent->mesh.triangleCount;i++)
    {
        uint16_t ia = trailRendererComponent->mesh.indices[i*3];
        uint16_t ib = trailRendererComponent->mesh.indices[i*3+1];
        uint16_t ic = trailRendererComponent->mesh.indices[i*3+2];
        Vector3 a = *((Vector3*)&trailRendererComponent->mesh.vertices[ia*3]);
        Vector3 b = *((Vector3*)&trailRendererComponent->mesh.vertices[ib*3]);
        Vector3 c = *((Vector3*)&trailRendererComponent->mesh.vertices[ic*3]);
        // DrawTriangle3D(a, b, c, RED);
        DrawLine3D(a, b, BLUE);
        DrawLine3D(b, c, BLUE);
        DrawLine3D(c, a, BLUE);
    }
#endif
}

void TrailRendererComponent_addTrailWidth(TrailRendererComponent* trailRendererComponent, float width, float percent)
{
    trailRendererComponent->trailWidthCount++;
    trailRendererComponent->trailWidths = (TrailWidthStep*)realloc(trailRendererComponent->trailWidths, trailRendererComponent->trailWidthCount * sizeof(TrailWidthStep));
    trailRendererComponent->trailWidths[trailRendererComponent->trailWidthCount - 1].width = width;
    trailRendererComponent->trailWidths[trailRendererComponent->trailWidthCount - 1].percent = percent;
}

void TrailRendererComponentRegister()
{
    psg.trailRendererComponentTypeId = SceneGraph_registerComponentType(psg.sceneGraph, "TrailRendererComponent", sizeof(TrailRendererComponent),
        (SceneComponentTypeMethods) {
            .onInitialize = TrailRendererComponent_onInitialize,
            .onDestroy = TrailRendererComponent_onDestroy,
            .updateTick = TrailRendererComponent_onUpdate,
            .draw = TrailRendererComponent_onDraw });
}