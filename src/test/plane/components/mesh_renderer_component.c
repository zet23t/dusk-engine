#include "../plane_sim_g.h"

static void MeshRendererDraw(Camera3D camera, SceneObject* node, SceneComponentId sceneComponentId, void* component, void* userdata)
{
    MeshRendererComponent* meshRenderer = (MeshRendererComponent*)component;
    Matrix m = SceneObject_getWorldMatrix(node);
    DrawMesh(*meshRenderer->mesh, meshRenderer->material, m);
}

void MeshRendererComponentRegister()
{
    psg.meshRendererComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "MeshRenderer", sizeof(MeshRendererComponent),
        (SceneComponentTypeMethods) {
            .draw = MeshRendererDraw,
        });
}