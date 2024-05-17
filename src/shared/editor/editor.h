#ifndef __EDITOR_STATE_H__
#define __EDITOR_STATE_H__

#include "../scene_graph/scene_graph.h"

typedef struct EditorState
{
    float gameTime;
    int frameCount;
    bool displayHierarchy;
    bool displayInspector;
    bool paused;
    bool stepped;
    SceneObjectId selectedObjectId;
    Rectangle hierarchyPanelBounds;
    Rectangle inspectorPanelBounds;
} EditorState;

void Editor_drawControls(EditorState* state, SceneGraph *graph);
void Editor_draw(EditorState* state, SceneGraph *graph);

#endif