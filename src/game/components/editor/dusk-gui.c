#include "dusk-gui.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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

static void DuskGui_defaultDrawStyle(DuskGuiParamsEntry *params, DuskGuiState *state, DuskGuiStyle *fallbackStyle);

static DuskGuiStyle _defaultButtonStyle = {
    .draw = DuskGui_defaultDrawStyle,
    .font = {0},
    .fontSize = 10,
    .fontSpacing = 1,
    .textAlignmentX = 0.5f,
    .textAlignmentY = 0.5f,
    .paddingLeft = 4,
    .paddingRight = 4,
    .paddingTop = 4,
    .paddingBottom = 4,
    .textColorNormal = BLACK,
    .textColorHover = BLACK,
    .textColorPressed = RED,
    .backgroundColorNormal = (Color){200,200,200,255},
    .backgroundColorHover = (Color){200,220,250,255},
    .backgroundColorPressed = WHITE,
    .textureNormal = {0},
    .textureHover = {0},
    .texturePressed = {0},
    .styleUserData = NULL
};

void DuskGui_setDefaultFont(Font font, float fontSize, int fontSpacing, Color normal, Color hover, Color pressed)
{
    _defaultButtonStyle.font = font;
    _defaultButtonStyle.fontSize = fontSize;
    _defaultButtonStyle.fontSpacing = fontSpacing;
    _defaultButtonStyle.textColorNormal = normal;
    _defaultButtonStyle.textColorHover = hover;
    _defaultButtonStyle.textColorPressed = pressed;
}

static void DuskGui_defaultDrawStyle(DuskGuiParamsEntry *entry, DuskGuiState *state, DuskGuiStyle *fallbackStyle)
{
    DuskGuiStyle *style = entry->params.style ? entry->params.style : fallbackStyle;
    Color textColor = style->textColorNormal;
    Color backgroundColor = style->backgroundColorNormal;
    Texture2D texture = style->textureNormal;
    NPatchInfo nPatchInfo = style->nPatchInfoNormal;
    if (entry->isHovered)
    {
        textColor = style->textColorHover;
        backgroundColor = style->backgroundColorHover;
        texture = style->textureHover.id != 0 ? style->textureHover : texture;
        nPatchInfo = style->nPatchInfoHover;
    }
    if (entry->isPressed)
    {
        textColor = style->textColorPressed;
        backgroundColor = style->backgroundColorPressed;
        texture = style->texturePressed.id != 0 ? style->texturePressed : texture;
        nPatchInfo = style->nPatchInfoPressed;
    }

    if (texture.id == 0)
    {
        DrawRectangleRec(entry->params.bounds, backgroundColor);
    }
    else
    {
        DrawTextureNPatch(texture, nPatchInfo, entry->params.bounds, (Vector2){0,0}, 0, backgroundColor);
    }

    if (entry->params.text)
    {
        Font font = style->font;
        if (font.texture.id == 0)
        {
            font = GetFontDefault();
        }
        char text[strlen(entry->params.text) + 4];
        strcpy(text, entry->params.text);
        Vector2 box = {0}, availableSpace;
        availableSpace = (Vector2) {
            entry->params.bounds.width - style->paddingLeft - style->paddingRight,
            entry->params.bounds.height - style->paddingTop - style->paddingBottom
        };
        
        while (1 && text[3] != '\0') {
            box = MeasureTextEx(font, text, style->fontSize, style->fontSpacing);
            if (box.x > availableSpace.x)
            {
                int len = strlen(text);
                text[len - 1] = '\0';
                for (int i = len - 2, j = 2; i >= 0 && j >= 0; i--, j--)
                {
                    text[i] = '.';
                }
            }
            else
            {
                break;
            }
        }
        int x = (int) (entry->params.bounds.x + style->paddingLeft + (availableSpace.x - box.x) * style->textAlignmentX);
        int y = (int) (entry->params.bounds.y + style->paddingTop + (availableSpace.y - box.y) * style->textAlignmentY);
        DrawTextEx(font, text, (Vector2){x,y}, style->fontSize, style->fontSpacing, textColor);
    }
}

