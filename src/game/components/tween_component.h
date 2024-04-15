#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(TweenComponent)
#elif defined(COMPONENT_DECLARATION)

#define TWEEN_LOCAL_POSITION "local_position"
#define TWEEN_LOCAL_POSITION_X "local_position_x"
#define TWEEN_LOCAL_POSITION_Y "local_position_y"
#define TWEEN_LOCAL_POSITION_Z "local_position_z"
#define TWEEN_LOCAL_ROTATION "local_rotation"
#define TWEEN_LOCAL_ROTATION_X "local_rotation_x"
#define TWEEN_LOCAL_ROTATION_Y "local_rotation_y"
#define TWEEN_LOCAL_ROTATION_Z "local_rotation_z"
#define TWEEN_LOCAL_SCALE "local_scale"
#define TWEEN_LOCAL_SCALE_X "local_scale_x"
#define TWEEN_LOCAL_SCALE_Y "local_scale_y"
#define TWEEN_LOCAL_SCALE_Z "local_scale_z"

typedef struct TweenComponent {
    float time;
    float maxTime;
    uint8_t transitionFunctionType;
    uint8_t targetIsObject;
    char *targetName;

    union {
        SceneComponentId componentId;
        SceneObjectId objectId;
    };

    union {
        Vector3 startVec3;
        float startFloat;
        Color startColor;
    };
    union 
    {
        Vector3 endVec3;
        float endFloat;
        Color endColor;
    };
} TweenComponent;

#else

#include "tween_component.c"

BEGIN_COMPONENT_REGISTRATION(TweenComponent) {
    .updateTick = TweenComponent_update,
    .onDestroy = TweenComponent_onDestroy,
    .onInitialize = TweenComponent_initialize,
} END_COMPONENT_REGISTRATION

#endif