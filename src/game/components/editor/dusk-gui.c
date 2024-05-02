#include "dusk-gui.h"
#include "raymath.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static DuskGuiState _duskGuiState;

static Color ColorLerp(Color a, Color b, float t)
{
    if (t <= 0)
        return a;
    if (t >= 1)
        return b;
    return (Color) {
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)
    };
}

static int IntLerp(int a, int b, float t)
{
    if (t <= 0)
        return a;
    if (t >= 1)
        return b;
    return (int)(a + (b - a) * t);
}

static DuskGuiStyle DuskGuiStyle_lerp(DuskGuiStyle* a, DuskGuiStyle* b, float t)
{
    if (t >= 1.0f)
        return *b;
    if (t <= 0.0f)
        return *a;
    DuskGuiStyle style = t < .5f ? *a : *b;
    style.transitionLingerTime = Lerp(a->transitionLingerTime, b->transitionLingerTime, t);
    style.transitionActivationTime = Lerp(a->transitionActivationTime, b->transitionActivationTime, t);
    style.textAlignment = Vector2Lerp(a->textAlignment, b->textAlignment, t);
    style.paddingLeft = IntLerp(a->paddingLeft, b->paddingLeft, t);
    style.paddingRight = IntLerp(a->paddingRight, b->paddingRight, t);
    style.paddingTop = IntLerp(a->paddingTop, b->paddingTop, t);
    style.paddingBottom = IntLerp(a->paddingBottom, b->paddingBottom, t);
    style.textColor = ColorLerp(a->textColor, b->textColor, t);
    style.backgroundColor = ColorLerp(a->backgroundColor, b->backgroundColor, t);
    style.iconColor = ColorLerp(a->iconColor, b->iconColor, t);
    style.iconSize = Vector2Lerp(a->iconSize, b->iconSize, t);
    style.iconPivot = Vector2Lerp(a->iconPivot, b->iconPivot, t);
    style.iconOffset = Vector2Lerp(a->iconOffset, b->iconOffset, t);
    style.iconAlignment = Vector2Lerp(a->iconAlignment, b->iconAlignment, t);
    style.iconRotationDegrees = Lerp(a->iconRotationDegrees, b->iconRotationDegrees, t);
    return style;
}

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

static DuskGuiStyle* DuskGuiStyleGroup_selectStyle(DuskGuiParamsEntry* entry, DuskGuiStyleGroup* styleGroup)
{
    DuskGuiStyleGroup* group = entry->params.styleGroup ? entry->params.styleGroup : styleGroup;
    DuskGuiStyle* style = group->normal ? group->normal : &group->fallbackStyle;
    if (entry->isHovered && group->hover) {
        style = group->hover;
    }
    if (entry->isFocused && group->focused) {
        style = group->focused;
    }
    if (entry->isPressed && group->pressed) {
        style = group->pressed;
    }
    if (entry->isTriggered && group->active) {
        style = group->active;
    }
    return style;
}

