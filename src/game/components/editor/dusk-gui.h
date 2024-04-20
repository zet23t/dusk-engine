#ifndef __DUSK_GUI_H__
#define __DUSK_GUI_H__

#include "raylib.h"

typedef struct DuskGuiParamsEntry DuskGuiParamsEntry;
typedef struct DuskGuiParams DuskGuiParams;
typedef struct DuskGuiStyle DuskGuiStyle;
typedef struct DuskGuiState DuskGuiState;

typedef struct DuskGuiParamsEntryList {
    DuskGuiParamsEntry *params;
    int count;
    int capacity;
} DuskGuiParamsList;

typedef struct DuskGuiStyle
{
    void (*draw)(DuskGuiParamsEntry *params, DuskGuiState *state, struct DuskGuiStyle *fallback);
    
    Font font;
    float fontSize;
    int fontSpacing;

    float textAlignmentX;
    float textAlignmentY;

    int paddingLeft;
    int paddingRight;
    int paddingTop;
    int paddingBottom;
    
    Color textColorNormal;
    Color textColorHover;
    Color textColorPressed;
    Color backgroundColorNormal;
    Color backgroundColorHover;
    Color backgroundColorPressed;

    Texture2D textureNormal;
    Texture2D textureHover;
    Texture2D texturePressed;
    NPatchInfo nPatchInfoNormal;
    NPatchInfo nPatchInfoHover;
    NPatchInfo nPatchInfoPressed;
    
    void *styleUserData;
} DuskGuiStyle;

typedef struct DuskGuiParams
{
    const char *text;
    Rectangle bounds;
    int rayCastTarget;
    DuskGuiStyle *style;
} DuskGuiParams;

typedef struct DuskGuiParamsEntry
{
    int id;
    char *txId;
    char isHovered;
    char isPressed;
    char isTriggered;
    DuskGuiParams params;
} DuskGuiParamsEntry;

typedef struct DuskGuiState {
    DuskGuiParamsList currentParams;
    DuskGuiParamsList prevParams;
    DuskGuiParamsEntry locked;
    int idCounter;
} DuskGuiState;

void DuskGui_init();
void DuskGui_evaluate();
void DuskGui_setDefaultFont(Font font, float fontSize, int fontSpacing, Color normal, Color hover, Color pressed);
int DuskGui_button(DuskGuiParams params);

#ifdef DUSK_GUI_IMPLEMENTATION

#include "dusk-gui.c"
#endif
#endif