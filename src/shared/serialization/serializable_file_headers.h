#include "define_serialization_macros.h"

#include "shared/scene_graph/scene_graph_serializables.h"

#define COMPONENT_DECLARATION
#include "game/component_list.h"
#undef COMPONENT_DECLARATION

#include "undef_serialization_macros.h"