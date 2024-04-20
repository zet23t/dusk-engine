#include "dusk-gui.h"
#include "stdlib.h"
#include "stdio.h"

static DuskGuiState _duskGuiState;

static int DuskGui_idEquals(DuskGuiParamsEntry *a, DuskGuiParamsEntry *b)
{
    if (a->id == b->id && a->txId == b->txId) return 1;
    return a->txId && b->txId && strcmp(a->txId, b->txId) == 0;
}

static DuskGuiParamsEntry* DuskGui_findParams(DuskGuiParamsList *paramsList, DuskGuiParamsEntry *params)
{
    if (params->id < paramsList->count && DuskGui_idEquals(&paramsList->params[params->id], params))
    {
        return &paramsList->params[params->id];
    }
    for (int i = 0; i < paramsList->count; i++)
    {
        if (DuskGui_idEquals(&paramsList->params[i], params))
        {
            return &paramsList->params[i];
        }
    }
    return NULL;
}

static void DuskGui_defaultDrawStyle(DuskGuiParamsEntry *params, DuskGuiState *state)
{
    DrawRectangleRec(params->params.bounds, params->isPressed ? YELLOW : (params->isHovered ? RED : BLUE));
}

void DuskGui_init()
{
    _duskGuiState.currentParams = (DuskGuiParamsList) {
        .params = (DuskGuiParamsEntry *)malloc(sizeof(DuskGuiParamsEntry) * 256),
        .count = 0,
        .capacity = 256
    };
    _duskGuiState.prevParams = (DuskGuiParamsList) {
        .params = (DuskGuiParamsEntry *)malloc(sizeof(DuskGuiParamsEntry) * 256),
        .count = 0,
        .capacity = 256
    };
}

int DuskGui_hasLock(DuskGuiParamsEntry *params)
{
    return _duskGuiState.locked.txId != NULL && DuskGui_idEquals(&_duskGuiState.locked, params);
}

int DuskGui_isNotLocked(DuskGuiParamsEntry *params)
{
    return _duskGuiState.locked.txId == NULL || DuskGui_idEquals(&_duskGuiState.locked, params);
}

int DuskGui_tryLock(DuskGuiParamsEntry *params)
{
    if (_duskGuiState.locked.txId != NULL)
    {
        return strcmp(_duskGuiState.locked.txId, params->txId) == 0;
    }

    if (_duskGuiState.locked.txId)
    {
        free(_duskGuiState.locked.txId);
    }
    _duskGuiState.locked = *params;
    _duskGuiState.locked.txId = strdup(params->txId);
    return 1;
}

void DuskGui_unlock()
{
    if (_duskGuiState.locked.txId)
    {
        free(_duskGuiState.locked.txId);
    }
    _duskGuiState.locked.txId = NULL;
}

static void DuskGui_addParams(DuskGuiParamsList *paramsList, DuskGuiParamsEntry *params)
{
    if (params->params.rayCastTarget == 0) return;
    int index = params->id;

    if (index >= paramsList->capacity)
    {
        paramsList->capacity *= 2;
        paramsList->params = (DuskGuiParamsEntry *)realloc(paramsList->params, sizeof(DuskGuiParamsEntry) * paramsList->capacity);
    }
    if (index >= paramsList->count)
    {
        //memset 0 empty entries
        for (int i = paramsList->count; i < index; i++)
        {
            paramsList->params[i] = (DuskGuiParamsEntry) {0};
        }
        paramsList->count = index + 1;
    }
    paramsList->params[index] = *params;
}

void DuskGui_evaluate()
{
    _duskGuiState.idCounter = 0;
    // Swap current and previous params
    DuskGuiParamsList tmp = _duskGuiState.prevParams;
    _duskGuiState.prevParams = _duskGuiState.currentParams;
    _duskGuiState.currentParams = tmp;
    // free entries
    for (int i = 0; i < _duskGuiState.currentParams.count; i++)
    {
        if (_duskGuiState.currentParams.params[i].txId)
        {
            free(_duskGuiState.currentParams.params[i].txId);
        }
    }
    _duskGuiState.currentParams.count = 0;

    // reversed raycast hit test
    int blocked = 0;
    for (int i = _duskGuiState.prevParams.count - 1; i >= 0; i--)
    {
        DuskGuiParamsEntry *entry = &_duskGuiState.prevParams.params[i];
        if (entry->params.rayCastTarget == 0) continue;
        int isHit = !blocked && CheckCollisionPointRec(GetMousePosition(), entry->params.bounds);
        if (isHit)
        {
            blocked = 1;
        }

        if (isHit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            DuskGui_tryLock(entry);
        }

        if (isHit && IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !DuskGui_hasLock(entry))
        {
            isHit = 0;
        }


        entry->isHovered = isHit;
        entry->isPressed = entry->isHovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        entry->isTriggered = isHit && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && DuskGui_hasLock(entry);
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        DuskGui_unlock();
    }
}

int DuskGui_button(DuskGuiParams params)
{
    DuskGuiParamsEntry entry = {
        .id = _duskGuiState.idCounter++,
        .txId = NULL,
        .params = params
    };
    if (params.text)
    {
        const char *text = params.text;
        
        for (int i=0;params.text[i];i++)
        {
            if (text[i] == '#' && text[i+1] == '#')
            {
                text = &text[i+2];
                break;
            }
        }
        entry.txId = strdup(text);
    }
    DuskGui_addParams(&_duskGuiState.currentParams, &entry);
    DuskGuiParamsEntry *match = DuskGui_findParams(&_duskGuiState.prevParams, &entry);
    if (match)
    {
        entry.isHovered = match->isHovered;
        entry.isPressed = match->isPressed;
        entry.isTriggered = match->isTriggered;
    }
    if (params.style) {
        params.style->draw(&entry, &_duskGuiState);
    } else {
        DuskGui_defaultDrawStyle(&entry, &_duskGuiState);
    }
    return entry.isTriggered;
}
