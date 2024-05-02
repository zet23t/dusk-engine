#include "dusk-gui.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static DuskGuiState _duskGuiState;

static int DuskGui_idEquals(DuskGuiParamsEntry* a, DuskGuiParamsEntry* b)
{
    if (a->id == b->id && a->txId == b->txId)
        return 1;
    return a->txId && b->txId && strcmp(a->txId, b->txId) == 0;
}

static DuskGuiParamsEntry* DuskGui_findParams(DuskGuiParamsList* paramsList, DuskGuiParamsEntry* params)
{
    if (params->id < paramsList->count && DuskGui_idEquals(&paramsList->params[params->id], params)) {
        return &paramsList->params[params->id];
    }
    for (int i = 0; i < paramsList->count; i++) {
        if (DuskGui_idEquals(&paramsList->params[i], params)) {
            return &paramsList->params[i];
        }
    }
    return NULL;
}

static void DuskGui_defaultDrawStyle(DuskGuiParamsEntry* params, DuskGuiState* state, DuskGuiStyleGroup* styleGroup);
static DuskGuiFontStyle _defaultFont;
static DuskGuiStyleSheet _defaultStyles;

void DuskGui_setDefaultFont(Font font, float fontSize, int fontSpacing)
{
    _defaultFont.font = font;
    _defaultFont.fontSize = fontSize;
    _defaultFont.fontSpacing = fontSpacing;
}

static void DuskGui_defaultDrawStyle(DuskGuiParamsEntry* entry, DuskGuiState* state, DuskGuiStyleGroup* styleGroup)
{
    DuskGuiStyleGroup* group = entry->params.styleGroup ? entry->params.styleGroup : styleGroup;
    DuskGuiStyle* style = group->normal ? group->normal : &group->fallbackStyle;
    if (entry->isHovered && group->hover) {
        style = group->hover;
    }
    if (entry->isPressed && group->pressed) {
        style = group->pressed;
    }
    if (entry->isTriggered && group->active) {
        style = group->active;
    }

    if (style->backgroundColor.a > 0) {
        if (style->backgroundTexture.id == 0) {
            DrawRectangleRec(entry->params.bounds, style->backgroundColor);
        } else {
            DrawTextureNPatch(style->backgroundTexture, style->backgroundPatchInfo, entry->params.bounds, (Vector2) { 0, 0 }, 0, style->backgroundColor);
        }
    }

    if (style->iconColor.a > 0 && style->iconTexture.id != 0) {
        Vector2 iconSize = style->iconSize;
        Vector2 iconPivot = style->iconPivot;
        Vector2 iconOffset = style->iconOffset;
        Vector2 iconAlignment = style->iconAlignment;
        float iconRotation = style->iconRotationDegrees;
        Vector2 iconPosition = (Vector2) {
            (entry->params.bounds.width - iconSize.x) * iconAlignment.x + entry->params.bounds.x + iconOffset.x,
            (entry->params.bounds.height - iconSize.y) * iconAlignment.y + entry->params.bounds.y + iconOffset.y
        };
        DrawTexturePro(style->iconTexture, (Rectangle) { 0, 0, style->iconTexture.width, style->iconTexture.height },
            (Rectangle) { iconPosition.x, iconPosition.y, iconSize.x, iconSize.y },
            (Vector2) { iconPivot.x * iconSize.x, iconPivot.y * iconSize.y }, iconRotation, style->iconColor);
    }

    if (entry->params.text && style->textColor.a > 0) {
        // draw debug outline
        // DrawRectangleLines(entry->params.bounds.x, entry->params.bounds.y, entry->params.bounds.width, entry->params.bounds.height, RED);

        Font font = style->fontStyle ? style->fontStyle->font : _defaultFont.font;
        if (font.texture.id == 0) {
            font = GetFontDefault();
        }
        char text[strlen(entry->params.text) + 4];
        strcpy(text, entry->params.text);

        for (int offset = strlen(text) - 1; offset > 0; offset--) {
            if (text[offset] == '#' && text[offset - 1] == '#') {
                text[offset - 1] = '\0';
                break;
            }
        }

        if (strlen(text) > 0) {

            Vector2 box = { 0 }, availableSpace;
            availableSpace = (Vector2) {
                entry->params.bounds.width - style->paddingLeft - style->paddingRight,
                entry->params.bounds.height - style->paddingTop - style->paddingBottom
            };

            int fontSize = style->fontStyle ? style->fontStyle->fontSize : _defaultFont.fontSize;
            int fontSpacing = style->fontStyle ? style->fontStyle->fontSpacing : _defaultFont.fontSpacing;
            Color textColor = style->textColor;

            while (1) {
                box = MeasureTextEx(font, text, fontSize, fontSpacing);
                if (box.x > availableSpace.x) {
                    int len = strlen(text);
                    text[len - 1] = '\0';
                    for (int i = len - 2, j = 2; i >= 0 && j >= 0; i--, j--) {
                        text[i] = '.';
                    }
                } else {
                    break;
                }
            }
            int x = (int)(entry->params.bounds.x + style->paddingLeft + (availableSpace.x - box.x) * style->textAlignment.x);
            int y = (int)(entry->params.bounds.y + style->paddingTop + (availableSpace.y - box.y) * style->textAlignment.y);
            DrawTextEx(font, text, (Vector2) { x, y }, fontSize, fontSpacing, textColor);
        }
    }
}

