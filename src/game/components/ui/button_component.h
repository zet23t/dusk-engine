#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(ButtonComponent)
#elif defined(COMPONENT_DECLARATION)
SERIALIZABLE_STRUCT_START(ButtonState)
    NONSERIALIZED_FIELD(SpriteAsset, spriteAsset)
    SERIALIZABLE_FIELD(Vector2, contentOffset)
SERIALIZABLE_STRUCT_END(ButtonState)

SERIALIZABLE_STRUCT_START(ButtonComponent)
    SERIALIZABLE_FIELD(SceneComponentId, clickZoneComponentId)
    SERIALIZABLE_FIELD(SceneComponentId, spriteRendererComponentId)
    SERIALIZABLE_FIELD(SceneObjectId, contentId)
    SERIALIZABLE_FIELD(ButtonState, normalState)
    SERIALIZABLE_FIELD(ButtonState, hoverState)
    SERIALIZABLE_FIELD(ButtonState, pressedState)
SERIALIZABLE_STRUCT_END(ButtonComponent)


#else

#include "button_component.c"

BEGIN_COMPONENT_REGISTRATION(ButtonComponent) {
    .updateTick = ButtonComponent_update,
} END_COMPONENT_REGISTRATION

#endif