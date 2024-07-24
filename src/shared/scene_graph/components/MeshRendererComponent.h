#ifdef COMPONENT

COMPONENT(MeshRendererComponent)

#elif defined(SERIALIZABLE_STRUCT_START)

SERIALIZABLE_STRUCT_START(MeshRendererComponent)
    SERIALIZABLE_ANNOTATION(Range, Vector2, (&(Vector2){0.0f, 1.0f}))
    SERIALIZABLE_FIELD(float, litAmount)
    NONSERIALIZED_FIELD(Material*, material)
    NONSERIALIZED_FIELD(Mesh*, mesh)
SERIALIZABLE_STRUCT_END(MeshRendererComponent)

#elif defined(COMPONENT_IMPLEMENTATION)

#include "MeshRendererComponent.c"

#endif