#include "game/game.h"

static void MeshRendererDraw(Camera3D camera, SceneObject* node, SceneComponentId sceneComponentId, void* component, void* userdata)
{
    MeshRendererComponent* meshRenderer = (MeshRendererComponent*)component;
    Matrix m = SceneObject_getWorldMatrix(node);
    DrawMesh(*meshRenderer->mesh, *meshRenderer->material, m);
}

BEGIN_COMPONENT_REGISTRATION(MeshRendererComponent) {
    .draw = MeshRendererDraw,
} END_COMPONENT_REGISTRATION