NPatchInfo GenNPatchInfo(int x, int y, int w, int h, int top, int right, int bottom, int left)
{
    NPatchInfo nPatchInfo = { 0 };
    nPatchInfo.source = (Rectangle) { x, y, w, h };
    nPatchInfo.left = left;
    nPatchInfo.top = top;
    nPatchInfo.right = right;
    nPatchInfo.bottom = bottom;
    nPatchInfo.layout = NPATCH_NINE_PATCH;
    return nPatchInfo;
}
#define MAX_STYLE_COUNT 64
static DuskGuiStyle _styleArray[MAX_STYLE_COUNT];
static int _styleGroupCount = 0;
DuskGuiStyle* DuskGui_createGuiStyle(DuskGuiStyle* fallbackStyle)
{
    if (_styleGroupCount >= MAX_STYLE_COUNT) {
        TraceLog(LOG_ERROR, "DuskGui_createGuiStyle: Maximum style groups reached");
        return NULL;
    }
    DuskGuiStyle* group = &_styleArray[_styleGroupCount++];
    *group = *fallbackStyle;
    return group;
}

static DuskGuiParamsEntry* DuskGui_getParentById(int parentIndex, int defaultToRoot)
{
    if (parentIndex <= 0) {
        return defaultToRoot ? &_duskGuiState.root : NULL;
    }
    if (parentIndex == 1)
    {
        return &_duskGuiState.root;
    }
    return &_duskGuiState.currentParams.params[parentIndex - 2];
}
static DuskGuiParamsEntry* DuskGui_getParent(DuskGuiParamsEntry* entry, int defaultToRoot)
{
    return DuskGui_getParentById(entry->parentIndex, defaultToRoot);
}