static void DuskGui_defaultDrawStyle(DuskGuiParamsEntry* entry, DuskGuiState* state, DuskGuiStyleGroup* styleGroup)
{
    int dbg = strcmp(entry->txId, "Vector3 points[4]") == 0;
    DuskGuiStyle* style = DuskGuiStyleGroup_selectStyle(entry, styleGroup);
    DuskGuiStyle lerpedStyle = *style;
    float t = entry->transitionTime / entry->transitionDuration;
    if (t < 1) {
        lerpedStyle = DuskGuiStyle_lerp(&entry->cachedStartStyle, style, t);
    }
    if (entry->nextStyle != style) {
        entry->cachedStartStyle = DuskGuiStyle_lerp(&entry->cachedStartStyle, entry->nextStyle ? entry->nextStyle : &styleGroup->fallbackStyle, t);
        entry->origStartStyle = entry->nextStyle;
        entry->nextStyle = style;
        entry->transitionTime = 0;
        entry->transitionDuration = style->transitionActivationTime + entry->cachedStartStyle.transitionLingerTime;
        lerpedStyle = entry->cachedStartStyle;
    }

    if (lerpedStyle.backgroundColor.a > 0) {
        if (lerpedStyle.backgroundTexture.id == 0) {
            DrawRectangleRec(entry->params.bounds, lerpedStyle.backgroundColor);
        } else {
            DrawTextureNPatch(lerpedStyle.backgroundTexture, lerpedStyle.backgroundPatchInfo, entry->params.bounds, (Vector2) { 0, 0 }, 0, lerpedStyle.backgroundColor);
        }
    }

    if (lerpedStyle.iconColor.a > 0 && lerpedStyle.iconTexture.id != 0) {
        Vector2 iconSize = lerpedStyle.iconSize;
        Vector2 iconPivot = lerpedStyle.iconPivot;
        Vector2 iconOffset = lerpedStyle.iconOffset;
        Vector2 iconAlignment = lerpedStyle.iconAlignment;
        float iconRotation = lerpedStyle.iconRotationDegrees;
        Vector2 iconPosition = (Vector2) {
            (entry->params.bounds.width - iconSize.x) * iconAlignment.x + entry->params.bounds.x + iconOffset.x,
            (entry->params.bounds.height - iconSize.y) * iconAlignment.y + entry->params.bounds.y + iconOffset.y
        };
        DrawTexturePro(lerpedStyle.iconTexture, (Rectangle) { 0, 0, lerpedStyle.iconTexture.width, lerpedStyle.iconTexture.height },
            (Rectangle) { iconPosition.x + iconPivot.x, iconPosition.y + iconPivot.y, iconSize.x, iconSize.y },
            iconPivot, iconRotation, lerpedStyle.iconColor);
    }

    if (entry->params.text && lerpedStyle.textColor.a > 0) {
        // draw debug outline
        // DrawRectangleLines(entry->params.bounds.x, entry->params.bounds.y, entry->params.bounds.width, entry->params.bounds.height, RED);

        Font font = lerpedStyle.fontStyle ? lerpedStyle.fontStyle->font : _defaultFont.font;
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
                entry->params.bounds.width - lerpedStyle.paddingLeft - lerpedStyle.paddingRight,
                entry->params.bounds.height - lerpedStyle.paddingTop - lerpedStyle.paddingBottom
            };

            int fontSize = lerpedStyle.fontStyle ? lerpedStyle.fontStyle->fontSize : _defaultFont.fontSize;
            int fontSpacing = lerpedStyle.fontStyle ? lerpedStyle.fontStyle->fontSpacing : _defaultFont.fontSpacing;
            Color textColor = lerpedStyle.textColor;

            box = MeasureTextEx(font, text, fontSize, fontSpacing);
            while (style->textOverflowType == DUSKGUI_OVERFLOW_ELLIPSIS && box.x > availableSpace.x) {
                int len = strlen(text);
                if (len < 1)
                    break;

                text[len - 1] = '\0';
                for (int i = len - 2, j = 2; i >= 0 && j >= 0; i--, j--) {
                    text[i] = '.';
                }
                box = MeasureTextEx(font, text, fontSize, fontSpacing);
            }
            // int contained = box.x >= entry->params.bounds.x && box.x <= entry->params.bounds.width + entry->params.bounds.x;

            int x = (int)(entry->params.bounds.x + lerpedStyle.paddingLeft + (availableSpace.x - box.x) * lerpedStyle.textAlignment.x) + entry->textOffset.x;
            int y = (int)(entry->params.bounds.y + lerpedStyle.paddingTop + (availableSpace.y - box.y) * lerpedStyle.textAlignment.y) + entry->textOffset.y;
            DrawTextEx(font, text, (Vector2) { x, y }, fontSize, fontSpacing, textColor);

            entry->textBounds = (Rectangle) { x, y, box.x, box.y };
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
    if (parentIndex == 1) {
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
        .transitionLingerTime = 0.033f,
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
    buttonGroup->hover->transitionLingerTime = 0.25f;
    buttonGroup->pressed = DuskGui_createGuiStyle(buttonGroup->normal);
    buttonGroup->pressed->backgroundColor = (Color) { 200, 200, 200, 255 };
    buttonGroup->pressed->textColor = RED;
    buttonGroup->pressed->transitionActivationTime = -.15f;

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
        .transitionLingerTime = 0.033f,
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
        .transitionLingerTime = 0.01f,
    };
    labelButtonGroup->normal = &labelButtonGroup->fallbackStyle;
    labelButtonGroup->hover = DuskGui_createGuiStyle(&labelButtonGroup->fallbackStyle);
    labelButtonGroup->hover->transitionActivationTime = 0.01f;
    labelButtonGroup->hover->transitionLingerTime = 0.3f;
    labelButtonGroup->hover->textColor = (Color) { 80, 80, 128, 255 };
    labelButtonGroup->pressed = DuskGui_createGuiStyle(&labelButtonGroup->fallbackStyle);
    labelButtonGroup->pressed->textColor = (Color) { 100, 100, 255, 255 };
    labelButtonGroup->pressed->transitionActivationTime = -0.25f;

    Image foldoutImage = GenImageColor(16, 16, (Color) { 255, 255, 255, 0 });
    Color* foldoutPixels = (Color*)foldoutImage.data;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (x - y < 0 && x + y < 16 && x > 2)
                foldoutPixels[y * 16 + x] = (Color) { 255, 255, 255, 255 };
        }
    }
    Texture2D foldoutIcon = LoadTextureFromImage(foldoutImage);
    UnloadImage(foldoutImage);
    SetTextureFilter(foldoutIcon, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(foldoutIcon, TEXTURE_WRAP_CLAMP);

    DuskGuiStyleGroup* foldoutOpen = &_defaultStyles.groups[DUSKGUI_STYLE_FOLDOUT_OPEN];
    foldoutOpen->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .textAlignment = (Vector2) { 0.0f, 0.5f },
        .paddingLeft = 10,
        .textColor = BLACK,
        .iconTexture = foldoutIcon,
        .iconColor = BLACK,
        .iconSize = (Vector2) { 16, 16 },
        .iconPivot = (Vector2) { 5, 8 },
        .iconOffset = (Vector2) { -2, -1 },
        .iconAlignment = (Vector2) { 0, 0.5f },
        .iconRotationDegrees = 90,
        .transitionLingerTime = 0.033f,
    };
    foldoutOpen->normal = &foldoutOpen->fallbackStyle;
    foldoutOpen->hover = DuskGui_createGuiStyle(&foldoutOpen->fallbackStyle);
    foldoutOpen->hover->iconRotationDegrees = 70.0f;
    foldoutOpen->hover->textColor = (Color) { 80, 80, 128, 255 };
    foldoutOpen->pressed = DuskGui_createGuiStyle(&foldoutOpen->fallbackStyle);
    foldoutOpen->pressed->textColor = (Color) { 100, 100, 255, 255 };
    foldoutOpen->pressed->iconRotationDegrees = 45;

    DuskGuiStyleGroup* foldoutClosed = &_defaultStyles.groups[DUSKGUI_STYLE_FOLDOUT_CLOSED];
    foldoutClosed->fallbackStyle = foldoutOpen->fallbackStyle;
    foldoutClosed->fallbackStyle.iconRotationDegrees = 0;
    foldoutClosed->normal = &foldoutClosed->fallbackStyle;
    foldoutClosed->hover = DuskGui_createGuiStyle(&foldoutClosed->fallbackStyle);
    foldoutClosed->hover->iconRotationDegrees = 20.0f;
    foldoutClosed->hover->textColor = (Color) { 80, 80, 128, 255 };
    foldoutClosed->pressed = DuskGui_createGuiStyle(&foldoutClosed->fallbackStyle);
    foldoutClosed->pressed->textColor = (Color) { 100, 100, 255, 255 };
    foldoutClosed->pressed->iconRotationDegrees = 45;

    Image textFieldPatch9 = GenImageColor(16, 16, (Color) { 255, 255, 255, 255 });
    Color* textFieldPixels = (Color*)textFieldPatch9.data;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (y > 13) {
                textFieldPixels[y * 16 + x] = (Color) { 240, 240, 240, 255 };
            }
            if (x == 0 || x == 15 || y == 0 || y == 15) {
                textFieldPixels[y * 16 + x] = (Color) { 0, 0, 0, 255 };
            }
        }
    }
    Texture2D textFieldTexture = LoadTextureFromImage(textFieldPatch9);
    UnloadImage(textFieldPatch9);
    SetTextureFilter(textFieldTexture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(textFieldTexture, TEXTURE_WRAP_CLAMP);
    Image textFieldCursor = GenImageColor(2, 16, (Color) { 255, 255, 255, 255 });
    Texture2D cursorTexture = LoadTextureFromImage(textFieldCursor);
    UnloadImage(textFieldCursor);

    DuskGuiStyleGroup* textFieldGroup = &_defaultStyles.groups[DUSKGUI_STYLE_INPUTTEXTFIELD];
    textFieldGroup->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .textAlignment = (Vector2) { 0.0f, 0.5f },
        .paddingLeft = 4,
        .paddingRight = 4,
        .paddingTop = 4,
        .paddingBottom = 4,
        .backgroundColor = (Color) { 255, 255, 255, 255 },
        .textColor = BLACK,
        .backgroundTexture = textFieldTexture,
        .backgroundPatchInfo = GenNPatchInfo(0, 0, 16, 16, 4, 4, 4, 4),
        .transitionLingerTime = 0.033f,
        .iconTexture = cursorTexture,
        .iconColor = (Color) { 0, 0, 0, 0 },
        .iconSize = (Vector2) { 2, 16 },
    };
    textFieldGroup->normal = &textFieldGroup->fallbackStyle;
    textFieldGroup->hover = DuskGui_createGuiStyle(&textFieldGroup->fallbackStyle);
    textFieldGroup->hover->backgroundColor = (Color) { 240, 240, 240, 255 };
    textFieldGroup->hover->iconColor = BLACK;
    textFieldGroup->pressed = DuskGui_createGuiStyle(&textFieldGroup->fallbackStyle);
    textFieldGroup->pressed->backgroundColor = (Color) { 220, 220, 220, 255 };
    textFieldGroup->pressed->iconColor = BLACK;
    textFieldGroup->focused = DuskGui_createGuiStyle(&textFieldGroup->fallbackStyle);
    textFieldGroup->focused->backgroundColor = (Color) { 250, 200, 200, 255 };
    textFieldGroup->focused->iconColor = BLACK;
    

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
        .transitionLingerTime = 0.033f,
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

