#include <stdlib.h>
#include <stdio.h>

#include "external/cjson.c"
#include "shared/scene_graph/scene_graph.c"
#include "shared/touch_util.c"
#include "test/plane/messagehub.c"
#include "test/plane/util/util_math.c"
#include "test/plane/util/prefab.c"
#include "test/plane/util/actions.c"
#include "test/plane/util/mapped_variables.c"
#include "test/plane/resource_manager.c"
#include "test/plane/components/bullet_component.c"
#include "test/plane/components/camera_component.c"
#include "test/plane/components/health_component.c"
#include "test/plane/components/auto_destroy_component.c"
#include "test/plane/components/update_callback_component.c"
#include "test/plane/components/velocity_component.c"
#include "test/plane/components/shooting_component.c"
#include "test/plane/components/target_component.c"
#include "test/plane/components/enemy_plane_behavior_component.c"
#include "test/plane/components/movement_pattern_component.c"
#include "test/plane/components/target_handler_component.c"
#include "test/plane/components/text_component.c"
#include "test/plane/components/timer_component.c"
#include "test/plane/components/action_component.c"
#include "test/plane/components/trail_renderer_component.c"
#include "test/plane/components/enemybehavior_component.c"

#define COMPONENT_IMPLEMENTATION
#include "test/plane/component_list.h"
#undef COMPONENT_IMPLEMENTATION

#include "test/plane/system/ground_tile_system.c"
#include "test/plane/system/player_input_handler.c"
#include "test/plane/system/target_spawn_system.c"
#include "test/plane/system/cloud_system.c"
#include "test/plane/system/level_system.c"
#include "test/plane/system/game_ui.c"
#include "test/plane/plane_sim_g.c"
#include "test/plane/plane_sim.c"
#include "test/plane/game_state_level.c"

#if PLATFORM_WEB
#define DLL_EXPORT
#else
#define STB_PERLIN_IMPLEMENTATION
#include "test/plane/util/stb_perlin.h"
#define DLL_EXPORT __declspec(dllexport)
#endif

DLL_EXPORT void InitializeGameCode(void *storedState)
{
    if (storedState)
    {
        memcpy(&psg, storedState, sizeof(PSG));
        free(storedState);
    }
    if (plane_sim_init()) {
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
    plane_sim_draw();
}

DLL_EXPORT void GameCodeUpdate(float dt)
{
    plane_sim_update(dt);
}
