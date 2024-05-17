#include "editor.h"
#include "shared/ui/dusk-gui.h"
#include "shared/util/arena.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DuskGuiStyleGroup _editor_titleStyleGroup = { 0 };
DuskGuiStyleGroup _editor_closeButtonStyleGroup = { 0 };
DuskGuiStyleGroup _editor_resizeAreaStyleGroup = { 0 };
DuskGuiStyleGroup _editor_invisibleStyleGroup = { 0 };
DuskGuiStyleGroup _editor_labelCenteredStyleGroup = { 0 };

Texture2D _editor_iconHierarchy = { 0 };
Texture2D _editor_iconInspector = { 0 };
Texture2D _editor_iconPlay = { 0 };
Texture2D _editor_iconPause = { 0 };
Texture2D _editor_iconStep = { 0 };

void EditorStyles_init()
{
    Image hierarchy = GenImageColor(16, 16, (Color) { 0, 0, 0, 0 });
    ImageDrawRectangle(&hierarchy, 2, 0, 1, 15, WHITE);
    ImageDrawRectangle(&hierarchy, 3, 3, 10, 1, WHITE);
    ImageDrawRectangle(&hierarchy, 3, 6, 10, 1, WHITE);
    ImageDrawRectangle(&hierarchy, 3, 9, 10, 1, WHITE);
    ImageDrawRectangle(&hierarchy, 5, 9, 1, 6, WHITE);
    ImageDrawRectangle(&hierarchy, 5, 12, 8, 1, WHITE);

    _editor_iconHierarchy = LoadTextureFromImage(hierarchy);
    UnloadImage(hierarchy);

    Image inspector = GenImageColor(16, 16, (Color) { 0, 0, 0, 0 });
    ImageDrawCircle(&inspector, 7, 7, 6, WHITE);
    ImageDrawLine(&inspector, 7, 7, 13, 13, WHITE);
    ImageDrawLine(&inspector, 6, 7, 12, 13, WHITE);
    ImageDrawLine(&inspector, 8, 7, 14, 13, WHITE);
    ImageDrawCircle(&inspector, 7, 7, 4, BLANK);
    _editor_iconInspector = LoadTextureFromImage(inspector);
    UnloadImage(inspector);

    Image play = GenImageColor(16, 16, (Color) { 0, 0, 0, 0 });
    for (int i = 0; i<7; i++)
    {
        ImageDrawRectangle(&play, i + 4, i+2, 1, 13-i*2, WHITE);
    }
    _editor_iconPlay = LoadTextureFromImage(play);
    UnloadImage(play);

    Image pause = GenImageColor(16, 16, (Color) { 0, 0, 0, 0 });
    ImageDrawRectangle(&pause, 2, 2, 4, 12, WHITE);
    ImageDrawRectangle(&pause, 10, 2, 4, 12, WHITE);
    _editor_iconPause = LoadTextureFromImage(pause);
    UnloadImage(pause);

    Image step = GenImageColor(16, 16, (Color) { 0, 0, 0, 0 });
    for (int i = 0; i<7; i++)
    {
        ImageDrawRectangle(&step, i + 3, i+2, 1, 13-i*2, WHITE);
    }
    ImageDrawRectangle(&step, 10, 2, 3, 12, WHITE);
    _editor_iconStep = LoadTextureFromImage(step);
    UnloadImage(step);


    _editor_titleStyleGroup.fallbackStyle = DuskGui_getStyleGroup(DUSKGUI_STYLE_PANEL)->fallbackStyle;
    _editor_titleStyleGroup.fallbackStyle.backgroundColor = (Color) { 100, 120, 180, 255 };
    _editor_titleStyleGroup.fallbackStyle.backgroundPatchInfo.source.height -= 4;
    _editor_titleStyleGroup.fallbackStyle.backgroundPatchInfo.bottom = 0;
    _editor_titleStyleGroup.fallbackStyle.textColor = WHITE;
    _editor_titleStyleGroup.normal = DuskGui_createGuiStyle(&_editor_titleStyleGroup.fallbackStyle);
    _editor_titleStyleGroup.hover = DuskGui_createGuiStyle(&_editor_titleStyleGroup.fallbackStyle);
    _editor_titleStyleGroup.hover->backgroundColor = (Color) { 120, 140, 200, 255 };
    _editor_titleStyleGroup.pressed = DuskGui_createGuiStyle(&_editor_titleStyleGroup.fallbackStyle);
    _editor_titleStyleGroup.pressed->backgroundColor = (Color) { 80, 100, 160, 255 };

    _editor_closeButtonStyleGroup = *DuskGui_getStyleGroup(DUSKGUI_STYLE_BUTTON);
    _editor_closeButtonStyleGroup.fallbackStyle.backgroundColor = (Color) { 200, 0, 0, 255 };
    _editor_closeButtonStyleGroup.fallbackStyle.textColor = WHITE;
    _editor_closeButtonStyleGroup.fallbackStyle.paddingBottom = 6;
    _editor_closeButtonStyleGroup.normal = DuskGui_createGuiStyle(&_editor_closeButtonStyleGroup.fallbackStyle);
    _editor_closeButtonStyleGroup.hover = DuskGui_createGuiStyle(&_editor_closeButtonStyleGroup.fallbackStyle);
    _editor_closeButtonStyleGroup.hover->backgroundColor = (Color) { 255, 0, 0, 255 };
    _editor_closeButtonStyleGroup.pressed = DuskGui_createGuiStyle(&_editor_closeButtonStyleGroup.fallbackStyle);
    _editor_closeButtonStyleGroup.pressed->backgroundColor = (Color) { 150, 0, 0, 255 };
    _editor_closeButtonStyleGroup.active = DuskGui_createGuiStyle(&_editor_closeButtonStyleGroup.fallbackStyle);

    _editor_labelCenteredStyleGroup = *DuskGui_getStyleGroup(DUSKGUI_STYLE_LABEL);
    _editor_labelCenteredStyleGroup.fallbackStyle.textAlignment.x = .5f;
    _editor_labelCenteredStyleGroup.normal = DuskGui_createGuiStyle(&_editor_labelCenteredStyleGroup.fallbackStyle);
    _editor_labelCenteredStyleGroup.hover = DuskGui_createGuiStyle(&_editor_labelCenteredStyleGroup.fallbackStyle);
    _editor_labelCenteredStyleGroup.pressed = DuskGui_createGuiStyle(&_editor_labelCenteredStyleGroup.fallbackStyle);
    _editor_labelCenteredStyleGroup.active = DuskGui_createGuiStyle(&_editor_labelCenteredStyleGroup.fallbackStyle);

    _editor_resizeAreaStyleGroup.fallbackStyle = (DuskGuiStyle) {
        .backgroundColor = { 0, 0, 0, 128 },
    };
}