static DuskGuiParamsEntry* DuskGui_getFocusedEntry()
{
    for (int i = 0; i < _duskGuiState.currentParams.count; i++) {
        if (_duskGuiState.currentParams.params[i].isFocused) {
            return &_duskGuiState.currentParams.params[i];
        }
    }
    return NULL;
}

void DuskGui_evaluate()
{
    _duskGuiState.idCounter = 0;

    // reversed raycast hit test
    int blocked = 0;
    const int maxBlockingParents = 64;
    DuskGuiParamsEntry* blockingParents[maxBlockingParents];
    int blockingParentsCount = 0;
    for (int i = _duskGuiState.currentParams.count - 1; i >= 0; i--) {
        DuskGuiParamsEntry* entry = &_duskGuiState.currentParams.params[i];
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
        if (entry->isTriggered && entry->params.isFocusable) {
            DuskGuiParamsEntry* currentFocus = DuskGui_getFocusedEntry();
            if (currentFocus) {
                currentFocus->isFocused = 0;
            }
            entry->isFocused = 1;
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        DuskGui_unlock();
    }

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

    _duskGuiState.currentPanelIndex = 0;
    _duskGuiState.root.params.bounds = (Rectangle) { 0, 0, GetScreenWidth(), GetScreenHeight() };
}

static DuskGuiParamsEntry* DuskGui_addParams(DuskGuiParamsList* paramsList, DuskGuiParamsEntry* params);
void DuskGui_update(DuskGuiParamsEntry* entry, DuskGuiStyleGroup* initStyle);
static DuskGuiParamsEntry* DuskGui_getCurrentPanel()
{
    return DuskGui_getParentById(_duskGuiState.currentPanelIndex, 1);
}

static DuskGuiParamsEntry* DuskGui_makeEntry(DuskGuiParams params, DuskGuiStyleGroup* initStyleGroup)
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
    DuskGui_update(added, initStyleGroup);
    return added;
}

