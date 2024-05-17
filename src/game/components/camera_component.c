#include "../game_g.h"

Camera3D CameraComponent_getCamera3D(SceneGraph* sceneGraph, SceneObjectId nodeId)
{
    CameraComponent* cameraComponent;
    if (!SceneGraph_getComponentByType(sceneGraph, nodeId, psg.CameraComponentId, (void**)&cameraComponent, 0))
    {
        return (Camera3D) {0};
    }

    Vector3 forward = SceneGraph_getWorldForward(sceneGraph, nodeId);
    Vector3 up = SceneGraph_getWorldUp(sceneGraph, nodeId);
    Vector3 position = SceneGraph_getWorldPosition(sceneGraph, nodeId);
    return (Camera3D) { 
        .position = position,
        .target = Vector3Add(position, forward),
        .up = up,
        .fovy = cameraComponent->camera.fovy,
        .far = cameraComponent->camera.far,
        .near = cameraComponent->camera.near,
    };
}
