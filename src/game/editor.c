#include "game_g.h"
#include "editor.h"

void Editor_init()
{
    SceneGraph_clear(psg.sceneGraph);
    SceneObjectId system = SceneGraph_createObject(psg.sceneGraph, "System");
    SceneGraph_addComponent(psg.sceneGraph, system, psg.ObjectConfiguratorEditorComponentId, &(ObjectConfiguratorEditorComponent) {
        0
    });
}