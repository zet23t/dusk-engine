#include "external/cjson.c"
#include "shared/ecs/pico_ecs.c"
#include "shared/scene_graph/scene_graph.c"
#include "test/plane/util/util_math.c"
#include "test/plane/util/prefab.c"
#include "test/plane/util/actions.c"
#include "test/plane/util/mapped_variables.c"
#include "test/plane/components/bullet_component.c"
#include "test/plane/components/camera_component.c"
#include "test/plane/components/health_component.c"
#include "test/plane/components/auto_destroy_component.c"
#include "test/plane/components/update_callback_component.c"
#include "test/plane/components/mesh_renderer_component.c"
#include "test/plane/components/velocity_component.c"
#include "test/plane/components/plane_behavior_component.c"
#include "test/plane/components/shooting_component.c"
#include "test/plane/components/target_component.c"
#include "test/plane/components/enemy_plane_behavior_component.c"
#include "test/plane/components/movement_pattern_component.c"
#include "test/plane/components/target_handler_component.c"
#include "test/plane/components/text_component.c"
#include "test/plane/components/timer_component.c"
#include "test/plane/components/action_component.c"
#include "test/plane/components/trail_renderer_component.c"
#include "test/plane/system/ground_tile_system.c"
#include "test/plane/system/player_input_handler.c"
#include "test/plane/system/target_spawn_system.c"
#include "test/plane/system/cloud_system.c"
#include "test/plane/system/level_system.c"
#include "test/plane/plane_sim_g.c"
#include "test/plane/plane_sim.c"
#include "dusk-engine.c"