void DuskGui_update(DuskGuiParamsEntry* entry, DuskGuiStyleGroup* initStyleGroup)
{
    DuskGuiParamsEntry* match = DuskGui_findParams(&_duskGuiState.prevParams, entry);
    if (match) {
        entry->isMouseOver = match->isMouseOver;
        entry->isHovered = match->isHovered;
        entry->isPressed = match->isPressed;
        entry->isTriggered = match->isTriggered;
        entry->isFolded = match->isFolded;
        entry->isFocused = match->isFocused;
        entry->contentOffset = match->contentOffset;
        entry->contentSize = match->contentSize;

        entry->cursorIndex = match->cursorIndex;
        entry->selectionStart = match->selectionStart;
        entry->selectionEnd = match->selectionEnd;
        entry->textOffset = match->textOffset;

        entry->cachedStartStyle = match->cachedStartStyle;
        entry->origStartStyle = match->origStartStyle;
        entry->nextStyle = match->nextStyle;
        entry->transitionTime = match->transitionTime + GetFrameTime();
        entry->transitionDuration = match->transitionDuration;
    } else if (initStyleGroup) {
        entry->cachedStartStyle = initStyleGroup->fallbackStyle;
        entry->origStartStyle = initStyleGroup->normal;
        entry->nextStyle = initStyleGroup->normal;
        entry->transitionTime = 0;
        entry->transitionDuration = 1.0f;
        for (int i=0;entry->params.text[i];i++)
        {
            entry->cursorIndex = i;
            if (entry->params.text[i] == '#' && entry->params.text[i + 1] == '#') {
                break;
            }
        }
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
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, NULL);
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
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, &_defaultStyles.groups[DUSKGUI_STYLE_PANEL]);
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
    DuskGuiParamsEntry* currentPanel = DuskGui_getParent(entry, 1);
    if (currentPanel != &_duskGuiState.root) {
        Rectangle bounds = currentPanel->params.bounds;
        // printf("end sci: %f %f %f %f\n", bounds.x, bounds.y, bounds.width, bounds.height);
        BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);
    }
}

