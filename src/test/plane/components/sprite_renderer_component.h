#undef COMPONENT_NAME
#define COMPONENT_NAME SpriteRendererComponent

#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(SpriteRendererComponent)
#elif defined(COMPONENT_DECLARATION)
typedef struct SpriteRendererComponent {
    Texture2D texture;
    Rectangle source;
    Vector4 scale9frame;
    Color tint;
    Vector2 pivot;
    float pixelsPerUnit;
    Vector2 size;
} SpriteRendererComponent;
#endif

#ifdef COMPONENT_IMPLEMENTATION
// === COMPONENT IMPLEMENTATION ===
#include "sprite_renderer_component.c"
#include "../util/component_macros.h"
BEGIN_COMPONENT_REGISTRATION(SpriteRendererComponent) {
    .sequentialDraw = SpriteRendererComponent_sequentialDraw,
} END_COMPONENT_REGISTRATION

#endif