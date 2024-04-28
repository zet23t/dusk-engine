#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(ObjectConfiguratorEditorComponent)
#elif defined(COMPONENT_DECLARATION)

SERIALIZABLE_STRUCT_START(ObjectConfiguratorEditorComponent)
    SERIALIZABLE_FIELD(SceneObjectId, cameraPivotYawId)
    SERIALIZABLE_FIELD(SceneObjectId, cameraPivotPitchId)
    SERIALIZABLE_FIELD(SceneObjectId, selectedObjectId)
SERIALIZABLE_STRUCT_END(ObjectConfiguratorEditorComponent)

#elif defined(COMPONENT_IMPLEMENTATION)

#include "object_configurator_editor.c"
#include "../../util/component_macros.h"

BEGIN_COMPONENT_REGISTRATION(ObjectConfiguratorEditorComponent) {
    .onInitialize = ObjectConfiguratorEditorComponent_init,
    .updateTick = ObjectConfiguratorEditorComponent_update,
    .draw = ObjectConfiguratorEditorComponent_draw,
    .draw2D = ObjectConfiguratorEditorComponent_draw2D,
} END_COMPONENT_REGISTRATION

#endif