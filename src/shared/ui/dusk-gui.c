#include "shared/ui/dusk-gui.h"
#include "raymath.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static DuskGuiState _duskGuiState;
DuskGuiTextBuffer* DuskGui_getTextBuffer(DuskGuiParamsEntry* entry, int createIfNull);

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
    style.icon.color = ColorLerp(a->icon.color, b->icon.color, t);
    style.icon.dst.x = IntLerp(a->icon.dst.x, b->icon.dst.x, t);
    style.icon.dst.y = IntLerp(a->icon.dst.y, b->icon.dst.y, t);
    style.icon.dst.width = IntLerp(a->icon.dst.width, b->icon.dst.width, t);
    style.icon.dst.height = IntLerp(a->icon.dst.height, b->icon.dst.height, t);
    style.icon.pivot = Vector2Lerp(a->icon.pivot, b->icon.pivot, t);
    style.icon.alignment = Vector2Lerp(a->icon.alignment, b->icon.alignment, t);
    style.icon.rotationDegrees = Lerp(a->icon.rotationDegrees, b->icon.rotationDegrees, t);
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

const char* DuskGui_getText(DuskGuiParamsEntry* entry)
{
    DuskGuiTextBuffer* buffer = DuskGui_getTextBuffer(entry, 0);
    return buffer ? buffer->buffer : entry->params.text;
}

