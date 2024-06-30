#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RuntimeContext.h"
#include "shared/ui/dusk-gui.h"

#define COMPONENT(t) void t##Register();
#include "component_list.h"
#undef COMPONENT

static RuntimeContext *_runtimeContext;
static SceneGraph *_scene;

int game_init(RuntimeContext *runtimeContext)
{
    _scene = SceneGraph_create();
    _runtimeContext = runtimeContext;

    DuskGui_init();
    Font font = ResourceManager_loadFont(&runtimeContext->resourceManager, "assets/myfont-regular.png");
    DuskGui_setDefaultFont(font, font.baseSize, 1);

#define COMPONENT(t) t##Register();
#include "component_list.h"
#undef COMPONENT
    return 1;
}

void game_draw()
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int panelHeight = 150;
    int panelWidth = 300;
    DuskGuiParamsEntryId panel = DuskGui_beginPanel(
        (DuskGuiParams){
            .rayCastTarget = 1,
            .bounds = (Rectangle){
                .x = (screenWidth - panelWidth) * 0.5f, 
                .y = (screenHeight - panelHeight) * 0.5f, 
                .width = panelWidth, .height = panelHeight}});

    static DuskGuiStyleGroup group = {0};
    group = *DuskGui_getStyleGroup(DUSKGUI_STYLE_LABEL);
    group.fallbackStyle.textAlignment.y = 0.0f;

    DuskGui_label((DuskGuiParams){
        .text = "Hello, World!\n"
            "This UI is made with the [color=840f]Dusk GUI system[/color].\n"
            "You can recompile and reload with [color=00ff]F8[/color].",
        .bounds = (Rectangle){10.0f, 10.0f, panelWidth, panelHeight},
        .styleGroup = &group
    });

    DuskGui_endPanel(panel);
    
    Editor_draw(&_runtimeContext->editorState, _scene);
    Editor_drawControls(&_runtimeContext->editorState, _scene);

    DuskGui_finalize();
}

void game_update(float dt)
{
    _runtimeContext->editorState.gameTime += dt;
    _runtimeContext->editorState.frameCount += 1;
}