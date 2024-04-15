#include "../game_g.h"
#include "../game_state_level.h"

#include <stdio.h>

typedef struct GameUiSystem {
    float time;
    SceneObjectId uiRootId;
    SceneObjectId menuPanelId;
    SceneObjectId gameOverPanelId;
    SceneComponentId menuButtonId;
    SceneComponentId restartButtonId;
    SceneComponentId remainingDistanceTextId;
} GameUiSystem;

static int _gameUi_messageId;
static void GameUi_Update(SceneObject* sceneObject, SceneComponentId SceneComponent,
        float delta, void* componentData)
{
    GameUiSystem* gameUiSystem = (GameUiSystem*)componentData;
    while (1) {
        MessageHubMessage* msg = MessageHub_getMessage(&_gameUi_messageId);
        if (!msg)
            break;
        if (msg->messageTypeId == MessageId_ClickZoneMessage() && msg->dataClickZoneMessage.flags & CLICK_ZONE_MESSAGE_FLAG_CLICK) {
            if (SceneComponentIdEquals(msg->dataClickZoneMessage.buttonComponentId, gameUiSystem->menuButtonId)) {
                SceneGraph_setObjectEnabled(sceneObject->graph, gameUiSystem->menuPanelId, 
                    !SceneGraph_isObjectEnabled(sceneObject->graph, gameUiSystem->menuPanelId));
            }
            if (SceneComponentIdEquals(msg->dataClickZoneMessage.buttonComponentId, gameUiSystem->restartButtonId)) {
                GameStateLevel_Init();
            }
        }
    }

    if (!SceneGraph_getObject(sceneObject->graph, psg.playerPlane)) {
        SceneGraph_setObjectEnabled(sceneObject->graph, gameUiSystem->gameOverPanelId, 1);
    
    }

    float remainingDistance = 1000.0f - gameUiSystem->time;
    char text[64];
    char hitpoints[64];
    HealthComponent *health;
    if (SceneGraph_getComponentOrFindByType(sceneObject->graph, psg.playerPlane, NULL, psg.healthComponentId, (void**)&health)) {
        int cpos = 0;
        for (int i=0;i<health->maxHealth;i+=1)
        {
            if (i < health->health)
            {
                hitpoints[cpos++] = 0xc2;
                hitpoints[cpos++] = 0x80;
            }
            else
            {
                hitpoints[cpos++] = 0x7f;
            }
        }
        hitpoints[cpos++] = 0;
    } else {
        SceneGraph_setComponentEnabled(sceneObject->graph, gameUiSystem->remainingDistanceTextId, 0);
        return;
    }
    sprintf(text, "Remaining Distance: %.0f\nHitpoints: %s", remainingDistance, hitpoints);
    TextComponent_setText(sceneObject->graph, gameUiSystem->remainingDistanceTextId, text);

    gameUiSystem->time += delta;
}

static SceneObjectId AddPanel(SceneObjectId parentId, float x, float y, float width, float height, int colorRGBA)
{
    SceneObjectId panel = SceneGraph_createObject(psg.sceneGraph, "panel");
    SceneGraph_setParent(psg.sceneGraph, panel, parentId);
    SceneGraph_setLocalPosition(psg.sceneGraph, panel, (Vector3) { x, y, 0 });
    SceneGraph_addComponent(psg.sceneGraph, panel, psg.SpriteRendererComponentId,
        &(SpriteRendererComponent) {
            .spriteAsset = (SpriteAsset) {
                .texture = ResourceManager_loadTexture(&psg.resourceManager, "assets/ui_atlas.png", TEXTURE_FILTER_BILINEAR),
                .source = (Rectangle) { 0, 88, 82, 68 },
                .scale9frame = (Vector4) { 24, 24, 24, 30 },
            },
            .pivot = (Vector2) { 0.5f, 0.5f },
            .pixelsPerUnit = 40,
            .size = (Vector2) { width, height },
            .tint = (Color) { 
                colorRGBA>>24, 
                colorRGBA>>16&0xff, colorRGBA>>8&0xff, colorRGBA&0xff },
        });
    return panel;
}