static void DuskGui_defaultDrawStyle(DuskGuiParamsEntry* entry, DuskGuiState* state, DuskGuiStyleGroup* styleGroup)
{
    if (entry->params.styleGroup != NULL)
    {
        styleGroup = entry->params.styleGroup;
    }
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

    if (lerpedStyle.icon.color.a > 0 && lerpedStyle.icon.texture.id != 0) {
        Vector2 iconSize = (Vector2) {lerpedStyle.icon.dst.width, lerpedStyle.icon.dst.height};
        Vector2 iconPivot = lerpedStyle.icon.pivot;
        Vector2 iconOffset = (Vector2) {lerpedStyle.icon.dst.x, lerpedStyle.icon.dst.y};
        Vector2 iconAlignment = lerpedStyle.icon.alignment;
        float iconRotation = lerpedStyle.icon.rotationDegrees;
        Vector2 iconPosition = (Vector2) {
            (entry->params.bounds.width - iconSize.x) * iconAlignment.x + entry->params.bounds.x + iconOffset.x,
            (entry->params.bounds.height - iconSize.y) * iconAlignment.y + entry->params.bounds.y + iconOffset.y
        };
        // printf("iconPosition: %f, %f\n", iconPosition.x, iconPosition.y);
        DrawTexturePro(lerpedStyle.icon.texture, (Rectangle) { 0, 0, lerpedStyle.icon.texture.width, lerpedStyle.icon.texture.height },
            (Rectangle) { iconPosition.x + iconPivot.x, iconPosition.y + iconPivot.y, iconSize.x, iconSize.y },
            iconPivot, iconRotation, lerpedStyle.icon.color);
    }

    DuskGuiIconSprite* icon = &entry->params.icon;
    if (icon->texture.id != 0 && icon->color.a > 0) {
        Vector2 iconSize = icon->dst.width != 0 && icon->dst.height != 0 ? (Vector2) { icon->dst.width, icon->dst.height } : (Vector2) { entry->params.bounds.width, entry->params.bounds.height };
        Vector2 iconOffset = (Vector2) { icon->dst.x + entry->params.bounds.x, icon->dst.y + entry->params.bounds.y };
        float iconRotation = icon->rotationDegrees;
        Rectangle dest = (Rectangle) { iconOffset.x + icon->pivot.x, iconOffset.y + icon->pivot.y, iconSize.x, iconSize.y };
        DrawTexturePro(icon->texture, icon->src,
            dest,
            icon->pivot, iconRotation, icon->color);
    }

    const char* origText = DuskGui_getText(entry);
    if (origText && lerpedStyle.textColor.a > 0) {
        // draw debug outline
        // DrawRectangleLines(entry->params.bounds.x, entry->params.bounds.y, entry->params.bounds.width, entry->params.bounds.height, RED);

        Font font = lerpedStyle.fontStyle ? lerpedStyle.fontStyle->font : _defaultFont.font;
        if (font.texture.id == 0) {
            font = GetFontDefault();
        }
        char text[strlen(origText) + 4];
        strcpy(text, origText);

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

void DuskGui_openMenu(const char* menuName)
{
    int n = DuskGui_isMenuOpen(menuName);
    if (n > 0) {
        _duskGuiState.activeMenus[n - 1].lastTriggerTime = GetTime();
        return;
    }

    // opening a submenu from a menu should close other submenus of the same level
    // this means, we need to figure out the currently active menu, find the index in the activeMenus array
    // and close all menus after that index.
    if (_duskGuiState.menuStackCount > 0) {
        DuskGuiParamsEntry* currentMenu = _duskGuiState.menuStack[_duskGuiState.menuStackCount - 1];
        for (int i = 0; i < DUSKGUI_MAX_ACTIVE_MENU_COUNT && _duskGuiState.activeMenus[i].menuName; i++) {
            if (strcmp(_duskGuiState.activeMenus[i].menuName, currentMenu->txId) == 0) {
                i++;
                while (i < DUSKGUI_MAX_ACTIVE_MENU_COUNT && _duskGuiState.activeMenus[i].menuName) {
                    free(_duskGuiState.activeMenus[i].menuName);
                    _duskGuiState.activeMenus[i].menuName = NULL;
                    i++;
                }
                break;
            }
        }
    }

    int openMenuCount = 0;
    while (openMenuCount < DUSKGUI_MAX_ACTIVE_MENU_COUNT && _duskGuiState.activeMenus[openMenuCount].menuName) {
        openMenuCount++;
    }
    _duskGuiState.activeMenus[openMenuCount].menuName = strdup(menuName);
    _duskGuiState.activeMenus[openMenuCount].firstActiveTime = GetTime();
    _duskGuiState.activeMenus[openMenuCount].lastTriggerTime = GetTime();
    _duskGuiState.activeMenus[openMenuCount + 1].menuName = NULL;
}

int DuskGui_isMenuOpen(const char* menuName)
{
    for (int i = 0; i < DUSKGUI_MAX_ACTIVE_MENU_COUNT && _duskGuiState.activeMenus[i].menuName; i++) {
        if (strcmp(_duskGuiState.activeMenus[i].menuName, menuName) == 0) {
            return 1 + i;
        }
    }
    return 0;
}

void DuskGui_closeAllMenus()
{
    for (int i = 0; i < DUSKGUI_MAX_ACTIVE_MENU_COUNT && _duskGuiState.activeMenus[i].menuName; i++) {
        free(_duskGuiState.activeMenus[i].menuName);
        _duskGuiState.activeMenus[i].menuName = NULL;
    }
}

// closes the first menu of given name and also all subsequent menus
int DuskGui_closeMenu(const char* menuName)
{
    for (int i = 0; i < DUSKGUI_MAX_ACTIVE_MENU_COUNT && _duskGuiState.activeMenus[i].menuName; i++) {
        if (strcmp(_duskGuiState.activeMenus[i].menuName, menuName) == 0) {
            while (i < DUSKGUI_MAX_ACTIVE_MENU_COUNT && _duskGuiState.activeMenus[i].menuName) {
                free(_duskGuiState.activeMenus[i].menuName);
                _duskGuiState.activeMenus[i].menuName = NULL;
                i++;
            }
            return 1;
        }
    }
    return 0;
}

void DuskGui_init()
{
    _defaultStyles.groups[DUSKGUI_STYLE_BUTTON].name = "DUSKGUI_STYLE_BUTTON";
    _defaultStyles.groups[DUSKGUI_STYLE_LABEL].name = "DUSKGUI_STYLE_LABEL";
    _defaultStyles.groups[DUSKGUI_STYLE_LABELBUTTON].name = "DUSKGUI_STYLE_LABELBUTTON";
    _defaultStyles.groups[DUSKGUI_STYLE_FOLDOUT_OPEN].name = "DUSKGUI_STYLE_FOLDOUT_OPEN";
    _defaultStyles.groups[DUSKGUI_STYLE_FOLDOUT_CLOSED].name = "DUSKGUI_STYLE_FOLDOUT_CLOSED";
    _defaultStyles.groups[DUSKGUI_STYLE_PANEL].name = "DUSKGUI_STYLE_PANEL";
    _defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_LINE].name = "DUSKGUI_STYLE_HORIZONTAL_LINE";
    _defaultStyles.groups[DUSKGUI_STYLE_INPUTTEXTFIELD].name = "DUSKGUI_STYLE_INPUTTEXTFIELD";
    _defaultStyles.groups[DUSKGUI_STYLE_INPUTNUMBER_FIELD].name = "DUSKGUI_STYLE_INPUTNUMBER_FIELD";
    _defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_SLIDER_BACKGROUND].name = "DUSKGUI_STYLE_HORIZONTAL_SLIDER_BACKGROUND";
    _defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_SLIDER_HANDLE].name = "DUSKGUI_STYLE_HORIZONTAL_SLIDER_HANDLE";
    _defaultStyles.groups[DUSKGUI_STYLE_MENU].name = "DUSKGUI_STYLE_MENU";
    _defaultStyles.groups[DUSKGUI_STYLE_MENU_ITEM].name = "DUSKGUI_STYLE_MENU_ITEM";

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

    Image menuTexture = GenImageColor(16, 16, (Color) { 255, 255, 255, 255 });
    Color* menuPixels = (Color*)menuTexture.data;
    for (int y = 0; y < 15; y++) {
        for (int x = 0; x < 15; x++) {
            if (x == 0 || y == 0 || x == 14 || y == 14)
                menuPixels[y * 16 + x] = (Color) { 0, 0, 0, 255 };
        }
    }
    for (int x = 0; x < 16; x++) {
        menuPixels[15 * 16 + x] = (Color) { 0, 0, 0, x == 0 ? 0 : 127 };
    }
    for (int y = 0; y < 16; y++) {
        menuPixels[y * 16 + 15] = (Color) { 0, 0, 0, y == 0 ? 0 : 127 };
    }
    menuPixels[0xff] = (Color) { 0, 0, 0, 64 };
    Texture2D menuBackground = LoadTextureFromImage(menuTexture);
    UnloadImage(menuTexture);

    DuskGuiStyleGroup* menuGroup = &_defaultStyles.groups[DUSKGUI_STYLE_MENU];
    menuGroup->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .paddingLeft = 1,
        .paddingRight = 2,
        .paddingTop = 1,
        .paddingBottom = 3,
        .backgroundColor = (Color) { 220, 220, 220, 255 },
        .textColor = BLACK,
        .backgroundTexture = menuBackground,
        .backgroundPatchInfo = GenNPatchInfo(0, 0, 16, 16, 3, 3, 3, 3),
    };
    menuGroup->fallbackStyle.textColor = BLANK;

    DuskGuiStyleGroup* menuItemGroup = &_defaultStyles.groups[DUSKGUI_STYLE_MENU_ITEM];
    menuItemGroup->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .paddingLeft = 4,
        .paddingRight = 4,
        .paddingTop = 2,
        .paddingBottom = 2,
        .backgroundColor = (Color) { 220, 220, 220, 255 },
        .textColor = BLACK,
    };
    menuItemGroup->normal = &menuItemGroup->fallbackStyle;
    menuItemGroup->hover = DuskGui_createGuiStyle(&menuItemGroup->fallbackStyle);
    menuItemGroup->hover->backgroundColor = (Color) { 200, 200, 250, 255 };
    menuItemGroup->hover->transitionLingerTime = 0.25f;
    menuItemGroup->pressed = DuskGui_createGuiStyle(&menuItemGroup->fallbackStyle);
    menuItemGroup->pressed->backgroundColor = (Color) { 200, 200, 200, 255 };
    menuItemGroup->pressed->textColor = RED;
    menuItemGroup->pressed->transitionActivationTime = -.15f;

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
        .icon.texture = foldoutIcon,
        .icon.color = BLACK,
        .icon.dst.x = -2.0f,
        .icon.dst.y = -1.0f,
        .icon.dst.width = 16.0f,
        .icon.dst.height = 16.0f,
        .icon.pivot = (Vector2) { 5, 8 },
        .icon.alignment = (Vector2) { 0, 0.5f },
        .icon.src = (Rectangle) { 0, 0, 16, 16 },
        .icon.rotationDegrees = 90,
        .transitionLingerTime = 0.033f,
    };
    foldoutOpen->normal = &foldoutOpen->fallbackStyle;
    foldoutOpen->hover = DuskGui_createGuiStyle(&foldoutOpen->fallbackStyle);
    foldoutOpen->hover->icon.rotationDegrees = 70.0f;
    foldoutOpen->hover->textColor = (Color) { 80, 80, 128, 255 };
    foldoutOpen->pressed = DuskGui_createGuiStyle(&foldoutOpen->fallbackStyle);
    foldoutOpen->pressed->textColor = (Color) { 100, 100, 255, 255 };
    foldoutOpen->pressed->icon.rotationDegrees = 45;

    DuskGuiStyleGroup* foldoutClosed = &_defaultStyles.groups[DUSKGUI_STYLE_FOLDOUT_CLOSED];
    foldoutClosed->fallbackStyle = foldoutOpen->fallbackStyle;
    foldoutClosed->fallbackStyle.icon.rotationDegrees = 0;
    foldoutClosed->normal = &foldoutClosed->fallbackStyle;
    foldoutClosed->hover = DuskGui_createGuiStyle(&foldoutClosed->fallbackStyle);
    foldoutClosed->hover->icon.rotationDegrees = 20.0f;
    foldoutClosed->hover->textColor = (Color) { 80, 80, 128, 255 };
    foldoutClosed->pressed = DuskGui_createGuiStyle(&foldoutClosed->fallbackStyle);
    foldoutClosed->pressed->textColor = (Color) { 100, 100, 255, 255 };
    foldoutClosed->pressed->icon.rotationDegrees = 45;

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
        .icon.texture = cursorTexture,
        .icon.color = (Color) { 0, 0, 0, 0 },
        .icon.dst = (Rectangle) { 0, 0, 2, 16 },
        .icon.src = (Rectangle) { 0, 0, 2, 16 },
    };
    textFieldGroup->normal = &textFieldGroup->fallbackStyle;
    textFieldGroup->hover = DuskGui_createGuiStyle(&textFieldGroup->fallbackStyle);
    textFieldGroup->hover->backgroundColor = (Color) { 240, 240, 240, 255 };
    textFieldGroup->hover->icon.color = BLACK;
    textFieldGroup->pressed = DuskGui_createGuiStyle(&textFieldGroup->fallbackStyle);
    textFieldGroup->pressed->backgroundColor = (Color) { 220, 220, 220, 255 };
    textFieldGroup->pressed->icon.color = BLACK;
    textFieldGroup->focused = DuskGui_createGuiStyle(&textFieldGroup->fallbackStyle);
    textFieldGroup->focused->backgroundColor = (Color) { 250, 200, 200, 255 };
    textFieldGroup->focused->icon.color = BLACK;

    Image numberHArrows = GenImageColor(16, 16, (Color) { 255, 255, 255, 0 });
    Color* numberHArrowsPixels = (Color*)numberHArrows.data;
    for (int y = 0; y < 16; y++) {
        int dy = (y - 8) * 2;
        for (int x = 0; x < 16; x++) {
            int dx = (x - 8) * 2 + 1;
            int absDist = abs(dx) + abs(dy);
            if (absDist < 11 && abs(dx) > 2) {
                numberHArrowsPixels[y * 16 + x] = (Color) { 255, 255, 255, 255 };
            }
        }
    }
    Texture2D numberHArrowsTexture = LoadTextureFromImage(numberHArrows);
    UnloadImage(numberHArrows);
    DuskGuiStyleGroup* numberInputField = &_defaultStyles.groups[DUSKGUI_STYLE_INPUTNUMBER_FIELD];
    numberInputField->fallbackStyle = textFieldGroup->fallbackStyle;
    numberInputField->fallbackStyle.paddingLeft = 16;
    numberInputField->fallbackStyle.paddingTop = 6;
    numberInputField->fallbackStyle.icon.texture = numberHArrowsTexture;
    numberInputField->fallbackStyle.icon.dst = (Rectangle) { 0, 0, 16, 16 };
    numberInputField->fallbackStyle.icon.pivot = (Vector2) { 8, 8 };
    numberInputField->fallbackStyle.icon.alignment = (Vector2) { 0, 0.5f };
    numberInputField->fallbackStyle.icon.color = BLACK;

    numberInputField->normal = &numberInputField->fallbackStyle;
    numberInputField->hover = DuskGui_createGuiStyle(&numberInputField->fallbackStyle);
    numberInputField->hover->backgroundColor = (Color) { 240, 240, 240, 255 };
    numberInputField->hover->icon.color = (Color) { 80, 80, 120, 255 };
    numberInputField->pressed = DuskGui_createGuiStyle(&numberInputField->fallbackStyle);
    numberInputField->pressed->backgroundColor = (Color) { 220, 220, 220, 255 };
    numberInputField->pressed->icon.color = (Color) { 200, 100, 100, 255 };
    numberInputField->focused = DuskGui_createGuiStyle(&numberInputField->fallbackStyle);
    numberInputField->focused->backgroundColor = (Color) { 250, 200, 200, 255 };
    numberInputField->focused->icon.color = (Color) { 200, 100, 100, 255 };

    DuskGuiStyleGroup* horizontalSliderBackgroundGroup = &_defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_SLIDER_BACKGROUND];
    horizontalSliderBackgroundGroup->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .paddingLeft = 1,
        .paddingRight = 1,
        .paddingTop = 1,
        .paddingBottom = 1,
        .backgroundColor = (Color) { 200, 200, 200, 255 },
        .textColor = BLACK,
        .transitionLingerTime = 0.033f,
        .backgroundTexture = defaultTexture,
        .backgroundPatchInfo = GenNPatchInfo(o, o, w, h, n, n, n, n),
    };
    horizontalSliderBackgroundGroup->normal = &horizontalSliderBackgroundGroup->fallbackStyle;
    horizontalSliderBackgroundGroup->hover = DuskGui_createGuiStyle(&horizontalSliderBackgroundGroup->fallbackStyle);
    horizontalSliderBackgroundGroup->hover->backgroundColor = (Color) { 220, 220, 220, 255 };
    horizontalSliderBackgroundGroup->pressed = DuskGui_createGuiStyle(&horizontalSliderBackgroundGroup->fallbackStyle);
    horizontalSliderBackgroundGroup->pressed->backgroundColor = (Color) { 200, 200, 250, 255 };
    horizontalSliderBackgroundGroup->focused = DuskGui_createGuiStyle(&horizontalSliderBackgroundGroup->fallbackStyle);
    horizontalSliderBackgroundGroup->focused->backgroundColor = (Color) { 250, 200, 200, 255 };

    DuskGuiStyleGroup* horizontalSliderHandleGroup = &_defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_SLIDER_HANDLE];
    horizontalSliderHandleGroup->fallbackStyle = (DuskGuiStyle) {
        .fontStyle = &_defaultFont,
        .paddingLeft = 4,
        .paddingRight = 4,
        .paddingTop = 4,
        .paddingBottom = 4,
        .backgroundColor = (Color) { 200, 200, 200, 255 },
        .textColor = BLACK,
        .textAlignment = (Vector2) { 0.5f, 0.5f },
        .transitionLingerTime = 0.033f,
        .backgroundTexture = defaultTexture,
        .backgroundPatchInfo = GenNPatchInfo(o, o, w, h, n, n, n, n),
    };
    horizontalSliderHandleGroup->normal = &horizontalSliderHandleGroup->fallbackStyle;
    horizontalSliderHandleGroup->hover = DuskGui_createGuiStyle(&horizontalSliderHandleGroup->fallbackStyle);
    horizontalSliderHandleGroup->hover->backgroundColor = (Color) { 220, 220, 220, 255 };
    horizontalSliderHandleGroup->pressed = DuskGui_createGuiStyle(&horizontalSliderHandleGroup->fallbackStyle);
    horizontalSliderHandleGroup->pressed->backgroundColor = (Color) { 200, 200, 250, 255 };
    horizontalSliderHandleGroup->focused = DuskGui_createGuiStyle(&horizontalSliderHandleGroup->fallbackStyle);
    horizontalSliderHandleGroup->focused->backgroundColor = (Color) { 250, 200, 200, 255 };

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

