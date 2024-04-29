#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(ShootingComponent)
#elif defined(COMPONENT_DECLARATION)

SERIALIZABLE_STRUCT_START(ShootingConfig)
SERIALIZABLE_FIELD(float, shootInterval)
SERIALIZABLE_FIELD(float, bulletSpeed)
SERIALIZABLE_FIELD(float, bulletLifetime)
SERIALIZABLE_FIELD(int, bulletMask)
NONSERIALIZED_FIELD(void, (*onShoot)(SceneGraph*, SceneComponentId shooter, ShootingComponent*, ShootingConfig* shootingComponent))
SERIALIZABLE_STRUCT_END(ShootingConfig)

SERIALIZABLE_STRUCT_START(ShootingComponent)
SERIALIZABLE_FIELD(float, cooldown)
SERIALIZABLE_FIELD(int8_t, shooting)
SERIALIZABLE_FIELD(ShootingConfig, config)
SERIALIZABLE_FIELD(SceneObjectId, spawnPoint)
SERIALIZABLE_STRUCT_END(ShootingComponent)

#elif defined(COMPONENT_IMPLEMENTATION)

#include "shooting_component.c"

BEGIN_COMPONENT_REGISTRATION(ShootingComponent) {
    .updateTick = ShootingComponentUpdate,
} END_COMPONENT_REGISTRATION

#endif