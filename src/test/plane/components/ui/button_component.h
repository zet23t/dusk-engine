#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(ButtonComponent)
#elif defined(COMPONENT_DECLARATION)

typedef struct ButtonComponent {
    SceneComponentId clickZoneComponentId;
    SceneComponentId spriteRendererComponentId;

} ButtonComponent;

#else

#include "button_component.c"

BEGIN_COMPONENT_REGISTRATION(ButtonComponent) {
    .updateTick = Button_update,
} END_COMPONENT_REGISTRATION

#endif