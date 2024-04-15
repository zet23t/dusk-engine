#include "../game_g.h"
#include <string.h>

typedef struct TweenTargetMapping {
    const char *name;
    void (*tween)(SceneObject *sceneObject, SceneComponentId SceneComponentId, TweenComponent* tweenComponent, float p);
} TweenTargetMapping;

static void _tweenNop(SceneObject *sceneObject, SceneComponentId SceneComponentId, TweenComponent* tweenComponent, float p)
{
    TraceLog(LOG_WARNING, "IMPLEMENT ME");
}

static void _tweenPosition(SceneObject *sceneObject, SceneComponentId SceneComponentId, TweenComponent* tweenComponent, float p)
{
    Vector3 pos = Vector3Lerp(tweenComponent->startVec3, tweenComponent->endVec3, p);
    SceneGraph_setLocalPosition(sceneObject->graph, sceneObject->id, pos);
}

static const TweenTargetMapping _tweenTargetMap[] = {
    {TWEEN_LOCAL_POSITION, _tweenPosition},
    {TWEEN_LOCAL_POSITION_X, _tweenNop},
    {TWEEN_LOCAL_POSITION_Y, _tweenNop},
    {TWEEN_LOCAL_POSITION_Z, _tweenNop},
    {TWEEN_LOCAL_ROTATION, _tweenNop},
    {TWEEN_LOCAL_ROTATION_X, _tweenNop},
    {TWEEN_LOCAL_ROTATION_Y, _tweenNop},
    {TWEEN_LOCAL_ROTATION_Z, _tweenNop},
    {TWEEN_LOCAL_SCALE, _tweenNop},
    {TWEEN_LOCAL_SCALE_X, _tweenNop},
    {TWEEN_LOCAL_SCALE_Y, _tweenNop},
    {TWEEN_LOCAL_SCALE_Z, _tweenNop},
    {NULL, _tweenNop}
};
static void TweenComponent_initialize(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void* arg)
{
    TweenComponent *src = (TweenComponent*) arg;
    TweenComponent *dst = (TweenComponent*) componentData;
    memcpy(dst, src, sizeof(*src));
    dst->targetName = strdup(src->targetName);
}

static void TweenComponent_onDestroy(SceneObject* obj, SceneComponentId sceneComponentId, void* componentData)
{
    TweenComponent *tweenComponent = (TweenComponent*) componentData;
    free(tweenComponent->targetName);
}

static void TweenComponent_update(SceneObject* node, SceneComponentId id, float dt, void *componentData)
{
    if (dt <= 0.0f) return;
    TweenComponent *tweenComponent = (TweenComponent*) componentData;
    float t = tweenComponent->time / tweenComponent->maxTime;
    tweenComponent->time += dt;
    if (t > 1.0f) t = 1.0f;
    if (tweenComponent->targetIsObject)
    {
        for (int i = 0; _tweenTargetMap[i].name; i++)
        {
            if (strcmp(_tweenTargetMap[i].name, tweenComponent->targetName))
            {
                _tweenTargetMap[i].tween(node, id, tweenComponent, t);
                break;
            }
        }
    }
    if (t == 1.0f)
    {
        SceneGraph_destroyComponent(node->graph, id);
    }
}
