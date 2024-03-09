#include "../plane_sim_g.h"

void TweenComponentRegister()
{
    psg.tweenComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "Tween", sizeof(TweenComponent),
        (SceneComponentTypeMethods) {
            .onInitialize = TweenComponent_onInitialize,
            .onDestroy = TweenComponent_onDestroy,
            .onUpdate = TweenComponent_onUpdate,
        });
}