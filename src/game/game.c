#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RuntimeContext.h"
#include "shared/ui/dusk-gui.h"

#define COMPONENT(t) void t##Register();
#include "component_list.h"
#undef COMPONENT

int game_init(RuntimeContext *runtimeContext)
{
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

    DuskGui_label((DuskGuiParams){
        .text = "Hello, World!\n"
            "This is a test of the Dusk GUI system.\n"
            "Recompile and reload with F8.",
        .bounds = (Rectangle){10, 0, panelWidth, panelHeight},
    });

    DuskGui_endPanel(panel);
    
    // Editor_draw(_editorState, _scene);
    // Editor_drawControls(_editorState, _scene);

    DuskGui_finalize();
}

void game_update(float dt)
{
    
}