void DuskGui_init()
{
    DuskGui_setDefaultFont(GetFontDefault(), 10, 1);
    _duskGuiState.root.params.bounds = (Rectangle) { 0, 0, GetScreenWidth(), GetScreenHeight() };
    _duskGuiState.root.params.text = "root";
    _duskGuiState.root.txId = "root";
    _duskGuiState.root.parentIndex = 0;
    _duskGuiState.currentPanelIndex = 0;

    _duskGuiState.currentParams = (DuskGuiParamsList) {
        .params = (DuskGuiParamsEntry*)malloc(sizeof(DuskGuiParamsEntry) * 256),
        .count = 0,
        .capacity = 256
    };
    _duskGuiState.prevParams = (DuskGuiParamsList) {
        .params = (DuskGuiParamsEntry*)malloc(sizeof(DuskGuiParamsEntry) * 256),
        .count = 0,
        .capacity = 256
    };

    // create default texture procedurally
    int size = 16;
    Image defaultImage = GenImageColor(size, size, WHITE);
    Color* pixels = (Color*)defaultImage.data;
    float half = (float)(size - 1) / 2;
    const float a = 0.6f;
    const float b = 0.5f;
    for (int y = 0; y < size; y++) {
        float ybrightness = 1.1f - (float)y / (size - 1) * .4f;
        if (ybrightness < 0)
            ybrightness = 0;
        if (ybrightness > 1)
            ybrightness = 1;
        int ibrightness = (int)(ybrightness * 255.0f);
        for (int x = 0; x < size; x++) {
            float dx = (x - half) / half;
            float dy = (y - half) / half;
            float sqd = dx * dx + dy * dy;
            if (sqd < a) {
                float d = sqrt(sqd);
                float alpha = (a - d) * 8.0f;
                float brightness = (b - d) * 8.0f;
                if (alpha < 0)
                    alpha = 0;
                if (alpha > 1)
                    alpha = 1;
                if (brightness < 0)
                    brightness = 0;
                if (brightness > 1)
                    brightness = 1;
                brightness *= ybrightness;
                int color = (int)(brightness * 255.0f);
                pixels[y * size + x] = (Color) { color, color, color, (float)(255.0f * alpha) };
            } else {
                pixels[y * size + x] = (Color) { ibrightness, ibrightness, ibrightness, 0 };
            }
        }
    }

    DuskGuiStyleGroup* buttonGroup = &_defaultStyles.groups[DUSKGUI_STYLE_BUTTON];
    buttonGroup->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .textAlignment = (Vector2) { 0.5f, 0.5f },
        .paddingLeft = 4,
        .paddingRight = 4,
        .paddingTop = 6,
        .paddingBottom = 4,
        .backgroundColor = (Color) { 200, 200, 200, 255 },
        .textColor = BLACK,
    };
    
    Texture2D defaultTexture = LoadTextureFromImage(defaultImage);
    UnloadImage(defaultImage);
    buttonGroup->fallbackStyle.backgroundTexture = defaultTexture;
    SetTextureFilter(defaultTexture, TEXTURE_FILTER_BILINEAR);
    int o = 4;
    int w = size - o * 2;
    int h = size - o * 2;
    int n = w / 2;
    buttonGroup->fallbackStyle.backgroundPatchInfo = GenNPatchInfo(o, o, w, h, n, n, n, n);

    buttonGroup->normal = &buttonGroup->fallbackStyle;
    buttonGroup->hover = DuskGui_createGuiStyle(buttonGroup->normal);
    buttonGroup->hover->backgroundColor = (Color) { 200, 220, 250, 255 };
    buttonGroup->pressed = DuskGui_createGuiStyle(buttonGroup->normal);
    buttonGroup->pressed->backgroundColor = (Color) { 200, 200, 200, 255 };
    buttonGroup->pressed->textColor = RED;


    DuskGuiStyleGroup* panelGroup = &_defaultStyles.groups[DUSKGUI_STYLE_PANEL];
    panelGroup->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .paddingLeft = 4,
        .paddingRight = 4,
        .paddingTop = 4,
        .paddingBottom = 4,
        .backgroundColor = (Color) { 220, 220, 220, 255 },
        .textColor = BLACK,
        .backgroundTexture = defaultTexture,
        .backgroundPatchInfo = GenNPatchInfo(o, o, w, h, n, n, n, n),
    };

    DuskGuiStyleGroup* labelGroup = &_defaultStyles.groups[DUSKGUI_STYLE_LABEL];
    labelGroup->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .textAlignment = (Vector2) { 0.0f, 0.5f },
        .textColor = BLACK,
    };

    DuskGuiStyleGroup* labelButtonGroup = &_defaultStyles.groups[DUSKGUI_STYLE_LABELBUTTON];
    labelButtonGroup->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .textAlignment = (Vector2) { 0.0f, 0.5f },
        .textColor = BLACK,
    };
    labelButtonGroup->normal = &labelButtonGroup->fallbackStyle;
    labelButtonGroup->hover = DuskGui_createGuiStyle(&labelButtonGroup->fallbackStyle);
    labelButtonGroup->hover->textColor = (Color) { 80, 80, 128, 255 };
    labelButtonGroup->pressed = DuskGui_createGuiStyle(&labelButtonGroup->fallbackStyle);
    labelButtonGroup->pressed->textColor = (Color) { 100, 100, 255, 255 };

    Image foldoutImage = GenImageColor(16, 16, (Color) { 255, 255, 255, 0 });
    Color* foldoutPixels = (Color*)foldoutImage.data;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (x - y < 0 && x + y < 16)
                foldoutPixels[y * 16 + x] = (Color) { 255, 255, 255, 255 };
        }
    }
    Texture2D foldoutIcon = LoadTextureFromImage(foldoutImage);
    UnloadImage(foldoutImage);

    DuskGuiStyleGroup* foldoutGroup = &_defaultStyles.groups[DUSKGUI_STYLE_FOLDOUT_OPEN];
    foldoutGroup->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .textAlignment = (Vector2) { 0.0f, 0.5f },
        .paddingLeft = 16,
        .textColor = BLACK,
        .iconTexture = foldoutIcon,
        .iconSize = (Vector2) { 16, 16 },
        .iconPivot = (Vector2) { 8, 8 },
        .iconOffset = (Vector2) { 2, 0 },
        .iconAlignment = (Vector2) { 0, 0.5f },
        .iconRotationDegrees = 0,
    };
    foldoutGroup->normal = &foldoutGroup->fallbackStyle;
    foldoutGroup->hover = DuskGui_createGuiStyle(&foldoutGroup->fallbackStyle);
    foldoutGroup->hover->textColor = (Color) { 80, 80, 128, 255 };
    foldoutGroup->pressed = DuskGui_createGuiStyle(&foldoutGroup->fallbackStyle);
    foldoutGroup->pressed->textColor = (Color) { 100, 100, 255, 255 };

    Image horizontalImage = GenImageColor(8, 8, WHITE);
    Texture2D horizontalLineTexture = LoadTextureFromImage(horizontalImage);
    UnloadImage(horizontalImage);
    DuskGuiStyleGroup* horizontalLineGroup = &_defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_LINE];
    horizontalLineGroup->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .paddingLeft = 2,
        .paddingRight = 2,
        .paddingTop = 0,
        .paddingBottom = 0,
        .backgroundColor = (Color) { 90, 90, 90, 255 },
        .textColor = (Color) { 170, 170, 170, 255 },
        .textAlignment = (Vector2) { 0.0f, 0.5f },
        .backgroundTexture = horizontalLineTexture,
        .backgroundPatchInfo = GenNPatchInfo(0, 0, 4, 4, 2, 2, 2, 2),
    };
    horizontalLineGroup->normal = &horizontalLineGroup->fallbackStyle;
}