static SceneComponentId AddButton(Font font, uint32_t buttonRGBA, SceneObjectId parentId, float x, float y, float w, float h, const char* text, SpriteAsset* icon)
{
    Color tint = (Color) {
        .r = (uint8_t)(buttonRGBA >> 24),
        .g = (uint8_t)((buttonRGBA >> 16) & 0xff),
        .b = (uint8_t)((buttonRGBA >> 8) & 0xff),
        .a = (uint8_t)(buttonRGBA & 0xff),
    };
    SceneObjectId clickableThing = SceneGraph_createObject(psg.sceneGraph, "clickable-thing");
    SceneGraph_setParent(psg.sceneGraph, clickableThing, parentId);
    SceneGraph_setLocalPosition(psg.sceneGraph, clickableThing, (Vector3) { x, y, 0 });
    ButtonComponent buttonComponent = (ButtonComponent) { 0 };
    SpriteAsset buttonNormal = (SpriteAsset) {
        .texture = ResourceManager_loadTexture(&psg.resourceManager, "assets/ui_atlas.png", TEXTURE_FILTER_BILINEAR),
        .source = (Rectangle) { 0, 0, 82, 80 },
        .scale9frame = (Vector4) { 24, 24, 24, 24 },
    };
    SpriteAsset buttonHover = (SpriteAsset) {
        .texture = ResourceManager_loadTexture(&psg.resourceManager, "assets/ui_atlas.png", TEXTURE_FILTER_BILINEAR),
        .source = (Rectangle) { 84, 0, 82, 80 },
        .scale9frame = (Vector4) { 24, 24, 24, 24 },
    };
    SpriteAsset buttonPressed = (SpriteAsset) {
        .texture = ResourceManager_loadTexture(&psg.resourceManager, "assets/ui_atlas.png", TEXTURE_FILTER_BILINEAR),
        .source = (Rectangle) { 168, 0, 82, 80 },
        .scale9frame = (Vector4) { 24, 24, 24, 24 },
    };
    float yOffset = 0.15f;
    buttonComponent.normalState.spriteAsset = buttonNormal;
    buttonComponent.normalState.contentOffset = (Vector2) { 0, 0 + yOffset };
    buttonComponent.hoverState.spriteAsset = buttonHover;
    buttonComponent.hoverState.contentOffset = (Vector2) { 0, 0 + yOffset };
    buttonComponent.pressedState.spriteAsset = buttonPressed;
    buttonComponent.pressedState.contentOffset = (Vector2) { 0.0f, -0.05f + yOffset };

    buttonComponent.clickZoneComponentId = SceneGraph_addComponent(psg.sceneGraph, clickableThing, psg.ClickZoneComponentId,
        &(ClickZoneComponent) { .boxSize = (Vector3) { w, h, 0 }, .zoneId = 0 });
    buttonComponent.spriteRendererComponentId = SceneGraph_addComponent(psg.sceneGraph, clickableThing, psg.SpriteRendererComponentId,
        &(SpriteRendererComponent) {
            .spriteAsset = buttonNormal,
            .pivot = (Vector2) { 0.5f, 0.5f },
            .pixelsPerUnit = 40,
            .size = (Vector2) { w, h },
            .tint = tint,
        });
    SceneObjectId contentId = SceneGraph_createObject(psg.sceneGraph, "content");
    SceneGraph_setParent(psg.sceneGraph, contentId, clickableThing);
    SceneGraph_setLocalPosition(psg.sceneGraph, contentId, (Vector3){0,0.0f,0});
    buttonComponent.contentId = contentId;
    SceneGraph_addComponent(psg.sceneGraph, contentId, psg.textComponentId,
        &(TextComponent) {
            .text = (char*)text,
            .font = font,
            .fontSize = 10,
            .fontSpacing = -.5f,
            .align = (Vector2) { 0.5f, 0.5f },
            .color = (Color) { 255, 255, 255, 255 },
        });
    SceneGraph_addComponent(psg.sceneGraph, clickableThing, psg.ButtonComponentId, &buttonComponent);

    if (icon)
    {
        SceneObjectId iconId = SceneGraph_createObject(psg.sceneGraph, "icon");
        SceneGraph_setParent(psg.sceneGraph, iconId, contentId);
        SceneGraph_setLocalPosition(psg.sceneGraph, iconId, (Vector3){0.0f, -0.1f, 0});
        SceneGraph_addComponent(psg.sceneGraph, iconId, psg.SpriteRendererComponentId, &(SpriteRendererComponent){
            .pixelsPerUnit = 10,
            .size = (Vector2){w * .8f, h * .8f},
            .tint = WHITE,
            .spriteAsset = *icon,
        });
    }

    return buttonComponent.clickZoneComponentId;
}

