#include <stdlib.h>
#include <stdio.h>

#include "RuntimeContext.h"
#include "external/cjson.c"
#include "shared/scene_graph/scene_graph.c"
#include "shared/touch_util.c"
#include "shared/resource_manager.c"
#include "shared/ui/dusk-gui.c"
#include "shared/serialization/reflection.c"
#include "shared/serialization/serializable_structs.h"
#include "shared/serialization/serializers.c"
#include "shared/editor/editor_styles.c"
#include "shared/editor/editor.c"
#include "shared/util/arena.c"


#define COMPONENT_IMPLEMENTATION
#include "game/component_list.h"
#undef COMPONENT_IMPLEMENTATION

#include "game/game.c"

#if PLATFORM_WEB
#define DLL_EXPORT
#else
#define DLL_EXPORT __declspec(dllexport)
#endif

DLL_EXPORT void InitializeGameCode(RuntimeContext *runtimeContext)
{
    if (game_init(runtimeContext)) {
        printf("InitializeGameCode failed\n");
        return;
    }

    printf("InitializeGameCode successfully\n");
}

DLL_EXPORT void UnloadGameCode()
{
    printf("UnloadGameCode successfully\n");
    
}

DLL_EXPORT void GameCodeDraw()
{
    game_draw();
}

DLL_EXPORT void GameCodeUpdate(float dt)
{
    game_update(dt);
}
