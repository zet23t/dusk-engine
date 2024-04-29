#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(HealthComponent)
#elif defined(COMPONENT_DECLARATION)

SERIALIZABLE_STRUCT_START(HealthComponent)
    SERIALIZABLE_FIELD(float, health)
    SERIALIZABLE_FIELD(float, maxHealth)
SERIALIZABLE_STRUCT_END(HealthComponent)


#elif defined(COMPONENT_IMPLEMENTATION)

#include "health_component.c"

BEGIN_COMPONENT_REGISTRATION(HealthComponent) {
    .updateTick = HealthComponentUpdate,
    .onInitialize = HealthComponentInitialize,
} END_COMPONENT_REGISTRATION

#endif