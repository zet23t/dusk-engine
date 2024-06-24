#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(SpriteRendererComponent)
#elif defined(COMPONENT_DECLARATION)

SERIALIZABLE_STRUCT_START(SpriteAsset)
    NONSERIALIZED_FIELD(Texture2D, texture)
    NONSERIALIZED_FIELD(Rectangle, source)
    NONSERIALIZED_FIELD(Vector4, scale9frame)
SERIALIZABLE_STRUCT_END(SpriteAsset)

SERIALIZABLE_STRUCT_START(SpriteRendererComponent)
    SERIALIZABLE_FIELD(SpriteAsset, spriteAsset)
    SERIALIZABLE_FIELD(int, sortId)
    SERIALIZABLE_FIELD(int, previousSortIndex)
    SERIALIZABLE_FIELD(Color, tint)
    SERIALIZABLE_FIELD(Vector2, pivot)
    SERIALIZABLE_FIELD(float, snapping)
    SERIALIZABLE_FIELD(float, pixelsPerUnit)
    SERIALIZABLE_FIELD(Vector2, size)
    SERIALIZABLE_FIELD(Vector2, flip)
SERIALIZABLE_STRUCT_END(SpriteRendererComponent)
#elif defined(COMPONENT_IMPLEMENTATION)

// === COMPONENT IMPLEMENTATION ===
#include "sprite_renderer_component.c"
#include "../util/component_macros.h"
BEGIN_COMPONENT_REGISTRATION(SpriteRendererComponent) {
    .sequentialDraw = SpriteRendererComponent_sequentialDraw,
} END_COMPONENT_REGISTRATION

#endif