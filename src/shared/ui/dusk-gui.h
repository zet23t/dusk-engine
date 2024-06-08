#ifndef __DUSK_GUI_H__
#define __DUSK_GUI_H__

#include "raylib.h"
#include <inttypes.h>

#define DUSKGUI_OVERFLOW_ELLIPSIS 0
#define DUSKGUI_OVERFLOW_CLIP 1

typedef int DuskGuiParamsEntryId;
typedef struct DuskGuiParamsEntry DuskGuiParamsEntry;
typedef struct DuskGuiParams DuskGuiParams;
typedef struct DuskGuiStyle DuskGuiStyle;
typedef struct DuskGuiState DuskGuiState;
typedef struct DuskGuiStyleGroup DuskGuiStyleGroup;

typedef void (*DuskGuiDrawFn)(DuskGuiParamsEntry *params, DuskGuiState *state, DuskGuiStyleGroup *style);

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

typedef struct DuskGuiIconSprite
{
    Texture2D texture;
    Color color;
    Vector2 pivot;
    Vector2 alignment;
    Rectangle src;
    Rectangle dst;
    float rotationDegrees;
} DuskGuiIconSprite;

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
    
    DuskGuiIconSprite icon;

    Texture2D backgroundTexture;
    NPatchInfo backgroundPatchInfo;

    void *styleUserData;
} DuskGuiStyle;

typedef struct DuskGuiStyleGroup
{
    const char *name;
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
    DuskGuiIconSprite icon;
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
    Rectangle originalBounds;
    int cursorIndex;
    int selectionStart;
    int selectionEnd;
    char *txId;
    union {
        uint8_t flags;
        struct {
            uint8_t isMouseOver:1;
            uint8_t isHovered:1;
            uint8_t isPressed:1;
            uint8_t isTriggered:1;
            uint8_t isFolded:1;
            uint8_t isFocused:1;
            uint8_t isMenu:1;
            uint8_t isOpen:1;
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
    DuskGuiParamsEntryId parentIndex;

    DuskGuiDrawFn drawFn;
    DuskGuiStyleGroup *drawStyleGroup;
    float iconRotationDegrees;
    void *drawUserData;
} DuskGuiParamsEntry;

#define DUSKGUI_MAX_MENU_DEPTH 16
#define DUSKGUI_MAX_ACTIVE_MENU_COUNT 16

typedef struct DuskGuiActiveMenuStats
{
    char *menuName;
    float lastTriggerTime;
    float firstActiveTime;
} DuskGuiActiveMenuStats;

typedef struct DuskGuiState {
    DuskGuiParamsList currentParams;
    DuskGuiParamsList prevParams;
    DuskGuiParamsEntry locked;
    Vector2 lockScreenPos;
    Vector2 lockRelativePos;
    int idCounter;
    DuskGuiParamsEntry root;
    int currentPanelIndex;

    // active menus, persisted between frames
    DuskGuiActiveMenuStats activeMenus[DUSKGUI_MAX_ACTIVE_MENU_COUNT];
    
    DuskGuiParamsEntry *menuStack[DUSKGUI_MAX_MENU_DEPTH];
    int menuStackCount;

    DuskGuiParamsEntry *lastEntry;
} DuskGuiState;

typedef enum DuskGuiStyleType {
    DUSKGUI_STYLE_BUTTON = 0,
    DUSKGUI_STYLE_LABEL,
    DUSKGUI_STYLE_LABELBUTTON,
    DUSKGUI_STYLE_FOLDOUT_OPEN,
    DUSKGUI_STYLE_FOLDOUT_CLOSED,
    DUSKGUI_STYLE_PANEL,
    DUSKGUI_STYLE_HORIZONTAL_LINE,
    DUSKGUI_STYLE_ICON,
    DUSKGUI_STYLE_INPUTTEXTFIELD,
    DUSKGUI_STYLE_INPUTNUMBER_FIELD,
    DUSKGUI_STYLE_HORIZONTAL_SLIDER_BACKGROUND,
    DUSKGUI_STYLE_HORIZONTAL_SLIDER_HANDLE,
    DUSKGUI_STYLE_MENU,
    DUSKGUI_STYLE_MENU_ITEM,
    DUSKGUI_MAX_STYLESHEETS
} DuskGuiStyleType;

typedef struct DuskGuiStyleSheet {
    DuskGuiStyleGroup groups[DUSKGUI_MAX_STYLESHEETS];
} DuskGuiStyleSheet;

void DuskGui_init();
// finalizes the frame operation; draws menus as a last step
void DuskGui_finalize();

DuskGuiParamsEntry* DuskGui_getLastEntry();
DuskGuiParamsEntry* DuskGui_getEntry(const char *txId);

// style management
void DuskGui_setDefaultFont(Font font, float fontSize, int fontSpacing);
DuskGuiStyleGroup* DuskGui_getStyleGroup(int styleType);
// copies the style and allocates memory for it
DuskGuiStyle* DuskGui_createGuiStyle(DuskGuiStyle* fallbackStyle);

// menu management
void DuskGui_openMenu(const char *menuName);
int DuskGui_isMenuOpen(const char *menuName);
int DuskGui_closeMenu(const char *menuName);
void DuskGui_closeAllMenus();

// layouting helpers
Rectangle DuskGui_fillHorizontally(int yOffset, int left, int right, int height);


// widgets
int DuskGui_button(DuskGuiParams params);
DuskGuiParamsEntry* DuskGui_icon(const char *id, Rectangle dst, Texture2D icon, Rectangle src, int raycastTarget);
int DuskGui_dragArea(DuskGuiParams params);
int DuskGui_label(DuskGuiParams params);
int DuskGui_textInputField(DuskGuiParams params, char** buffer);
int DuskGui_foldout(DuskGuiParams params);
void DuskGui_horizontalLine(DuskGuiParams params);
Vector2 DuskGui_getAvailableSpace();
int DuskGui_horizontalFloatSlider(DuskGuiParams params, float* value, float min, float max);
int DuskGui_floatInputField(DuskGuiParams params, float *value, float min, float max);
int DuskGui_menuItem(int opensSubmenu, DuskGuiParams params);

// containers
DuskGuiParamsEntryId DuskGui_beginScrollArea(DuskGuiParams params);
void DuskGui_endScrollArea(DuskGuiParamsEntryId entry);

DuskGuiParamsEntryId DuskGui_beginPanel(DuskGuiParams params);
void DuskGui_endPanel(DuskGuiParamsEntryId entry);

DuskGuiParamsEntry* DuskGui_beginMenu(DuskGuiParams params);
void DuskGui_endMenu();

DuskGuiParamsEntry* DuskGui_getEntryById(DuskGuiParamsEntryId entryId);

#ifdef DUSK_GUI_IMPLEMENTATION

#include "dusk-gui.c"
#endif
#endif