NPatchInfo GenNPatchInfo(Texture2D texture, int top, int right, int bottom, int left)
{
    NPatchInfo nPatchInfo = {0};
    nPatchInfo.source = (Rectangle){0, 0, texture.width, texture.height};
    nPatchInfo.left = left;
    nPatchInfo.top = top;
    nPatchInfo.right = right;
    nPatchInfo.bottom = bottom;
    nPatchInfo.layout = NPATCH_NINE_PATCH;
    return nPatchInfo;
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

    // create default texture procedurally
    int size = 16;
    Image defaultImage = GenImageColor(size, size, WHITE);
    Color *pixels = (Color*) defaultImage.data;
    float half = (float) (size - 1) / 2;
    for (int x=0;x<size;x++)
    {
        for (int y=0;y<size;y++)
        {
            float dx = (x - half) / half;
            float dy = (y - half) / half;
            float sqd = dx * dx + dy * dy;
            if (sqd < 1.0f)
            {
                float d = sqrt(sqd);
                float alpha = (1.0f - d) * 8.0f;
                float brightness = (.85f - d) * 8.0f;
                if (alpha < 0) alpha = 0;
                if (alpha > 1) alpha = 1;
                if (brightness < 0) brightness = 0;
                if (brightness > 1) brightness = 1;
                int color = (int) (brightness * 255.0f);
                pixels[y * size + x] = (Color) {color, color, color, (float)(255.0f * alpha)};
            }
            else
            {
                pixels[y * size + x] = (Color) {255, 255, 255, 0};
            }
        }
    }

    Texture2D defaultTexture = LoadTextureFromImage(defaultImage);
    UnloadImage(defaultImage);
    _defaultButtonStyle.textureNormal = defaultTexture;
    SetTextureFilter(defaultTexture, TEXTURE_FILTER_BILINEAR);
    int n = size / 2;
    _defaultButtonStyle.nPatchInfoNormal = GenNPatchInfo(defaultTexture, n, n, n, n);
    _defaultButtonStyle.nPatchInfoHover = GenNPatchInfo(defaultTexture, n, n, n, n);
    _defaultButtonStyle.nPatchInfoPressed = GenNPatchInfo(defaultTexture, n, n, n, n);
    _defaultButtonStyle.font = GetFontDefault();

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
    if (params->params.rayCastTarget == 0) {
        free(params->txId);
        params->txId = NULL;
        return;
    }
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

static DuskGuiParamsEntry DuskGui_makeEntry(DuskGuiParams params)
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
    return entry;
}

int DuskGui_dragArea(DuskGuiParams params)
{
    DuskGuiParamsEntry entry = DuskGui_makeEntry(params);
    DuskGui_addParams(&_duskGuiState.currentParams, &entry);
    DuskGuiParamsEntry *match = DuskGui_findParams(&_duskGuiState.prevParams, &entry);
    if (match)
    {
        entry.isHovered = match->isHovered;
        entry.isPressed = match->isPressed;
        entry.isTriggered = match->isTriggered;
    }
    return DuskGui_hasLock(&entry);
}

int DuskGui_button(DuskGuiParams params)
{
    DuskGuiParamsEntry entry = DuskGui_makeEntry(params);
    DuskGui_addParams(&_duskGuiState.currentParams, &entry);
    DuskGuiParamsEntry *match = DuskGui_findParams(&_duskGuiState.prevParams, &entry);
    if (match)
    {
        entry.isHovered = match->isHovered;
        entry.isPressed = match->isPressed;
        entry.isTriggered = match->isTriggered;
    }
    if (params.style && params.style->draw) {
        params.style->draw(&entry, &_duskGuiState, &_defaultButtonStyle);
    } else {
        DuskGui_defaultDrawStyle(&entry, &_duskGuiState, &_defaultButtonStyle);
    }
    return entry.isTriggered;
}