DuskGuiStyleGroup* DuskGui_getStyle(int styleType)
{
    return &_defaultStyles.groups[styleType];
}

int DuskGui_hasLock(DuskGuiParamsEntry* params)
{
    return _duskGuiState.locked.txId != NULL && DuskGui_idEquals(&_duskGuiState.locked, params);
}

int DuskGui_isNotLocked(DuskGuiParamsEntry* params)
{
    return _duskGuiState.locked.txId == NULL || DuskGui_idEquals(&_duskGuiState.locked, params);
}

int DuskGui_tryLock(DuskGuiParamsEntry* params)
{
    if (_duskGuiState.locked.txId != NULL) {
        return strcmp(_duskGuiState.locked.txId, params->txId) == 0;
    }

    if (_duskGuiState.locked.txId) {
        free(_duskGuiState.locked.txId);
    }
    _duskGuiState.locked = *params;
    _duskGuiState.locked.txId = strdup(params->txId);
    return 1;
}

void DuskGui_unlock()
{
    if (_duskGuiState.locked.txId) {
        free(_duskGuiState.locked.txId);
    }
    _duskGuiState.locked.txId = NULL;
}

static DuskGuiParamsEntry* DuskGui_addParams(DuskGuiParamsList* paramsList, DuskGuiParamsEntry* params)
{
    int index = params->id;

    if (index >= paramsList->capacity) {
        paramsList->capacity *= 2;
        paramsList->params = (DuskGuiParamsEntry*)realloc(paramsList->params, sizeof(DuskGuiParamsEntry) * paramsList->capacity);
    }
    if (index >= paramsList->count) {
        // memset 0 empty entries
        for (int i = paramsList->count; i < index; i++) {
            paramsList->params[i] = (DuskGuiParamsEntry) { 0 };
        }
        paramsList->count = index + 1;
    }
    paramsList->params[index] = *params;
    return &paramsList->params[index];
}