Rectangle DuskGui_fillHorizontally(int yOffset, int left, int right, int height)
{
    DuskGuiParamsEntry* parent = DuskGui_getParentById(_duskGuiState.currentPanelIndex, 1);
    Rectangle origBounds = parent->originalBounds;
    return (Rectangle) {
        left + parent->cachedStartStyle.paddingLeft,
        yOffset + parent->cachedStartStyle.paddingTop,
        origBounds.width - left - right - parent->cachedStartStyle.paddingLeft - parent->cachedStartStyle.paddingRight,
        height
    };
}

DuskGuiStyleGroup* DuskGui_getStyleGroup(int styleType)
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

int DuskGui_tryLock(DuskGuiParamsEntry* params, int x, int y, int relX, int relY)
{
    if (_duskGuiState.locked.txId != NULL) {
        return strcmp(_duskGuiState.locked.txId, params->txId) == 0;
    }

    if (_duskGuiState.locked.txId) {
        free(_duskGuiState.locked.txId);
    }
    _duskGuiState.locked = *params;
    _duskGuiState.locked.txId = strdup(params->txId);
    _duskGuiState.lockScreenPos = (Vector2) { x, y };
    _duskGuiState.lockRelativePos = (Vector2) { relX, relY };
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

void Rectangle_getMinMax(Rectangle* r, int* minX, int* minY, int* maxX, int* maxY)
{
    *minX = r->x;
    *minY = r->y;
    *maxX = r->x + r->width;
    *maxY = r->y + r->height;
}

void DuskGui_finalize()
{
    // auto close behavior of menus:
    // Menus have a last trigger time
    // when a parent menu has a higher last trigger time (+ threshold) than a child menu, the child menu is closed
    float t = GetTime();
    float autoCloseTime = t - 1.5f;
    int openMenuCount = 0;
    while (openMenuCount < DUSKGUI_MAX_ACTIVE_MENU_COUNT && _duskGuiState.activeMenus[openMenuCount].menuName) {
        openMenuCount++;
    }
    if (openMenuCount > 1) {
        DuskGuiActiveMenuStats* am = _duskGuiState.activeMenus;
        // printf("openMenuCount: %i; %f.1 / %f.1 %f.1\n", openMenuCount, am[openMenuCount - 1].lastTriggerTime, am[openMenuCount - 2].lastTriggerTime, autoCloseTime);
        // printf("  %i %i\n", am[openMenuCount - 1].lastTriggerTime < autoCloseTime, am[openMenuCount - 2].lastTriggerTime > am[openMenuCount - 1].lastTriggerTime);
        if (am[openMenuCount - 1].lastTriggerTime < autoCloseTime && am[openMenuCount - 2].lastTriggerTime > am[openMenuCount - 1].lastTriggerTime) {
            DuskGui_closeMenu(am[openMenuCount - 1].menuName);
        }
    }

    // draw all deferred entries (the ones with drawFn set)
    for (int i = 0; i < _duskGuiState.currentParams.count; i++) {
        DuskGuiParamsEntry* entry = &_duskGuiState.currentParams.params[i];
        if (entry->drawFn) {
            entry->drawFn(entry, &_duskGuiState, entry->drawStyleGroup);
            // it's a dupped string, so we can free it
            free((char*)entry->params.text);
        }
    }

    _duskGuiState.idCounter = 0;
    for (int i = _duskGuiState.currentParams.count - 1; i >= 0; i--) {
        _duskGuiState.currentParams.params[i].isMouseOver = 0;
    }

    // reversed raycast hit test
    int blocked = 0;
    const int maxBlockingParents = 64;
    DuskGuiParamsEntry* blockingParents[maxBlockingParents];
    int blockingParentsCount = 0;
    for (int pass = 0; pass < 2; pass++) {
        for (int i = _duskGuiState.currentParams.count - 1; i >= 0; i--) {
            DuskGuiParamsEntry* entry = &_duskGuiState.currentParams.params[i];
            if (entry->params.rayCastTarget == 0 || ((entry->drawFn == NULL) ^ (pass == 1)))
                continue;
            
            int isHit = CheckCollisionPointRec(GetMousePosition(), entry->params.bounds);

            if (isHit)
            {
                DuskGuiParamsEntry* parent = DuskGui_getParent(entry, 0);
                while (parent != NULL && isHit) {
                    if (parent->params.rayCastTarget) {
                        isHit = CheckCollisionPointRec(GetMousePosition(), parent->params.bounds);
                    }
                    parent = DuskGui_getParent(parent, 0);
                }
            }
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
                        TraceLog(LOG_ERROR, "Error, parent chain too long\n");
                        break;
                    }
                    blockingParents[blockingParentsCount++] = parent;
                    parent = DuskGui_getParent(parent, 0);
                }
                blocked = 1;
            }

            if (isHit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                int relX = GetMouseX() - entry->params.bounds.x;
                int relY = GetMouseY() - entry->params.bounds.y;
                DuskGui_tryLock(entry, GetMouseX(), GetMouseY(), relX, relY);
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
        if (_duskGuiState.currentParams.params[i].textBuffer) {
            _duskGuiState.currentParams.params[i].textBuffer->refCount--;
            if (_duskGuiState.currentParams.params[i].textBuffer->refCount <= 0) {
                free(_duskGuiState.currentParams.params[i].textBuffer->buffer);
                free(_duskGuiState.currentParams.params[i].textBuffer);
            }
        }
    }
    _duskGuiState.currentParams.count = 0;
    _duskGuiState.lastEntry = NULL;
    _duskGuiState.menuStackCount = 0;
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
    if (params.styleGroup != NULL) {
        initStyleGroup = params.styleGroup;
    }
    DuskGuiParamsEntry* parent = DuskGui_getCurrentPanel();
    Rectangle origBounds = params.bounds;
    params.bounds.x += parent->params.bounds.x;
    params.bounds.y += parent->params.bounds.y;

    params.bounds.x += parent->contentOffset.x;
    params.bounds.y += parent->contentOffset.y;
    DuskGuiParamsEntry entry = {
        .id = _duskGuiState.idCounter++,
        .originalBounds = origBounds,
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
    _duskGuiState.lastEntry = added;
    return added;
}

DuskGuiParamsEntry* DuskGui_getLastEntry()
{
    return _duskGuiState.lastEntry;
}

void DuskGui_update(DuskGuiParamsEntry* entry, DuskGuiStyleGroup* initStyleGroup)
{
    DuskGuiParamsEntry* match = DuskGui_findParams(&_duskGuiState.prevParams, entry);
    entry->isMouseOver = 0;
    if (match) {
        entry->flags = match->flags;

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

        if (entry->isFocused) {
            entry->textBuffer = match->textBuffer;
            if (entry->textBuffer != NULL) {
                entry->textBuffer->refCount++;
            }
        } else {
            entry->textBuffer = NULL;
        }
    } else if (initStyleGroup != NULL || entry->params.styleGroup != NULL) {
        DuskGuiStyleGroup* styleGroup = entry->params.styleGroup != NULL ? entry->params.styleGroup : initStyleGroup;
        entry->cachedStartStyle = styleGroup->fallbackStyle;
        entry->origStartStyle = styleGroup->normal;
        entry->nextStyle = styleGroup->normal;
        entry->transitionTime = 0;
        entry->transitionDuration = 1.0f;
        for (int i = 0; entry->params.text && entry->params.text[i]; i++) {
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
    // DrawRectangle(entry->params.bounds.x, entry->params.bounds.y, entry->params.bounds.width, entry->params.bounds.height, (Color) { 0, 0, 0, 255 });
    return DuskGui_hasLock(entry);
}

static void DuskGui_drawStyle(DuskGuiParamsEntry* params, DuskGuiState* state, DuskGuiStyleGroup* styleGroup)
{
    if (state->menuStackCount > 0) {
        // we have a menu open, draw the content in the deferred
        if (params->drawStyleGroup == NULL)
            params->drawStyleGroup = styleGroup;
        params->params.text = strdup(params->params.text);
        params->drawFn = DuskGui_drawStyle;
    } else if (params->params.styleGroup && params->params.styleGroup->draw) {
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

int DuskGui_floatInputField(DuskGuiParams params, float* value, float min, float max)
{
    DuskGuiStyleGroup* styleGroup = &_defaultStyles.groups[DUSKGUI_STYLE_INPUTNUMBER_FIELD];

    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, styleGroup);
    DuskGui_drawStyle(entry, &_duskGuiState, styleGroup);
    int modified = 0;
    if (DuskGui_hasLock(entry)) {
        float delta = GetMouseDelta().x;
        SetMousePosition(_duskGuiState.lockScreenPos.x, _duskGuiState.lockScreenPos.y);
        if (IsKeyDown(KEY_LEFT_CONTROL)) {
            delta *= 0.1f;
        }
        *value += delta;
        *value = *value < min ? min : (*value > max ? max : *value);
        modified = 1;
    }
    return modified;
}

int DuskGui_pushMenuEntry(DuskGuiParamsEntry* entry)
{
    if (_duskGuiState.menuStackCount >= DUSKGUI_MAX_MENU_DEPTH) {
        TraceLog(LOG_ERROR, "Menu stack overflow\n");
        return 0;
    }
    _duskGuiState.menuStack[_duskGuiState.menuStackCount++] = entry;
    return 1;
}

DuskGuiParamsEntry* DuskGui_popMenuEntry()
{
    if (_duskGuiState.menuStackCount <= 0) {
        TraceLog(LOG_ERROR, "Menu stack underflow\n");
        return NULL;
    }
    return _duskGuiState.menuStack[--_duskGuiState.menuStackCount];
}

static void DuskGui_drawStyleNop(DuskGuiParamsEntry* params, DuskGuiState* state, DuskGuiStyleGroup* styleGroup)
{
}
int DuskGui_fullScreenBlocker()
{
    char id[64];
    sprintf(id, "blocker_%i", _duskGuiState.idCounter);
    DuskGuiParamsEntry* entry = DuskGui_makeEntry((DuskGuiParams) { .text = strdup(id) }, NULL);
    entry->params.bounds = (Rectangle) { 0, 0, GetScreenWidth(), GetScreenHeight() };
    entry->params.rayCastTarget = 1;
    entry->drawFn = DuskGui_drawStyleNop;
    return entry->isTriggered;
}

int DuskGui_menuItem(int opensSubmenu, DuskGuiParams params)
{
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, &_defaultStyles.groups[DUSKGUI_STYLE_MENU_ITEM]);
    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_MENU_ITEM]);
    return opensSubmenu ? entry->isHovered : entry->isTriggered;
}

DuskGuiParamsEntry* DuskGui_beginMenu(DuskGuiParams params)
{
    int isOpen = DuskGui_isMenuOpen(params.text) != 0;
    params.rayCastTarget = isOpen;
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, &_defaultStyles.groups[DUSKGUI_STYLE_MENU]);

    entry->isMenu = 1;
    entry->isOpen = isOpen;
    if (isOpen) {
        if (_duskGuiState.menuStackCount == 0 && DuskGui_fullScreenBlocker()) {
            isOpen = 0;
            DuskGui_closeAllMenus();
        } else {
            _duskGuiState.currentPanelIndex = DuskGui_toParentIndex(entry);
            DuskGui_pushMenuEntry(entry);
            DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_MENU]);
        }

        if (entry->isMouseOver) {
            // update last trigger time of menu
            // printf("updating last trigger time %s\n", params.text);
            DuskGui_openMenu(params.text);
        }
    }

    return isOpen ? entry : NULL;
}

