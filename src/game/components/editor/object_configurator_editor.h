#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(ObjectConfiguratorEditorComponent)
#elif defined(COMPONENT_DECLARATION)

typedef struct ObjectConfiguratorEditorComponent {
    SceneObjectId cameraPivotYawId;
    SceneObjectId cameraPivotPitchId;
} ObjectConfiguratorEditorComponent;

#else

#include "object_configurator_editor.c"
#include "../../util/component_macros.h"

BEGIN_COMPONENT_REGISTRATION(ObjectConfiguratorEditorComponent) {
    .onInitialize = ObjectConfiguratorEditorComponent_init,
    .updateTick = ObjectConfiguratorEditorComponent_update,
    .draw = ObjectConfiguratorEditorComponent_draw,
    .draw2D = ObjectConfiguratorEditorComponent_draw2D,
} END_COMPONENT_REGISTRATION

#endif