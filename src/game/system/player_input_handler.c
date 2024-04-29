#include "../game_g.h"

static float planeMovementRangeX = 5.0f;
static float planeMovementRangeZ = 5.0f;
static float planeOffsetZ = 0;

static cJSON* playerInputConfigMarker = NULL;

void HandlePlayerInputUpdate(SceneObject *object, SceneComponentId componentId, float dt, void *userdata)
{
    if (playerInputConfigMarker != psg.levelConfig) {
        playerInputConfigMarker = psg.levelConfig;
        cJSON* playerInputConfig = cJSON_GetObjectItem(psg.levelConfig, "playerInput");
        if (playerInputConfig != NULL) {
            MappedVariable variables[] = {
                { "movementRangeX", VALUE_TYPE_FLOAT, .floatValue = &planeMovementRangeX },
                { "movementRangeZ", VALUE_TYPE_FLOAT, .floatValue = &planeMovementRangeZ },
                { "offsetZ", VALUE_TYPE_FLOAT, .floatValue = &planeOffsetZ },
                { 0 }
            };
            ReadMappedVariables(playerInputConfig, variables);
        }
    }
    if (psg.deltaTime == 0) return;
    Vector3 position = SceneGraph_getLocalPosition(psg.sceneGraph, psg.playerPlane);
    LinearVelocityComponent *velocity;
    SceneGraph_getComponentByType(psg.sceneGraph, psg.playerPlane, psg.LinearVelocityComponentId, (void**)&velocity, 0);
    if (velocity == NULL) return;

    int cnt = GetTouchPointCount();

    static int prevCnt = 0;
    static int prevX = 0;
    static int prevY = 0;
    int touchX = cnt > 0 ? GetTouchX() : GetMouseX();
    int touchY = cnt > 0 ? GetTouchY() : GetMouseY();
    float div = GetScreenWidth() < GetScreenHeight() ? GetScreenHeight() : GetScreenWidth();
    if ((prevCnt > 0 && cnt > 0) || (cnt == 0 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)))
    {
        int dx = touchX - prevX;
        int dy = touchY - prevY;

        velocity->velocity.x -= dx * psg.deltaTime * 15000.0f / div;
        velocity->velocity.z -= dy * psg.deltaTime * 15000.0f / div;
    }
    prevX = touchX;
    prevY = touchY;
    prevCnt = cnt;

    if (IsKeyDown(KEY_LEFT)) {
        velocity->velocity.x += psg.deltaTime * 50.0f;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        velocity->velocity.x -= psg.deltaTime * 50.0f;
    }
    if (IsKeyDown(KEY_UP)) {
        velocity->velocity.z += psg.deltaTime * 50.0f;
    }
    if (IsKeyDown(KEY_DOWN)) {
        velocity->velocity.z -= psg.deltaTime * 50.0f;
    }
    if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || cnt > 1 ) {
        for (int i=0;1;i++)
        {
            ShootingComponent *shooting;
            SceneGraph_getComponentByType(psg.sceneGraph, psg.playerPlane, psg.ShootingComponentId, (void**)&shooting, i);
            if (shooting == NULL) break;
            shooting->shooting = 1;
        }
    }

    const float movementRangeX = planeMovementRangeX;
    const float movementRangeZ = planeMovementRangeZ;
    if (position.x > movementRangeX)
    {
        velocity->velocity.x = velocity->velocity.x * .5f - (position.x - movementRangeX) * 1.0f;
    }
    else if (position.x < -movementRangeX)
    {
        velocity->velocity.x = velocity->velocity.x * .5f - (position.x + movementRangeX) * 1.0f;
    }
    if (position.z + planeOffsetZ > movementRangeZ)
    {
        velocity->velocity.z = velocity->velocity.z * .5f - (position.z + planeOffsetZ - movementRangeZ) * 1.0f;
    }
    else if (position.z + planeOffsetZ < -movementRangeZ)
    {
        velocity->velocity.z = velocity->velocity.z * .5f - (position.z + planeOffsetZ + movementRangeZ) * 1.0f;
    }

    Vector3 rotation = SceneGraph_getLocalRotation(psg.sceneGraph, psg.playerPlane);
    rotation.z += -velocity->velocity.x * 1.0f;
    SceneGraph_setLocalRotation(psg.sceneGraph, psg.playerPlane, rotation);
    // SceneGraph_setLocalRotation(psg.sceneGraph, plane, (Vector3) { 0, t * 30, 0 });

}

void PlayerInputHandlerRegister()
{
    psg.playerInputHandlerId = SceneGraph_registerComponentType(psg.sceneGraph, "PlayerInputHandler",
        0,
        (SceneComponentTypeMethods) {
            .updateTick = HandlePlayerInputUpdate,
        });
}