void DuskGui_evaluate()
{
    _duskGuiState.idCounter = 0;
    // Swap current and previous params
    DuskGuiParamsList tmp = _duskGuiState.prevParams;
    _duskGuiState.prevParams = _duskGuiState.currentParams;
    _duskGuiState.currentParams = tmp;
    // free entries
    for (int i = 0; i < _duskGuiState.currentParams.count; i++) {
        if (_duskGuiState.currentParams.params[i].txId) {
            free(_duskGuiState.currentParams.params[i].txId);
        }
    }
    _duskGuiState.currentParams.count = 0;

    // reversed raycast hit test
    int blocked = 0;
    const int maxBlockingParents = 64;
    DuskGuiParamsEntry* blockingParents[maxBlockingParents];
    int blockingParentsCount = 0;
    for (int i = _duskGuiState.prevParams.count - 1; i >= 0; i--) {
        DuskGuiParamsEntry* entry = &_duskGuiState.prevParams.params[i];
        if (entry->params.rayCastTarget == 0)
            continue;
        int isHit = CheckCollisionPointRec(GetMousePosition(), entry->params.bounds);
        if (isHit && blocked) {
            isHit = 0;
            for (int j = 0; j < blockingParentsCount; j++) {
                // printf("%i blocked by %s, checking if parent %s\n", j, entry->txId, blockingParents[j]->txId);
                if (DuskGui_idEquals(blockingParents[j], entry)) {
                    // printf("parent found\n");
                    entry->isMouseOver = 1;
                    break;
                }
            }
        }

        if (isHit) {
            DuskGuiParamsEntry* parent = entry;
            while (parent != NULL) {
                if (blockingParentsCount == maxBlockingParents) {
                    printf("Warning, parent chain too long\n");
                    break;
                }
                blockingParents[blockingParentsCount++] = parent;
                parent = DuskGui_getParent(parent, 0);
            }
            blocked = 1;
        }

        if (isHit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            DuskGui_tryLock(entry);
        }

        if (isHit && IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !DuskGui_hasLock(entry)) {
            isHit = 0;
        }

        entry->isHovered = isHit;
        if (!entry->isMouseOver) {
            entry->isMouseOver = isHit;
        }

        entry->isPressed = entry->isHovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        entry->isTriggered = isHit && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && DuskGui_hasLock(entry);
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        DuskGui_unlock();
    }

    _duskGuiState.currentPanelIndex = 0;
    _duskGuiState.root.params.bounds = (Rectangle) { 0, 0, GetScreenWidth(), GetScreenHeight() };
}

static DuskGuiParamsEntry* DuskGui_addParams(DuskGuiParamsList* paramsList, DuskGuiParamsEntry* params);
void DuskGui_update(DuskGuiParamsEntry *entry);
static DuskGuiParamsEntry* DuskGui_getCurrentPanel()
{
    return DuskGui_getParentById(_duskGuiState.currentPanelIndex, 1);
}

static DuskGuiParamsEntry* DuskGui_makeEntry(DuskGuiParams params)
{
    DuskGuiParamsEntry* parent = DuskGui_getCurrentPanel();
    params.bounds.x += parent->params.bounds.x;
    params.bounds.y += parent->params.bounds.y;

    params.bounds.x += parent->contentOffset.x;
    params.bounds.y += parent->contentOffset.y;
    DuskGuiParamsEntry entry = {
        .id = _duskGuiState.idCounter++,
        .txId = NULL,
        .params = params,
        .parentIndex = _duskGuiState.currentPanelIndex,
    };
    if (params.text) {
        const char* text = params.text;

        for (int i = 0; params.text[i]; i++) {
            if (text[i] == '#' && text[i + 1] == '#') {
                text = &text[i + 2];
                break;
            }
        }
        entry.txId = strdup(text);
    }

    DuskGuiParamsEntry* added = DuskGui_addParams(&_duskGuiState.currentParams, &entry);
    DuskGui_update(added);
    return added;
}

void DuskGui_update(DuskGuiParamsEntry *entry)
{
    DuskGuiParamsEntry* match = DuskGui_findParams(&_duskGuiState.prevParams, entry);
    if (match) {
        entry->isMouseOver = match->isMouseOver;
        entry->isHovered = match->isHovered;
        entry->isPressed = match->isPressed;
        entry->isTriggered = match->isTriggered;
        entry->isFolded = match->isFolded;
        entry->contentOffset = match->contentOffset;
        entry->contentSize = match->contentSize;
    }
}

void DuskGui_setContentOffset(DuskGuiParamsEntry entry, Vector2 contentOffset)
{
    DuskGuiParamsEntry* match = DuskGui_findParams(&_duskGuiState.currentParams, &entry);
    if (match) {
        match->contentOffset = contentOffset;
    }
}
void DuskGui_setContentSize(DuskGuiParamsEntry entry, Vector2 contentSize)
{
    DuskGuiParamsEntry* match = DuskGui_findParams(&_duskGuiState.currentParams, &entry);
    if (match) {
        match->contentSize = contentSize;
    }
}

int DuskGui_dragArea(DuskGuiParams params)
{
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params);
    DuskGui_addParams(&_duskGuiState.currentParams, entry);
    DuskGui_update(entry);
    return DuskGui_hasLock(entry);
}

