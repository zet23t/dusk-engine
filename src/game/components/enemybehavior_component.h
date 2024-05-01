#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(EnemyBehaviorComponent)
#elif defined(COMPONENT_DECLARATION)

SERIALIZABLE_STRUCT_START(EnemyBehaviorComponent)
SERIALIZABLE_FIELD(int32_t, behaviorType)
SERIALIZABLE_FIELD(int32_t, phase)
SERIALIZABLE_FIELD(float, time)
SERIALIZABLE_FIELD(float, velocity)
SERIALIZABLE_FIELD(float, agility)
SERIALIZABLE_FIXED_ARRAY(Vector3, points, 4)
SERIALIZABLE_FIELD(int8_t, pointCount)
SERIALIZABLE_FIELD(int8_t, initialized)
SERIALIZABLE_FIELD(int8_t, autoDestroy)
SERIALIZABLE_FIELD(SceneComponentId, velocityComponentId)
SERIALIZABLE_STRUCT_END(EnemyBehaviorComponent)

#elif defined(COMPONENT_IMPLEMENTATION)

#include "enemybehavior_component.c"

BEGIN_COMPONENT_REGISTRATION(EnemyBehaviorComponent) {
    .updateTick = EnemyBehaviorComponent_onUpdate,
} END_COMPONENT_REGISTRATION

#endif