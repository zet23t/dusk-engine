#include "../game_g.h"

void BulletComponentRegister()
{
    psg.bulletComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "Bullet", sizeof(BulletComponent),
        (SceneComponentTypeMethods) {0});
}