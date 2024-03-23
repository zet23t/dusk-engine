#ifdef MESSAGE_TYPE
MESSAGE_TYPE(ClickZoneMessage)
#endif

#ifdef DECLARATION
#include "shared/scene_graph/scene_graph.h"
#define CLICK_ZONE_MESSAGE_FLAG_ENTER 1
#define CLICK_ZONE_MESSAGE_FLAG_EXIT 2
#define CLICK_ZONE_MESSAGE_FLAG_CLICK 4
#define CLICK_ZONE_MESSAGE_FLAG_POINTER_WAS_PRESSED 8
#define CLICK_ZONE_MESSAGE_FLAG_POINTER_WAS_RELEASED 16
#define CLICK_ZONE_MESSAGE_FLAG_POINTER_IS_DOWN 32
typedef struct ClickZoneMessage {
    int flags;
    SceneComponentId buttonComponentId;
    int zoneId;
} ClickZoneMessage;
#endif