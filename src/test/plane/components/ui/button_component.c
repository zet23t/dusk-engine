#include "../../plane_sim_g.h"

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
    TextComponent *textComponent;
    if (!SceneGraph_getComponent(node->graph, buttonComponent->textComponentId, (void*) &textComponent))
    {
        return;
    }
    int flags = clickZoneComponent->flags;
    int isActive = flags & CLICKZONECOMPONENT_FLAG_ACTIVE;
    int isHovered = flags & CLICKZONECOMPONENT_FLAG_HOVER;
    if (isActive && isHovered)
    {
        spriteRendererComponent->spriteAsset = buttonComponent->pressedState.spriteAsset;
        textComponent->offset = buttonComponent->pressedState.textOffset;
    }
    else if (isActive || isHovered)
    {
        spriteRendererComponent->spriteAsset = buttonComponent->hoverState.spriteAsset;
        textComponent->offset = buttonComponent->hoverState.textOffset;
    }
    else
    {
        spriteRendererComponent->spriteAsset = buttonComponent->normalState.spriteAsset;
        textComponent->offset = buttonComponent->normalState.textOffset;
    }
}