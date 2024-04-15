#include "../../game_g.h"

static void ButtonComponent_update(SceneObject* node, SceneComponentId id, float dt, void *componentData)
{
    ButtonComponent *buttonComponent = (ButtonComponent *)componentData;
    SpriteRendererComponent *spriteRendererComponent;
    if (!SceneGraph_getComponent(node->graph, buttonComponent->spriteRendererComponentId, (void*) &spriteRendererComponent))
    {
        return;
    }
    ClickZoneComponent *clickZoneComponent;
    if (!SceneGraph_getComponent(node->graph, buttonComponent->clickZoneComponentId, (void*) &clickZoneComponent))
    {
        return;
    }
    int flags = clickZoneComponent->flags;
    int isActive = flags & CLICKZONECOMPONENT_FLAG_ACTIVE;
    int isHovered = flags & CLICKZONECOMPONENT_FLAG_HOVER;
    Vector2 offset;
    if (isActive && isHovered)
    {
        spriteRendererComponent->spriteAsset = buttonComponent->pressedState.spriteAsset;
        offset = buttonComponent->pressedState.contentOffset;
    }
    else if (isActive || isHovered)
    {
        spriteRendererComponent->spriteAsset = buttonComponent->hoverState.spriteAsset;
        offset = buttonComponent->hoverState.contentOffset;
    }
    else
    {
        spriteRendererComponent->spriteAsset = buttonComponent->normalState.spriteAsset;
        offset = buttonComponent->normalState.contentOffset;
    }
    SceneGraph_setLocalPosition(node->graph, buttonComponent->contentId, (Vector3) { offset.x, offset.y, 0 });
}