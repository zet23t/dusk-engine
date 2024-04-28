#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(ClickZoneComponent)
#elif defined(COMPONENT_DECLARATION)

#ifndef CLICKZONECOMPONENT_FLAG_NONE
#define CLICKZONECOMPONENT_FLAG_NONE 0
#define CLICKZONECOMPONENT_FLAG_HOVER 1
#define CLICKZONECOMPONENT_FLAG_ACTIVE 2
#endif

SERIALIZABLE_STRUCT_START(ClickZoneComponent)
    SERIALIZABLE_FIELD(Vector3, boxSize)
    SERIALIZABLE_FIELD(int, zoneId)
    NONSERIALIZED_FIELD(int16_t, flags)
    NONSERIALIZED_FIELD(int16_t, mouseButtonLock)
    NONSERIALIZED_FIELD(int, touchIdLock)
SERIALIZABLE_STRUCT_END(ClickZoneComponent)

#else

#include "click_zone_component.c"

BEGIN_COMPONENT_REGISTRATION(ClickZoneComponent) {
    .updateTick = ClickZone_update,
} END_COMPONENT_REGISTRATION

#endif