void DuskGui_endMenu()
{
    DuskGuiParamsEntry* entry = DuskGui_popMenuEntry();
    if (entry != NULL)
        _duskGuiState.currentPanelIndex = entry->parentIndex;
}

int DuskGui_horizontalFloatSlider(DuskGuiParams params, float* value, float min, float max)
{
    DuskGuiStyleGroup* handleStyle = &_defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_SLIDER_HANDLE];
    DuskGuiStyleGroup* backgroundStyle = &_defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_SLIDER_BACKGROUND];
    DuskGuiParams sliderBackgroundParams = params;
    const char* text = sliderBackgroundParams.text;
    while (text && text[0] != '\0' && (text[0] != '#' || text[1] != '#')) {
        text++;
    }
    sliderBackgroundParams.text = text;
    DuskGuiParamsEntry* entryBackground = DuskGui_makeEntry(sliderBackgroundParams, backgroundStyle);
    DuskGui_drawStyle(entryBackground, &_duskGuiState, backgroundStyle);
    DuskGuiStyle* bgStyle = &backgroundStyle->fallbackStyle;
    float invValue = (*value - min) / (max - min);
    float handleWidth = handleStyle->fallbackStyle.paddingLeft + handleStyle->fallbackStyle.paddingRight;
    float handleHeight = params.bounds.height - bgStyle->paddingTop - bgStyle->paddingBottom;
    char handleId[64];
    snprintf(handleId, 64, "%s_handle", text);
    float handleX = params.bounds.x + bgStyle->paddingLeft + invValue * (params.bounds.width - bgStyle->paddingLeft - bgStyle->paddingRight - handleWidth);
    float handleY = params.bounds.y + bgStyle->paddingTop;
    Rectangle handleBounds = (Rectangle) { handleX, handleY, handleWidth, handleHeight };

    DuskGuiParams sliderHandleParams = {
        .bounds = handleBounds,
        .text = handleId,
        .rayCastTarget = 1,
        .isFocusable = 0,
    };
    DuskGuiParamsEntry* entryHandle = DuskGui_makeEntry(sliderHandleParams, handleStyle);

    if (DuskGui_hasLock(entryHandle)) {
        float x = GetMouseX() - _duskGuiState.lockRelativePos.x - entryBackground->params.bounds.x - bgStyle->paddingLeft;
        float t = x / (params.bounds.width - bgStyle->paddingLeft - bgStyle->paddingRight - handleWidth);
        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        *value = min + t * (max - min);
        float newX = params.bounds.x + bgStyle->paddingLeft + t * (params.bounds.width - bgStyle->paddingLeft - bgStyle->paddingRight - handleWidth);
        entryHandle->params.bounds.x += newX - handleX;
    }

    DuskGui_drawStyle(entryHandle, &_duskGuiState, handleStyle);

    return entryBackground->isTriggered;
}

