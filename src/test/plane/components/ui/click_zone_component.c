#undef COMPONENT_NAME
#define COMPONENT_NAME ClickZoneComponent

#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(ClickZoneComponent)
#elif defined(COMPONENT_DECLARATION)

#define CLICKZONECOMPONENT_FLAG_NONE 0
#define CLICKZONECOMPONENT_FLAG_HOVER 1
#define CLICKZONECOMPONENT_FLAG_ACTIVE 2

typedef struct COMPONENT_NAME {
    Vector3 boxSize;
    int zoneId;
    int16_t flags;
    int16_t mouseButtonLock;
    int touchIdLock;
} COMPONENT_NAME;

#else
#include "../../plane_sim_g.h"
#include "stdio.h"
#include "stdlib.h"

static int ClickZone_isHit(Vector2 inputPos, Camera3D camera, Matrix m, ClickZoneComponent* clickZone)
{
    Ray ray = GetMouseRay(inputPos, camera);
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
    return rayCollision.hit;
}

static void ClickZone_update(SceneObject* sceneObject, SceneComponentId sceneComponentId,
    float delta, void* componentData)
{
    ClickZoneComponent* clickZone = (ClickZoneComponent*)componentData;
    Camera3D camera = CameraComponent_getCamera3D(sceneObject->graph, psg.camera);

    // let's assume there's potentially a valid touch id 0 - -1 mean there's no lock
    int touchIdLock = clickZone->touchIdLock - 1;
    int touchCount = GetTouchPointCount();
    Matrix m = SceneObject_getWorldMatrix(sceneObject);
    m = MatrixInvert(m);
    int isHit;
    if (touchIdLock >= 0) {
        int phase = GetTouchPhase(touchIdLock);
        if (phase == TOUCH_PHASE_ENDED) {
            int isClick = clickZone->flags & CLICKZONECOMPONENT_FLAG_HOVER;
            clickZone->touchIdLock = -1;
            clickZone->flags = CLICKZONECOMPONENT_FLAG_NONE;
            MessageHub_queueClickZoneMessage((ClickZoneMessage) {
                .buttonComponentId = sceneComponentId,
                .zoneId = ((ClickZoneComponent*)componentData)->zoneId,
                .flags = CLICK_ZONE_MESSAGE_FLAG_POINTER_WAS_RELEASED });
            if (isClick) {
                MessageHub_queueClickZoneMessage((ClickZoneMessage) {
                    .buttonComponentId = sceneComponentId,
                    .zoneId = ((ClickZoneComponent*)componentData)->zoneId,
                    .flags = CLICK_ZONE_MESSAGE_FLAG_CLICK });
            }
        }

        Vector2 lockedTouchPoint = GetTouchPositionById(touchIdLock);
        isHit = ClickZone_isHit(lockedTouchPoint, camera, m, clickZone);
        if (isHit)
            clickZone->flags |= CLICKZONECOMPONENT_FLAG_HOVER;
        else
            clickZone->flags &= ~CLICKZONECOMPONENT_FLAG_HOVER;

        return;
    }

    if (touchIdLock >= 0) {
        return;
    }

    int mouseButtonLock = clickZone->mouseButtonLock - 1;
    if (IsMouseButtonReleased(mouseButtonLock) && clickZone->flags == CLICKZONECOMPONENT_FLAG_HOVER) {
        clickZone->flags = CLICKZONECOMPONENT_FLAG_NONE;
        clickZone->mouseButtonLock = -1;
        MessageHub_queueClickZoneMessage((ClickZoneMessage) {
            .buttonComponentId = sceneComponentId,
            .zoneId = ((ClickZoneComponent*)componentData)->zoneId,
            .flags = CLICK_ZONE_MESSAGE_FLAG_POINTER_WAS_RELEASED });
        MessageHub_queueClickZoneMessage((ClickZoneMessage) {
            .buttonComponentId = sceneComponentId,
            .zoneId = ((ClickZoneComponent*)componentData)->zoneId,
            .flags = CLICK_ZONE_MESSAGE_FLAG_CLICK });
        return;
    }

    for (int i = 0; i < touchCount; i += 1) {
        Vector2 touchPos = GetTouchPosition(i);
        int touchId = GetTouchPointId(i);
        int phase = GetTouchPhase(touchId);
        if (phase != TOUCH_PHASE_BEGAN) {
            continue;
        }

        isHit = ClickZone_isHit(touchPos, camera, m, clickZone);
        if (isHit) {
            clickZone->flags = CLICKZONECOMPONENT_FLAG_HOVER | CLICKZONECOMPONENT_FLAG_ACTIVE;
            clickZone->touchIdLock = touchId + 1;
            MessageHub_queueClickZoneMessage((ClickZoneMessage) {
                .buttonComponentId = sceneComponentId,
                .zoneId = ((ClickZoneComponent*)componentData)->zoneId,
                .flags = CLICK_ZONE_MESSAGE_FLAG_POINTER_WAS_PRESSED });
            return;
        }
    }

    isHit = ClickZone_isHit(GetMousePosition(), camera, m, clickZone);
    if (!isHit) {
        if (clickZone->flags & CLICKZONECOMPONENT_FLAG_HOVER) {
            clickZone->flags &= ~CLICKZONECOMPONENT_FLAG_HOVER;
            MessageHub_queueClickZoneMessage((ClickZoneMessage) {
                .buttonComponentId = sceneComponentId,
                .flags = CLICK_ZONE_MESSAGE_FLAG_EXIT,
                .zoneId = ((ClickZoneComponent*)componentData)->zoneId,
            });
        }
        return;
    }
    if (!clickZone->flags & CLICKZONECOMPONENT_FLAG_HOVER) {
        clickZone->flags |= CLICKZONECOMPONENT_FLAG_HOVER;
        MessageHub_queueClickZoneMessage((ClickZoneMessage) {
            .buttonComponentId = sceneComponentId,
            .flags = CLICK_ZONE_MESSAGE_FLAG_ENTER,
            .zoneId = ((ClickZoneComponent*)componentData)->zoneId,
        });
    }

    for (int i = MOUSE_BUTTON_LEFT; i <= MOUSE_BUTTON_MIDDLE; i++) {
        if (IsMouseButtonPressed(i)) {
            clickZone->flags |= CLICKZONECOMPONENT_FLAG_ACTIVE;
            clickZone->mouseButtonLock = i + 1;
            MessageHub_queueClickZoneMessage((ClickZoneMessage) {
                .buttonComponentId = sceneComponentId,
                .flags = CLICK_ZONE_MESSAGE_FLAG_POINTER_WAS_PRESSED,
                .zoneId = ((ClickZoneComponent*)componentData)->zoneId,
            });
            return;
        }
    }
}

BEGIN_COMPONENT_REGISTRATION {
    .updateTick = ClickZone_update,
} END_COMPONENT_REGISTRATION

#endif