static void DuskGui_drawStyle(DuskGuiParamsEntry* params, DuskGuiState* state, DuskGuiStyleGroup* styleGroup)
{
    if (params->params.styleGroup && params->params.styleGroup->draw) {
        params->params.styleGroup->draw(params, state, styleGroup);
    } else {
        DuskGui_defaultDrawStyle(params, state, styleGroup);
    }
}

static int DuskGui_toParentIndex(DuskGuiParamsEntry* entry)
{
    return entry->id + 2;
}

DuskGuiParamsEntry* DuskGui_beginScrollArea(DuskGuiParams params)
{
    DuskGuiParamsEntry *entry = DuskGui_makeEntry(params);
    _duskGuiState.currentPanelIndex = DuskGui_toParentIndex(entry);

    if (entry->isMouseOver) {
        Vector2 wheelMove = GetMouseWheelMoveV();
        entry->contentOffset.y += wheelMove.y * 15.0f;
        if (-entry->contentOffset.y > entry->contentSize.y - entry->params.bounds.height) {
            entry->contentOffset.y = -(entry->contentSize.y - entry->params.bounds.height);
        }
        if (entry->contentOffset.y > 0 || entry->contentSize.y <= entry->params.bounds.height) {
            entry->contentOffset.y = 0;
        }
    }

    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_PANEL]);

    // printf("beg sci: %f %f %f %f %d %d\n", entry.params.bounds.x, entry.params.bounds.y, entry.params.bounds.width, entry.params.bounds.height, GetScreenWidth(), GetScreenHeight());
    BeginScissorMode((int)entry->params.bounds.x, (int)entry->params.bounds.y, (int)entry->params.bounds.width, (int)entry->params.bounds.height);
    return entry;
}
void DuskGui_endScrollArea(DuskGuiParamsEntry* entry)
{
    EndScissorMode();
    _duskGuiState.currentPanelIndex = entry->parentIndex;
    DuskGuiParamsEntry *currentPanel = DuskGui_getParent(entry, 1);
    if (currentPanel != &_duskGuiState.root) {
        Rectangle bounds = currentPanel->params.bounds;
        // printf("end sci: %f %f %f %f\n", bounds.x, bounds.y, bounds.width, bounds.height);
        BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);
    }
}

DuskGuiParamsEntry* DuskGui_beginPanel(DuskGuiParams params)
{
    DuskGuiParamsEntry *entry = DuskGui_makeEntry(params);
    DuskGui_addParams(&_duskGuiState.currentParams, entry);
    _duskGuiState.currentPanelIndex = DuskGui_toParentIndex(entry);

    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_PANEL]);

    BeginScissorMode((int)entry->params.bounds.x, (int)entry->params.bounds.y, (int)entry->params.bounds.width, (int)entry->params.bounds.height);
    return entry;
}

void DuskGui_endPanel(DuskGuiParamsEntry* entry)
{
    EndScissorMode();
    _duskGuiState.currentPanelIndex = entry->parentIndex;
    DuskGuiParamsEntry *currentPanel = DuskGui_getParent(entry, 1);
    if (currentPanel != &_duskGuiState.root) {
        Rectangle bounds = currentPanel->params.bounds;
        BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);
    }
}

int DuskGui_foldout(DuskGuiParams params)
{
    DuskGuiParamsEntry *entry = DuskGui_makeEntry(params);
    
    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_FOLDOUT_OPEN]);
    if (entry->isTriggered) {
        entry->isFolded = !entry->isFolded;
    }

    return !entry->isFolded;
}

int DuskGui_label(DuskGuiParams params)
{
    DuskGuiParamsEntry *entry = DuskGui_makeEntry(params);
    
    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_LABEL]);
    return entry->isTriggered;
}

void DuskGui_horizontalLine(DuskGuiParams params)
{
    DuskGuiParamsEntry *entry = DuskGui_makeEntry(params);
    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_LINE]);
}

int DuskGui_button(DuskGuiParams params)
{
    DuskGuiParamsEntry *entry = DuskGui_makeEntry(params);
    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_BUTTON]);
    return entry->isTriggered;
}

Vector2 DuskGui_getAvailableSpace()
{
    DuskGuiParamsEntry *panel = DuskGui_getCurrentPanel();
    Rectangle rect = panel->params.bounds;
    return (Vector2) { rect.width, rect.height };
}