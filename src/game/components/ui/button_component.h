#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(ButtonComponent)
#elif defined(COMPONENT_DECLARATION)
typedef struct ButtonState {
    SpriteAsset spriteAsset;
    Vector2 contentOffset;
} ButtonState;

typedef struct ButtonComponent {
    SceneComponentId clickZoneComponentId;
    SceneComponentId spriteRendererComponentId;
    SceneObjectId contentId;
    ButtonState normalState;
    ButtonState hoverState;
    ButtonState pressedState;
} ButtonComponent;


#else

#include "button_component.c"

BEGIN_COMPONENT_REGISTRATION(ButtonComponent) {
    .updateTick = ButtonComponent_update,
} END_COMPONENT_REGISTRATION

#endif