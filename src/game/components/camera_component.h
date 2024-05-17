#ifdef COMPONENT
COMPONENT(CameraComponent)
#elif defined(COMPONENT_DECLARATION)

SERIALIZABLE_STRUCT_START(CameraComponent)
    SERIALIZABLE_FIELD(bool, trackTarget)
    SERIALIZABLE_FIELD(float, targetDistance)
    SERIALIZABLE_FIELD(Vector3, targetPoint)
    NONSERIALIZED_FIELD(Camera3D, camera)
SERIALIZABLE_STRUCT_END(CameraComponent)

#elif defined(COMPONENT_IMPLEMENTATION)

#include "camera_component.c"

BEGIN_COMPONENT_REGISTRATION(CameraComponent) {
} END_COMPONENT_REGISTRATION
#else
Camera3D CameraComponent_getCamera3D(SceneGraph* sceneGraph, SceneObjectId nodeId);
#endif