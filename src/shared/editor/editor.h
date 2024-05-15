#ifndef __EDITOR_STATE_H__
#define __EDITOR_STATE_H__

#include "../scene_graph/scene_graph.h"

typedef struct EditorState
{
    bool displayHierarchy;
    bool displayInspector;
    SceneObjectId selectedObjectId;
    Rectangle hierarchyPanelBounds;
    Rectangle inspectorPanelBounds;
} EditorState;

void Editor_draw(EditorState* state, SceneGraph *graph);

#endif