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
    void (*draw)(DuskGuiParamsEntry *params, DuskGuiState *state);
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
int DuskGui_button(DuskGuiParams params);

#ifdef DUSK_GUI_IMPLEMENTATION

#include "dusk-gui.c"
#endif
#endif