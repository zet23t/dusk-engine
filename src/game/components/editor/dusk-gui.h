#ifndef __DUSK_GUI_H__
#define __DUSK_GUI_H__

#include "raylib.h"

#define DUSKGUI_OVERFLOW_ELLIPSIS 0
#define DUSKGUI_OVERFLOW_CLIP 1

typedef struct DuskGuiParamsEntry DuskGuiParamsEntry;
typedef struct DuskGuiParams DuskGuiParams;
typedef struct DuskGuiStyle DuskGuiStyle;
typedef struct DuskGuiState DuskGuiState;

typedef struct DuskGuiParamsEntryList {
    DuskGuiParamsEntry *params;
    int count;
    int capacity;
} DuskGuiParamsList;

typedef struct DuskGuiFontStyle {
    Font font;
    float fontSize;
    int fontSpacing;
} DuskGuiFontStyle;

typedef struct DuskGuiStyle
{
    DuskGuiFontStyle *fontStyle;

    float transitionLingerTime;
    float transitionActivationTime;

    Vector2 textAlignment;

    int paddingLeft;
    int paddingRight;
    int paddingTop;
    int paddingBottom;

    int textOverflowType;
    
    Color textColor;
    Color backgroundColor;
    
    Texture2D iconTexture;
    Color iconColor;
    Vector2 iconSize;
    Vector2 iconPivot;
    Vector2 iconOffset;
    Vector2 iconAlignment;
    float iconRotationDegrees;

    Texture2D backgroundTexture;
    NPatchInfo backgroundPatchInfo;

    void *styleUserData;
} DuskGuiStyle;

typedef struct DuskGuiStyleGroup
{
    void (*draw)(DuskGuiParamsEntry *params, DuskGuiState *state, struct DuskGuiStyleGroup *fallback);

    DuskGuiStyle fallbackStyle;
    DuskGuiStyle* normal;
    DuskGuiStyle* hover;
    DuskGuiStyle* pressed;
    DuskGuiStyle* active;
    DuskGuiStyle* disabled;
    DuskGuiStyle* focused;
    DuskGuiStyle* selected;
} DuskGuiStyleGroup;

typedef struct DuskGuiParams
{
    const char *text;
    Rectangle bounds;
    unsigned char rayCastTarget:1;
    unsigned char isFocusable:1;
    DuskGuiStyleGroup *styleGroup;
} DuskGuiParams;

typedef struct DuskGuiTextBuffer
{
    char *buffer;
    int capacity;
    int refCount;
} DuskGuiTextBuffer;

typedef struct DuskGuiParamsEntry
{
    int id;
    int cursorIndex;
    int selectionStart;
    int selectionEnd;
    char *txId;
    union {
        uint8_t flags;
        struct {
            char isMouseOver:1;
            char isHovered:1;
            char isPressed:1;
            char isTriggered:1;
            char isFolded:1;
            char isFocused:1;
        };
    };
    Vector2 contentOffset;
    Vector2 contentSize;

    DuskGuiStyle cachedStartStyle;
    DuskGuiStyle *origStartStyle;
    DuskGuiStyle *nextStyle;
    float transitionTime;
    float transitionDuration;
    // set by draw operation
    Rectangle textBounds;
    Vector2 textOffset;

    DuskGuiTextBuffer *textBuffer;
    
    DuskGuiParams params;
    int parentIndex;
} DuskGuiParamsEntry;

typedef struct DuskGuiState {
    DuskGuiParamsList currentParams;
    DuskGuiParamsList prevParams;
    DuskGuiParamsEntry locked;
    Vector2 lockScreenPos;
    Vector2 lockRelativePos;
    int idCounter;
    DuskGuiParamsEntry root;
    int currentPanelIndex;
} DuskGuiState;

typedef enum DuskGuiStyleType {
    DUSKGUI_STYLE_BUTTON = 0,
    DUSKGUI_STYLE_LABEL,
    DUSKGUI_STYLE_LABELBUTTON,
    DUSKGUI_STYLE_FOLDOUT_OPEN,
    DUSKGUI_STYLE_FOLDOUT_CLOSED,
    DUSKGUI_STYLE_PANEL,
    DUSKGUI_STYLE_HORIZONTAL_LINE,
    DUSKGUI_STYLE_INPUTTEXTFIELD,
    DUSKGUI_STYLE_INPUTNUMBER_FIELD,
    DUSKGUI_STYLE_HORIZONTAL_SLIDER_BACKGROUND,
    DUSKGUI_STYLE_HORIZONTAL_SLIDER_HANDLE,
    DUSKGUI_MAX_STYLESHEETS
} DuskGuiStyleType;

typedef struct DuskGuiStyleSheet {
    DuskGuiStyleGroup groups[DUSKGUI_MAX_STYLESHEETS];
} DuskGuiStyleSheet;

void DuskGui_init();
void DuskGui_evaluate();
void DuskGui_setDefaultFont(Font font, float fontSize, int fontSpacing);
DuskGuiStyleGroup* DuskGui_getStyleGroup(int styleType);
int DuskGui_button(DuskGuiParams params);
int DuskGui_dragArea(DuskGuiParams params);
int DuskGui_label(DuskGuiParams params);
int DuskGui_textInputField(DuskGuiParams params, char** buffer);
int DuskGui_foldout(DuskGuiParams params);
void DuskGui_horizontalLine(DuskGuiParams params);
Vector2 DuskGui_getAvailableSpace();
int DuskGui_horizontalFloatSlider(DuskGuiParams params, float* value, float min, float max);
int DuskGui_floatInputField(DuskGuiParams params, float *value, float min, float max);

DuskGuiParamsEntry* DuskGui_beginScrollArea(DuskGuiParams params);
void DuskGui_endScrollArea(DuskGuiParamsEntry* entry);

DuskGuiParamsEntry* DuskGui_beginPanel(DuskGuiParams params);
void DuskGui_endPanel(DuskGuiParamsEntry* entry);

#ifdef DUSK_GUI_IMPLEMENTATION

#include "dusk-gui.c"
#endif
#endif