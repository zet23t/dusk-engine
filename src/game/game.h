#ifndef __game_H__
#define __game_H__

#include "raylib.h"
#include "RuntimeContext.h"
#include "shared/resource_manager.h"
#include "shared/scene_graph/scene_graph.h"

typedef struct ComponentIdMap {
#define COMPONENT(T) SceneComponentTypeId T##Id;
#include "game/component_list.h"
#undef COMPONENT
} ComponentIdMap;

extern ComponentIdMap _componentIdMap;
extern ResourceManager* _resourceManager;

#define BEGIN_COMPONENT_REGISTRATION(T) void T##_register(SceneGraph *g) { _componentIdMap.T##Id = \
    SceneGraph_registerComponentType(g, #T, sizeof(T), (SceneComponentTypeMethods)
#define END_COMPONENT_REGISTRATION );}


int game_init();
void game_draw();
void game_update(float dt);

#endif // __game_H__