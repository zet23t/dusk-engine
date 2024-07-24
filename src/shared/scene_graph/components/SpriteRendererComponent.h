#ifdef COMPONENT

COMPONENT(SpriteRendererComponent)

#elif defined(SERIALIZABLE_STRUCT_START)


SERIALIZABLE_STRUCT_START(SpriteAsset)
    NONSERIALIZED_FIELD(Texture2D, texture)
    SERIALIZABLE_FIELD(Rectangle, source)
    SERIALIZABLE_FIELD(Vector4, scale9frame)
SERIALIZABLE_STRUCT_END(SpriteAsset)

SERIALIZABLE_STRUCT_START(SpriteRendererComponent)
    SERIALIZABLE_FIELD(SpriteAsset, spriteAsset)
    SERIALIZABLE_FIELD(int, sortId)
    SERIALIZABLE_FIELD(int, previousSortIndex)
    SERIALIZABLE_FIELD(Color, tint)
    SERIALIZABLE_FIELD(Vector2, pivot)
    SERIALIZABLE_FIELD(float, pixelsPerUnit)
    SERIALIZABLE_FIELD(float, snapping)
    SERIALIZABLE_FIELD(Vector2, size)
    SERIALIZABLE_FIELD(Vector2, flip)
SERIALIZABLE_STRUCT_END(SpriteRendererComponent)

#elif defined(COMPONENT_IMPLEMENTATION)

#include "SpriteRendererComponent.c"

#endif