#include "../plane_sim_g.h"

void HandlePlayerInputUpdate()
{
    if (psg.deltaTime == 0) return;
    Vector3 position = SceneGraph_getLocalPosition(psg.sceneGraph, psg.playerPlane);
    LinearVelocityComponent *velocity;
    SceneGraph_getComponentByType(psg.sceneGraph, psg.playerPlane, psg.linearVelocityComponentId, (void**)&velocity, 0);
    if (velocity == NULL) return;

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
    if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        for (int i=0;1;i++)
        {
            ShootingComponent *shooting;
            SceneGraph_getComponentByType(psg.sceneGraph, psg.playerPlane, psg.shootingComponentId, (void**)&shooting, i);
            if (shooting == NULL) break;
            shooting->shooting = true;
        }
    }

    const float movementRangeX = 5.0f;
    const float movementRangeZ = 5.0f;
    if (position.x > movementRangeX)
    {
        velocity->velocity.x = velocity->velocity.x * .5f - (position.x - movementRangeX) * 1.0f;
    }
    else if (position.x < -movementRangeX)
    {
        velocity->velocity.x = velocity->velocity.x * .5f - (position.x + movementRangeX) * 1.0f;
    }
    if (position.z > movementRangeZ)
    {
        velocity->velocity.z = velocity->velocity.z * .5f - (position.z - movementRangeZ) * 1.0f;
    }
    else if (position.z < -movementRangeZ)
    {
        velocity->velocity.z = velocity->velocity.z * .5f - (position.z + movementRangeZ) * 1.0f;
    }

    Vector3 rotation = SceneGraph_getLocalRotation(psg.sceneGraph, psg.playerPlane);
    rotation.z += -velocity->velocity.x * 1.0f;
    SceneGraph_setLocalRotation(psg.sceneGraph, psg.playerPlane, rotation);
    // SceneGraph_setLocalRotation(psg.sceneGraph, plane, (Vector3) { 0, t * 30, 0 });

}