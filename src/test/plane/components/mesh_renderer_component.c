#include "../plane_sim_g.h"

static void MeshRendererDraw(Camera3D camera, SceneObject* node, SceneComponentId sceneComponentId, void* component, void* userdata)
{
    MeshRendererComponent* meshRenderer = (MeshRendererComponent*)component;
    Matrix m = SceneObject_getWorldMatrix(node);
    if (psg.disableDrawMesh)
    {
        return;
    }
    SetShaderValue(meshRenderer->material->shader, psg.litAmountIndex, &meshRenderer->litAmount, SHADER_UNIFORM_FLOAT);
    if (meshRenderer->mesh == NULL)
    {
        DrawCube(Vector3Transform((Vector3){0,0,0}, m), 1, 1, 1, PINK);
        return;
    }
    DrawMesh(*meshRenderer->mesh, *meshRenderer->material, m);
}

static void MeshRendererInitialize(SceneObject* node, SceneComponentId sceneComponentId, void* componentData, void *initArg)
{
    MeshRendererComponent* meshRenderer = (MeshRendererComponent*)componentData;
    ComponentInitializer* ci = (ComponentInitializer*)initArg;
    memset(meshRenderer, 0, sizeof(MeshRendererComponent));
    if (ci == NULL) {
        return;
    }
    if (ci->memData != NULL) {
        memcpy(meshRenderer, ci->memData, sizeof(MeshRendererComponent));
        return;
    }
    if (ci->config != NULL) {
        meshRenderer->material = &psg.model.materials[1];
        cJSON* meshName = cJSON_GetObjectItem(ci->config, "mesh");
        cJSON* litAmount = cJSON_GetObjectItem(ci->config, "litAmount");
        if (!cJSON_IsString(meshName)) {
            TraceLog(LOG_ERROR, "json initializer for MeshRendererComponent missing mesh");
            return;
        }
        for (int i=0;i<psg.model.meshCount;i++)
        {
            if (strcmp(psg.model.meshes[i].name, meshName->valuestring) == 0)
            {
                meshRenderer->mesh = &psg.model.meshes[i];
                break;
            }
        }
        if (meshRenderer->mesh == NULL)
        {
            TraceLog(LOG_ERROR, "mesh not found: %s", meshName->valuestring);
            return;
        }
        if (cJSON_IsNumber(litAmount)) {
            meshRenderer->litAmount = litAmount->valuedouble;
        }
    }
}

SceneComponentId AddMeshRendererComponent(SceneObjectId id, Mesh* mesh, float litAmount)
{
    // return (SceneComponentId){0};
    return SceneGraph_addComponent(psg.sceneGraph, id, psg.meshRendererComponentId, &(ComponentInitializer) {
        .memData = &(MeshRendererComponent) {
            .material = &psg.model.materials[1],
            .mesh = mesh,
            .litAmount = litAmount,
        }
    });
}

void MeshRendererComponentRegister()
{
    psg.meshRendererComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "MeshRendererComponent", sizeof(MeshRendererComponent),
        (SceneComponentTypeMethods) {
            .draw = MeshRendererDraw,
            .initialize = MeshRendererInitialize,
        });
}