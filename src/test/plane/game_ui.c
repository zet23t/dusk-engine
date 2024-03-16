#include "plane_sim_g.h"

#include <stdio.h>

void GameUi_Init()
{
    SceneObjectId ui = SceneGraph_createObject(psg.sceneGraph, "ui");
    AddMeshRendererComponentByName(ui, "ui-border", 1.0f);
    SceneGraph_setLocalPosition(psg.sceneGraph, ui, (Vector3) { 0, 0, 0 });
}