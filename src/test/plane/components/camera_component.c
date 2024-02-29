#include "../plane_sim_g.h"

Camera3D CameraComponent_getCamera3D(SceneGraph* sceneGraph, SceneObjectId nodeId)
{
    CameraComponent* cameraComponent;
    if (!SceneGraph_getComponentByType(sceneGraph, nodeId, psg.cameraComponentId, (void**)&cameraComponent, 0))
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
        .fovy = cameraComponent->fov,
    };
}

void CameraComponentRegister()
{
    psg.cameraComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "Camera", sizeof(CameraComponent),
        (SceneComponentTypeMethods) {
            0,
        });
}