DuskGuiParamsEntry* DuskGui_beginPanel(DuskGuiParams params)
{
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, &_defaultStyles.groups[DUSKGUI_STYLE_PANEL]);
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
    DuskGuiParamsEntry* currentPanel = DuskGui_getParent(entry, 1);
    if (currentPanel != &_duskGuiState.root) {
        Rectangle bounds = currentPanel->params.bounds;
        BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);
    }
}

int DuskGui_foldout(DuskGuiParams params)
{
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, &_defaultStyles.groups[DUSKGUI_STYLE_FOLDOUT_OPEN]);
    DuskGuiStyleGroup* styleGroup = &_defaultStyles.groups[entry->isFolded ? DUSKGUI_STYLE_FOLDOUT_CLOSED : DUSKGUI_STYLE_FOLDOUT_OPEN];
    if (entry->isTriggered) {
        entry->isFolded = !entry->isFolded;
    }
    DuskGui_drawStyle(entry, &_duskGuiState, styleGroup);

    return !entry->isFolded;
}

int DuskGui_label(DuskGuiParams params)
{
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, &_defaultStyles.groups[DUSKGUI_STYLE_LABEL]);

    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_LABEL]);
    return entry->isTriggered;
}

int DuskGui_textInputField(DuskGuiParams params, char** buffer)
{
    DuskGuiStyleGroup* group = &_defaultStyles.groups[DUSKGUI_STYLE_INPUTTEXTFIELD];
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, group);
    DuskGui_drawStyle(entry, &_duskGuiState, group);
    if (entry->isFocused && entry->params.text && entry->params.text[0] != '\0') {
        const char* text = entry->params.text;
        int len = strlen(text);
        char copy[len + 1];
        strcpy(copy, text);
        for (int i=0;copy[i];i++) {
            if (copy[i] == '#' && copy[i + 1] == '#') {
                copy[i] = '\0';
                len = i;
                break;
            }
        }
        int cursorIndex = entry->cursorIndex;
        if (cursorIndex < 0) {
            cursorIndex = 0;
        }
        if (cursorIndex >= len) {
            cursorIndex = len;
        }
        Rectangle textBounds = entry->textBounds;
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, entry->params.bounds)) {
            // TODO: Fix this. It's badly written and not UTF-8 compatible 🥴
            if (CheckCollisionPointRec(mousePos, textBounds))
            {
                char first = copy[0];
                char start[2]= {first, 0};
                Vector2 firstSize = MeasureTextEx(group->fallbackStyle.fontStyle->font, start, group->fallbackStyle.fontStyle->fontSize, group->fallbackStyle.fontStyle->fontSpacing);

                Rectangle boundCheck = textBounds;
                boundCheck.width -= firstSize.x * .5f;
                boundCheck.x += firstSize.x * .5f;
                if (!CheckCollisionPointRec(mousePos, boundCheck)) {
                    cursorIndex = 0;
                }
                else
                {
                    Rectangle prevBound = boundCheck;
                    for (int i = len - 1; i >= 0; i--) {
                        char chr = copy[i];
                        copy[i] = '\0';
                        boundCheck.width = MeasureTextEx(group->fallbackStyle.fontStyle->font, copy, group->fallbackStyle.fontStyle->fontSize, group->fallbackStyle.fontStyle->fontSpacing).x;
                        Rectangle check = boundCheck;
                        check.width = Lerp(prevBound.width, boundCheck.width, 0.5f);
                        prevBound = boundCheck;
                        cursorIndex = i + 1;
                        if (!CheckCollisionPointRec(mousePos, check)) {
                            copy[i] = chr;
                            break;
                        }
                    }
                    copy[0] = first;
                }
            }
            else if (mousePos.x < textBounds.x)
            {
                cursorIndex = 0;
            }
            else if (mousePos.x > textBounds.x + textBounds.width)
            {
                cursorIndex = len;
            }
        }
        entry->cursorIndex = cursorIndex;
        copy[cursorIndex] = '\0';
        DuskGuiStyle* style = DuskGuiStyleGroup_selectStyle(entry, group);
        Vector2 cursorPos = MeasureTextEx(style->fontStyle->font, copy, style->fontStyle->fontSize, style->fontStyle->fontSpacing);
        Rectangle dst = (Rectangle) { textBounds.x + cursorPos.x + style->iconOffset.x, textBounds.y + style->iconOffset.y, style->iconSize.x, style->iconSize.y };
        DrawTexturePro(style->iconTexture, (Rectangle) { 0, 0, style->iconTexture.width, style->iconTexture.height },
            dst,
            (Vector2) { 0, 0 },
            0, style->iconColor);
    }
    return entry->isTriggered;
}

void DuskGui_horizontalLine(DuskGuiParams params)
{
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, &_defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_LINE]);
    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_LINE]);
}

int DuskGui_button(DuskGuiParams params)
{
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, &_defaultStyles.groups[DUSKGUI_STYLE_BUTTON]);
    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_BUTTON]);
    return entry->isTriggered;
}

Vector2 DuskGui_getAvailableSpace()
{
    DuskGuiParamsEntry* panel = DuskGui_getCurrentPanel();
    Rectangle rect = panel->params.bounds;
    return (Vector2) { rect.width, rect.height };
}