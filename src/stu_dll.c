#include <stdlib.h>
#include <stdio.h>

#include "external/cjson.c"
#include "shared/scene_graph/scene_graph.c"
#include "shared/touch_util.c"
#include "game/messagehub.c"
#include "game/util/util_math.c"
#include "game/util/prefab.c"
#include "game/util/mapped_variables.c"
#include "game/resource_manager.c"
#include "game/components/bullet_component.c"
#include "game/components/camera_component.c"
#include "game/components/auto_destroy_component.c"
#include "game/components/update_callback_component.c"
#include "game/components/movement_pattern_component.c"
#include "game/components/target_handler_component.c"
#include "game/components/text_component.c"
#include "game/components/trail_renderer_component.c"
#include "game/components/enemybehavior_component.c"

#define COMPONENT_IMPLEMENTATION
#include "game/component_list.h"
#undef COMPONENT_IMPLEMENTATION

#include "game/system/ground_tile_system.c"
#include "game/system/player_input_handler.c"
#include "game/system/target_spawn_system.c"
#include "game/system/cloud_system.c"
#include "game/system/level_system.c"
#include "game/system/game_ui.c"
#include "game/game_g.c"
#include "game/game.c"
#include "game/editor.c"
#include "game/game_state_level.c"
#include "shared/serialization/serializers.c"

#if PLATFORM_WEB
#define DLL_EXPORT
#else
#define STB_PERLIN_IMPLEMENTATION
#include "game/util/stb_perlin.h"
#define DLL_EXPORT __declspec(dllexport)
#endif

DLL_EXPORT void InitializeGameCode(void *storedState)
{
    if (storedState)
    {
        memcpy(&psg, storedState, sizeof(PSG));
        free(storedState);
    }
    if (game_init()) {
        printf("InitializeGameCode failed\n");
        return;
    }

    printf("InitializeGameCode successfully\n");
}

DLL_EXPORT void* UnloadGameCode()
{
    printf("UnloadGameCode successfully\n");
    PSG* psgCopy = malloc(sizeof(PSG));
    memcpy(psgCopy, &psg, sizeof(PSG));
    return psgCopy;
}

DLL_EXPORT void GameCodeDraw()
{
    game_draw();
}

DLL_EXPORT void GameCodeUpdate(float dt)
{
    game_update(dt);
}