static int GetCharacterIndexByPixel(float x, float y, Font font, const char* text, float fontSize, float spacing)
{
    if ((font.texture.id == 0) || (text == NULL) || (x <= 0 && y <= 0))
        return 0;

    int size = TextLength(text); // Get size in bytes of text
    int byteCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f; // Used to count longer text line width

    float textHeight = fontSize;
    float scaleFactor = fontSize / (float)font.baseSize;

    int letter = 0; // Current character
    int index = 0; // Index position in sprite font

    float textLineSpacing = GetTextLineSpacing();

    x /= scaleFactor;

    for (int i = 0; i < size;) {
        byteCounter++;

        int next = 0;
        letter = GetCodepointNext(&text[i], &next);
        index = GetGlyphIndex(font, letter);

        i += next;

        float letterWidth = 0.0f;
        if (letter != '\n') {
            if (font.glyphs[index].advanceX != 0)
                letterWidth = font.glyphs[index].advanceX;
            else
                letterWidth = (font.recs[index].width + font.glyphs[index].offsetX);
            textWidth += letterWidth + spacing;
        } else {
            if (tempTextWidth < textWidth)
                tempTextWidth = textWidth;
            byteCounter = 0;
            textWidth = 0;

            if (textHeight >= y) {
                // point is below the baseline and we didn't cross x, means: cursor is to the right of the line
                return i;
            }

            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            textHeight += (float)textLineSpacing;
        }

        if (textWidth + letterWidth * .5f >= x && textHeight >= y)
            return i;
    }

    return size;
}

