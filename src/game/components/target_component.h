#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(TargetComponent)
#elif defined(COMPONENT_DECLARATION)

SERIALIZABLE_STRUCT_START(TargetComponent)
SERIALIZABLE_FIELD(float, radius)
SERIALIZABLE_FIELD(uint32_t, colliderMask)
NONSERIALIZED_FIELD(int, (*onHit)(SceneGraph*, SceneObjectId target, SceneObjectId bullet))
SERIALIZABLE_STRUCT_END(TargetComponent)

#elif defined(COMPONENT_IMPLEMENTATION)

#include "target_component.c"

BEGIN_COMPONENT_REGISTRATION(TargetComponent)
{
    .updateTick = TargetComponentUpdate,
#if defined(DEBUG)
    .draw = TargetComponent_draw,
#endif
}
END_COMPONENT_REGISTRATION

#endif