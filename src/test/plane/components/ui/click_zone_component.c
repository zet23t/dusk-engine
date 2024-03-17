#undef COMPONENT_NAME
#define COMPONENT_NAME ClickZoneComponent

#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(ClickZoneComponent)
#elif defined(COMPONENT_DECLARATION)

typedef struct COMPONENT_NAME {
    Vector3 boxSize;
    int zoneId;
    
} COMPONENT_NAME;

#else
#include "../../plane_sim_g.h"
#include "stdlib.h"
#include "stdio.h"
static void ClickZone_update(SceneObject* sceneObject, SceneComponentId sceneComponentId,
    float delta, void* componentData)
{
    ClickZoneComponent* clickZone = (ClickZoneComponent*)componentData;
    Camera3D camera = CameraComponent_getCamera3D(sceneObject->graph, psg.camera);

    Vector2 mousePos = GetMousePosition();
    Ray ray = GetMouseRay(mousePos, camera);
    // printf("Ray: %.2f %.2f %.2f -> %.2f %.2f %.2f\n", ray.position.x, ray.position.y, ray.position.z, ray.direction.x, ray.direction.y, ray.direction.z);
    Matrix m = SceneObject_getWorldMatrix(sceneObject);
    m = MatrixInvert(m);
    ray.position = Vector3Transform(ray.position, m);
    m.m12 = 0;
    m.m13 = 0;
    m.m14 = 0;
    ray.direction = Vector3Transform(ray.direction, m);
    BoundingBox box = {
        .min = (Vector3) { -clickZone->boxSize.x / 2, -clickZone->boxSize.y / 2, -clickZone->boxSize.z / 2 },
        .max = (Vector3) { clickZone->boxSize.x / 2, clickZone->boxSize.y / 2, clickZone->boxSize.z / 2 },
    };
    RayCollision rayCollision = GetRayCollisionBox(ray, box);
    if (rayCollision.hit) {
        printf("click zone hit: %.2f %.2f %.2f -> %.2f %.2f %.2f (%.2f %.2f %.2f): %.2f %.2f %.2f\n", 
            ray.position.x, ray.position.y, ray.position.z, ray.direction.x, ray.direction.y, ray.direction.z,
            clickZone->boxSize.x, clickZone->boxSize.y, clickZone->boxSize.z,
            rayCollision.point.x, rayCollision.point.y, rayCollision.point.z);
    }

    MessageHub_queueClickZoneMessage((ClickZoneMessage) {
        .buttonComponentId = sceneComponentId,
        .zoneId = ((ClickZoneComponent*)componentData)->zoneId,
    });
}

BEGIN_COMPONENT_REGISTRATION {
    .updateTick = ClickZone_update,
} END_COMPONENT_REGISTRATION

#endif