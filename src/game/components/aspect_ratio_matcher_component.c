#undef COMPONENT_NAME
#define COMPONENT_NAME AspectRatioMatcherComponent

#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(AspectRatioMatcherComponent)
#elif defined(COMPONENT_DECLARATION)
// === COMPONENT DECLARATION ===
typedef struct COMPONENT_NAME {
    SceneComponentId cameraComponentId;
    float canvasDistance;
    float width, height;
} COMPONENT_NAME;

#else
// === COMPONENT IMPLEMENTATION ===
#include "../plane_sim_g.h"
static void AspectRatioMatcher(SceneObject* sceneObject, SceneComponentId sceneComponentId,
    float delta, void* componentData)
{
    AspectRatioMatcherComponent* aspectRatioMatcher = (AspectRatioMatcherComponent*)componentData;
    CameraComponent* camera;
    SceneComponent* cameraComponent = SceneGraph_getComponentOrFindByType(psg.sceneGraph, (SceneObjectId) { 0 }, &aspectRatioMatcher->cameraComponentId, psg.cameraComponentId, (void**)&camera);

    if (camera == NULL) {
        return;
    }
    SceneObject* cameraObject = SceneGraph_getObject(sceneObject->graph, cameraComponent->objectId);
    if (cameraObject == NULL) {
        return;
    }
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();
    // Assume these are given
    float rectWidth = aspectRatioMatcher->width, rectHeight = aspectRatioMatcher->height;
    float cameraDistance = aspectRatioMatcher->canvasDistance;

    // Calculate aspect ratios
    float screenAspect = screenWidth / screenHeight;
    float rectAspect = rectWidth / rectHeight;

    float fov;

    // Calculate FOV
    if (screenAspect >= rectAspect) {
        // Screen is wider than rectangle, calculate FOV based on rectangle height
        fov = 2.0f * atan(rectHeight / (2.0f * cameraDistance));
    } else {
        // Screen is narrower than rectangle, calculate FOV based on rectangle width
        // Adjust for screen aspect ratio
        fov = 2.0f * atan((rectWidth / screenAspect) / (2.0f * cameraDistance));
    }

    // Convert FOV to degrees
    fov = fov * 180.0f / PI;
    camera->fov = fov;
}

#include "../util/component_macros.h"

BEGIN_COMPONENT_REGISTRATION(AspectRatioMatcherComponent) {
    .updateTick = AspectRatioMatcher,
} END_COMPONENT_REGISTRATION

#endif