DuskGuiTextBuffer* DuskGui_getTextBuffer(DuskGuiParamsEntry* entry, int createIfNull)
{
    if (entry->textBuffer == NULL && createIfNull) {
        entry->textBuffer = (DuskGuiTextBuffer*)malloc(sizeof(DuskGuiTextBuffer));
        entry->textBuffer->buffer = strdup(entry->params.text);
        entry->textBuffer->capacity = strlen(entry->params.text);
        for (int i = 0; entry->textBuffer->buffer[i]; i++) {
            if (entry->textBuffer->buffer[i] == '#' && entry->textBuffer->buffer[i + 1] == '#') {
                entry->textBuffer->buffer[i] = '\0';
                break;
            }
        }
        entry->textBuffer->refCount = 1;
    }
    return entry->textBuffer;
}

void DuskGuiTextBuffer_insert(DuskGuiTextBuffer* buffer, int index, const char* tx)
{
    int txLen = TextLength(tx);
    int len = TextLength(buffer->buffer);
    if (index < 0) {
        index = 0;
    }
    if (index > len) {
        index = len;
    }
    if (txLen + len >= buffer->capacity) {
        buffer->capacity = (txLen + len) * 2;
        buffer->buffer = (char*)realloc(buffer->buffer, buffer->capacity + 1);
    }
    for (int i = len; i >= index; i--) {
        buffer->buffer[i + txLen] = buffer->buffer[i];
    }
    for (int i = 0; i < txLen; i++) {
        buffer->buffer[index + i] = tx[i];
    }
}

