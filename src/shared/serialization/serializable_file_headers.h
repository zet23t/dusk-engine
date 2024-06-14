#include "define_serialization_macros.h"

#include "shared/scene_graph/scene_graph_serializables.h"

#define COMPONENT_DECLARATION
#ifdef TESTS
#include "tests/test_structs.h"
#else
#include "game/component_list.h"
#endif
#undef COMPONENT_DECLARATION

#include "undef_serialization_macros.h"