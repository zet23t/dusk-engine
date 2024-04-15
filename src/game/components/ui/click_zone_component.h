#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(ClickZoneComponent)
#elif defined(COMPONENT_DECLARATION)

#define CLICKZONECOMPONENT_FLAG_NONE 0
#define CLICKZONECOMPONENT_FLAG_HOVER 1
#define CLICKZONECOMPONENT_FLAG_ACTIVE 2

typedef struct ClickZoneComponent {
    Vector3 boxSize;
    int zoneId;
    int16_t flags;
    int16_t mouseButtonLock;
    int touchIdLock;
} ClickZoneComponent;

#else

#include "click_zone_component.c"

BEGIN_COMPONENT_REGISTRATION(ClickZoneComponent) {
    .updateTick = ClickZone_update,
} END_COMPONENT_REGISTRATION

#endif