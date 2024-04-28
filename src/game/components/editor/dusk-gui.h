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
    char isMouseOver;
    char isHovered;
    char isPressed;
    char isTriggered;
    Vector2 contentOffset;
    Vector2 contentSize;

    DuskGuiParams params;
    DuskGuiParamsEntry *parent;
} DuskGuiParamsEntry;

typedef struct DuskGuiState {
    DuskGuiParamsList currentParams;
    DuskGuiParamsList prevParams;
    DuskGuiParamsEntry locked;
    int idCounter;
    DuskGuiParamsEntry root;
    // Note: this is stupid; should use txId instead
    DuskGuiParamsEntry *currentPanel;

} DuskGuiState;

typedef enum DuskGuiStyleType {
    DUSKGUI_STYLE_BUTTON = 0,
    DUSKGUI_STYLE_LABEL,
    DUSKGUI_STYLE_LABELBUTTON,
    DUSKGUI_STYLE_PANEL,
    DUSKGUI_STYLE_HORIZONTAL_LINE,
    DUSKGUI_MAX_STYLESHEETS
} DuskGuiStyleType;

typedef struct DuskGuiStyleSheet {
    DuskGuiStyle styles[DUSKGUI_MAX_STYLESHEETS];
} DuskGuiStyleSheet;

void DuskGui_init();
void DuskGui_evaluate();
void DuskGui_setDefaultFont(Font font, float fontSize, int fontSpacing, Color normal, Color hover, Color pressed);
DuskGuiStyle* DuskGui_getStyle(int styleType);
int DuskGui_button(DuskGuiParams params);
int DuskGui_dragArea(DuskGuiParams params);
int DuskGui_label(DuskGuiParams params);
void DuskGui_horizontalLine(DuskGuiParams params);
void DuskGui_setContentSize(DuskGuiParamsEntry entry, Vector2 contentSize);
void DuskGui_setContentOffset(DuskGuiParamsEntry entry, Vector2 contentOffset);
Vector2 DuskGui_getAvailableSpace();

DuskGuiParamsEntry DuskGui_beginScrollArea(DuskGuiParams params);
void DuskGui_endScrollArea(DuskGuiParamsEntry entry);

DuskGuiParamsEntry DuskGui_beginPanel(DuskGuiParams params);
void DuskGui_endPanel(DuskGuiParamsEntry entry);

#ifdef DUSK_GUI_IMPLEMENTATION

#include "dusk-gui.c"
#endif
#endif