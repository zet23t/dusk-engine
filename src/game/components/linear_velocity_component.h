#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(LinearVelocityComponent)
#elif defined(COMPONENT_DECLARATION)

SERIALIZABLE_STRUCT_START(LinearVelocityComponent)
    SERIALIZABLE_FIELD(Vector3, velocity)
    SERIALIZABLE_FIELD(Vector3, acceleration)
    SERIALIZABLE_FIELD(Vector3, drag)
SERIALIZABLE_STRUCT_END(LinearVelocityComponent)

#elif defined(COMPONENT_IMPLEMENTATION)

#include "linear_velocity_component.c"

BEGIN_COMPONENT_REGISTRATION(LinearVelocityComponent) {
    .updateTick = LinearVelocityComponentUpdateTick,
    .onInitialize = LinearVelocityComponentInitialize,
} END_COMPONENT_REGISTRATION

#endif