void GameUiSystem_onInitialize(SceneObject* sceneObject, SceneComponentId SceneComponent, void* componentData, void* initArg)
{
    GameUiSystem* gameUiSystem = (GameUiSystem*)componentData;
    memset(gameUiSystem, 0, sizeof(GameUiSystem));
    SceneObjectId uiPlaneId = SceneGraph_createObject(psg.sceneGraph, "ui-plane");
    psg.uiRootId = uiPlaneId;
    SceneGraph_setParent(psg.sceneGraph, uiPlaneId, psg.camera);
    SceneGraph_addComponent(psg.sceneGraph, uiPlaneId, psg.PrimitiveRendererComponentId,
        &(PrimitiveRendererComponent) {
            .primitiveType = PRIMITIVE_TYPE_CUBE,
            .size = (Vector3) { 15, 10, 0 },
            .color = (Color) { 255, 255, 255, 255 },
            .isWireframe = 1,
        });
    float uiDistance = 70.0f;
    SceneGraph_setLocalPosition(psg.sceneGraph, uiPlaneId, (Vector3) { 0, 0, uiDistance });
    SceneGraph_addComponent(psg.sceneGraph, uiPlaneId, psg.AspectRatioMatcherComponentId,
        &(AspectRatioMatcherComponent) {
            .canvasDistance = uiDistance,
            .width = 16.5f,
            .height = 10.5f,
        });

    // Vector3 planePos = SceneGraph_getWorldPosition(psg.sceneGraph, psg.playerPlane);
    // SceneGraph_setLocalPosition(psg.sceneGraph, uiPlaneId, (Vector3) { planePos.x, planePos.y, planePos.z });

    // printf("position: %f, %f, %f\n", planePos.x, planePos.y, planePos.z);
    for (int side = 0; side < 4; side++) {

        SceneObjectId uiBorderId = SceneGraph_createObject(psg.sceneGraph, "ui-border-side");
        SceneGraph_setParent(psg.sceneGraph, uiBorderId, uiPlaneId);
        float x = side % 2 == 0 ? 1 : -1;
        float y = side < 3 ? 1 : -1;
        if (side < 2) {
            y = 0;
        } else {
            x = 0;
        }
        SceneGraph_setLocalPosition(psg.sceneGraph, uiBorderId, (Vector3) { 9 * x, y * 6, fabsf(y) });
        SceneGraph_setLocalRotation(psg.sceneGraph, uiBorderId, (Vector3) { 0, 0, 180 * side + (side >= 2 ? 90 : 0) });
        for (int i = -3; i <= 3; i += 1) {
            SceneObjectId element = SceneGraph_createObject(psg.sceneGraph, "ui-border-element");
            SceneGraph_setParent(psg.sceneGraph, element, uiBorderId);
            SceneGraph_setLocalPosition(psg.sceneGraph, element, (Vector3) { 0, i * 4, 0 });
            AddMeshRendererComponentByName(element, "ui-border", 1.0f);
        }
    }
    SceneObjectId landscapeText = SceneGraph_createObject(psg.sceneGraph, "landscape-text");
    SceneGraph_setParent(psg.sceneGraph, landscapeText, uiPlaneId);
    SceneGraph_setLocalPosition(psg.sceneGraph, landscapeText, (Vector3) { 0, 8.5f, 0 });
    Font font = ResourceManager_loadFont(&psg.resourceManager, "assets/myfont.png");
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    SceneGraph_addComponent(psg.sceneGraph, landscapeText, psg.textComponentId,
        &(TextComponent) {
            .text = "Play in Landscape",
            .font = font,
            .fontSize = 30,
            .offset = (Vector2) { 0, 0 },
            .fontSpacing = 1,
            .align = (Vector2) { 0.5f, 0.5f },
            .color = (Color) { 255, 255, 255, 255 },
        });

    SceneObjectId remainingDistanceText = SceneGraph_createObject(psg.sceneGraph, "remaining-distance-text");
    SceneGraph_setParent(psg.sceneGraph, remainingDistanceText, uiPlaneId);
    SceneGraph_setLocalPosition(psg.sceneGraph, remainingDistanceText, (Vector3) { 7.5f, 5.0f, 0 });
    gameUiSystem->remainingDistanceTextId = SceneGraph_addComponent(psg.sceneGraph, remainingDistanceText, psg.textComponentId,
        &(TextComponent) {
            .text = "Remaining Distance: 1000",
            .font = font,
            .fontSize = 8,
            .offset = (Vector2) { 0, 0 },
            .fontSpacing = 1,
            .lineSpacing = 1.0f,
            .align = (Vector2) { 0.0f, 0.0f },
            .color = (Color) { 255, 255, 255, 255 },
        });

    SceneObjectId panel = AddPanel(uiPlaneId, 0, 0, 5, 5, 0xffffffff);
    AddButton(font, 0xffffffff, panel, 0, 1.0f, 3.0f, 1.0f, "Option 1", NULL);
    AddButton(font, 0xffffffff, panel, 0, -0.0f, 3.0f, 1.0f, "OSTU 2", NULL);
    AddButton(font, 0xffffffff, panel, 0, -1.0f, 3.0f, 1.0f, "Option 3", NULL);

    gameUiSystem->menuButtonId = AddButton(font, 0xff8800ff, uiPlaneId, -7.0f, 4.5f, 1.0f, 1.0f, "", &(SpriteAsset){
        .scale9frame = {0},
        .source = (Rectangle){0, 160, 64, 64},
        .texture = ResourceManager_loadTexture(&psg.resourceManager, "assets/ui_atlas.png", TEXTURE_FILTER_BILINEAR)
    });
    gameUiSystem->menuPanelId = panel;
    gameUiSystem->gameOverPanelId = AddPanel(uiPlaneId, 0, 0, 8, 5,0x88221188);
    SceneObjectId gameOverText = SceneGraph_createObject(psg.sceneGraph, "game-over-text");
    SceneGraph_setParent(psg.sceneGraph, gameOverText, gameUiSystem->gameOverPanelId);
    SceneGraph_setLocalPosition(psg.sceneGraph, gameOverText, (Vector3) { 0, 1.5f, 0 });
    SceneGraph_addComponent(psg.sceneGraph, gameOverText, psg.textComponentId,
        &(TextComponent) {
            .text = "Game Over",
            .font = font,
            .fontSize = 14,
            .offset = (Vector2) { 0, 0 },
            .fontSpacing = 1,
            .align = (Vector2) { 0.5f, 0.5f },
            .color = (Color) { 255, 255, 255, 255 },
        });
    
    gameUiSystem->restartButtonId = AddButton(font, 0xff8800ff, gameUiSystem->gameOverPanelId, 0, -1.0f, 3.0f, 1.0f, "Restart", NULL);

    SceneGraph_setObjectEnabled(psg.sceneGraph, gameUiSystem->gameOverPanelId, 0);
    SceneGraph_setObjectEnabled(psg.sceneGraph, gameUiSystem->menuPanelId, 0);
    // SceneGraph_setLocalPosition(psg.sceneGraph, uiPlaneId, (Vector3) { 0, 0, 0 });

}

void GameUiSystemRegister()
{
    psg.gameUiSystemId = SceneGraph_registerComponentType(psg.sceneGraph, "GameUiSystem", sizeof(GameUiSystem),
        (SceneComponentTypeMethods) {
            .onInitialize = GameUiSystem_onInitialize,
            .updateTick = GameUi_Update
        });
}