int DuskGuiTextBuffer_delete(DuskGuiTextBuffer* buffer, int index)
{
    int len = TextLength(buffer->buffer);
    if (index >= len) {
        return 0;
    }
    if (index < 0) {
        return 0;
    }
    int codepointLen = 0;
    GetCodepointNext(&buffer->buffer[index], &codepointLen);
    for (int i = index; i <= len - codepointLen; i++) {
        buffer->buffer[i] = buffer->buffer[i + codepointLen];
    }
    return codepointLen;
}

static float _keyCheckTimes[512];
int _IsKeyPressedRepeated(int k)
{
    if (k < 0)
        return 0;
    if (k > 512)
        return 0;
    if (IsKeyDown(k)) {
        float t = GetTime();
        float prev = _keyCheckTimes[k];
        if (t - prev < 0.1f) {
            return 0;
        }
        _keyCheckTimes[k] = prev < 0.0f ? t + 0.2f : t;
        return 1;
    } else {
        _keyCheckTimes[k] = -1.0f;
    }
    return 0;
}

int DuskGui_textInputField(DuskGuiParams params, char** buffer)
{
    DuskGuiStyleGroup* group = &_defaultStyles.groups[DUSKGUI_STYLE_INPUTTEXTFIELD];
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, group);
    DuskGui_drawStyle(entry, &_duskGuiState, group);
    const char* text = DuskGui_getText(entry);
    if (entry->isFocused && text && text[0] != '\0') {
        int len = strlen(text);
        char copy[len + 1];
        strcpy(copy, text);
        for (int i = 0; copy[i]; i++) {
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
            if (CheckCollisionPointRec(mousePos, textBounds)) {
                DuskGuiFontStyle* fontStyle = group->fallbackStyle.fontStyle;
                int x = mousePos.x - textBounds.x;
                int y = mousePos.y - textBounds.y;
                cursorIndex = GetCharacterIndexByPixel(x, y, fontStyle->font, copy, fontStyle->fontSize, fontStyle->fontSpacing);
            } else if (mousePos.x < textBounds.x) {
                cursorIndex = 0;
            } else if (mousePos.x > textBounds.x + textBounds.width) {
                cursorIndex = len;
            }
        }
        entry->cursorIndex = cursorIndex;
        copy[cursorIndex] = '\0';
        DuskGuiStyle* style = DuskGuiStyleGroup_selectStyle(entry, group);
        Vector2 cursorPos = MeasureTextEx(style->fontStyle->font, copy, style->fontStyle->fontSize, style->fontStyle->fontSpacing);
        Rectangle dst = (Rectangle) { textBounds.x + cursorPos.x + style->icon.dst.x, textBounds.y + style->icon.dst.y, style->icon.dst.width, style->icon.dst.height };
        DrawTexturePro(style->icon.texture, (Rectangle) { 0, 0, style->icon.texture.width, style->icon.texture.height },
            dst,
            (Vector2) { 0, 0 },
            0, style->icon.color);
    }
    if (entry->isFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            // NOTE: Only allow keys in range [32..125]
            if ((key >= 32) && (key <= 125)) {
                DuskGuiTextBuffer* textBuffer = DuskGui_getTextBuffer(entry, 1);
                int codepointLen = 0;
                const char* codepoint = CodepointToUTF8(key, &codepointLen);
                DuskGuiTextBuffer_insert(textBuffer, entry->cursorIndex, codepoint);
                entry->cursorIndex += codepointLen;
            } else {
                printf("K: %d\n", key);
            }

            key = GetCharPressed(); // Check next character in the queue
        }

        if (_IsKeyPressedRepeated(KEY_BACKSPACE)) {
            entry->cursorIndex -= DuskGuiTextBuffer_delete(DuskGui_getTextBuffer(entry, 1), entry->cursorIndex - 1);
        }
        if (_IsKeyPressedRepeated(KEY_DELETE)) {
            DuskGuiTextBuffer_delete(DuskGui_getTextBuffer(entry, 1), entry->cursorIndex);
        }
        if (_IsKeyPressedRepeated(KEY_LEFT) && entry->cursorIndex > 0) {
            int codepointSize;
            GetCodepointPrevious(&DuskGui_getTextBuffer(entry, 1)->buffer[entry->cursorIndex], &codepointSize);
            entry->cursorIndex -= codepointSize;
        }
        if (_IsKeyPressedRepeated(KEY_RIGHT) && entry->cursorIndex < TextLength(DuskGui_getText(entry))) {
            int codepointSize;
            GetCodepointNext(&DuskGui_getTextBuffer(entry, 1)->buffer[entry->cursorIndex], &codepointSize);
            entry->cursorIndex += codepointSize;
        }
        if (_IsKeyPressedRepeated(KEY_HOME)) {
            entry->cursorIndex = 0;
        }
        if (_IsKeyPressedRepeated(KEY_END)) {
            entry->cursorIndex = TextLength(DuskGui_getText(entry));
        }
        if (_IsKeyPressedRepeated(KEY_ENTER)) {
            entry->isTriggered = 1;
            if (buffer) {
                *buffer = strdup(DuskGui_getText(entry));
            }
        }
    }
    return entry->isTriggered;
}

void DuskGui_horizontalLine(DuskGuiParams params)
{
    DuskGuiParamsEntry* entry = DuskGui_makeEntry(params, &_defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_LINE]);
    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_HORIZONTAL_LINE]);
}

DuskGuiParamsEntry* DuskGui_icon(const char *id, Rectangle dst, Texture2D icon, Rectangle src, int raycastTarget)
{
    DuskGuiParamsEntry* entry = DuskGui_makeEntry((DuskGuiParams) { 
        .text = id, 
        .bounds = dst, .rayCastTarget = raycastTarget,
        .icon = (DuskGuiIconSprite) {
            .texture = icon,
            .src = src,
            .dst = {0,0,dst.width,dst.height},
            .color = WHITE
        }
    }, &_defaultStyles.groups[DUSKGUI_STYLE_ICON]);
    
    DuskGui_drawStyle(entry, &_duskGuiState, &_defaultStyles.groups[DUSKGUI_STYLE_ICON]);
    return entry;
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