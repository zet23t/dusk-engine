#include "../game_g.h"
#include <rlgl.h>
#include <string.h>

static void DrawTextCodepoint3D(Font font, int codepoint, Matrix m, float fontSize, bool backface, Color tint)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scale = fontSize / (float)font.baseSize;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    m.m12 += (float)(font.glyphs[index].offsetX - font.glyphPadding) / (float)font.baseSize * scale;
    m.m13 += (float)(font.glyphs[index].offsetY - font.glyphPadding) / (float)font.baseSize * scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
        font.recs[index].width + 2.0f * font.glyphPadding, font.recs[index].height + 2.0f * font.glyphPadding };

    float width = (float)(font.recs[index].width + 2.0f * font.glyphPadding) / (float)font.baseSize * scale;
    float height = (float)(font.recs[index].height + 2.0f * font.glyphPadding) / (float)font.baseSize * scale;

    if (font.texture.id > 0) {
        // normalized texture coordinates of the glyph inside the font texture (0.0f -> 1.0f)
        const float tx = srcRec.x / font.texture.width;
        const float ty = srcRec.y / font.texture.height;
        const float tw = (srcRec.x + srcRec.width) / font.texture.width;
        const float th = (srcRec.y + srcRec.height) / font.texture.height;

        // if (SHOW_LETTER_BOUNDRY) DrawCubeWiresV((Vector3){ position.x + width/2, position.y, position.z + height/2}, (Vector3){ width, LETTER_BOUNDRY_SIZE, height }, LETTER_BOUNDRY_COLOR);

        rlCheckRenderBatchLimit(4 + 4 * backface);
        rlSetTexture(font.texture.id);

        rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        Vector3 v;
        // Front Face
        rlNormal3f(0.0f, 1.0f, 0.0f); // Normal Pointing Up
        rlTexCoord2f(tx, ty);
        rlVertex3f(m.m12, m.m13, m.m14); // Top Left Of The Texture and Quad
        rlTexCoord2f(tx, th);
        v = Vector3Transform((Vector3){ 0, -height, 0}, m);
        rlVertex3f(v.x, v.y, v.z); // Bottom Left Of The Texture and Quad
        rlTexCoord2f(tw, th);
        v = Vector3Transform((Vector3){ -width, -height, 0}, m);
        rlVertex3f(v.x, v.y, v.z); // Bottom Right Of The Texture and Quad
        rlTexCoord2f(tw, ty);
        v = Vector3Transform((Vector3){ -width, 0, 0}, m);
        rlVertex3f(v.x, v.y, v.z); // Top Right Of The Texture and Quad

        // if (backface) {
        //     // Back Face
        //     rlNormal3f(0.0f, -1.0f, 0.0f); // Normal Pointing Down
        //     rlTexCoord2f(tx, ty);
        //     rlVertex3f(x, y, z); // Top Right Of The Texture and Quad
        //     rlTexCoord2f(tw, ty);
        //     rlVertex3f(x + width, y, z); // Top Left Of The Texture and Quad
        //     rlTexCoord2f(tw, th);
        //     rlVertex3f(x + width, y, z + height); // Bottom Left Of The Texture and Quad
        //     rlTexCoord2f(tx, th);
        //     rlVertex3f(x, y, z + height); // Bottom Right Of The Texture and Quad
        // }
        rlEnd();

        rlSetTexture(0);
    }
}

// Draw a 2D text in 3D space
static void DrawText3D(Font font, const char* text, Matrix m, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint)
{
    int length = TextLength(text); // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0.0f; // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f; // Offset X to next character to draw

    float scale = fontSize / (float)font.baseSize;

    for (int i = 0; i < length;) {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f)
            codepointByteCount = 1;

        if (codepoint == '\n') {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += scale + lineSpacing / (float)font.baseSize * scale;
            textOffsetX = 0.0f;
        } else {
            if ((codepoint != ' ') && (codepoint != '\t')) {
                Matrix mm = m;
                mm.m12 -= textOffsetX;
                mm.m13 -= textOffsetY;
                DrawTextCodepoint3D(font, codepoint, mm, fontSize, backface, tint);
            }

            if (font.glyphs[index].advanceX == 0)
                textOffsetX += (float)(font.recs[index].width + fontSpacing) / (float)font.baseSize * scale;
            else
                textOffsetX += (float)(font.glyphs[index].advanceX + fontSpacing) / (float)font.baseSize * scale;
        }

        i += codepointByteCount; // Move text bytes counter to next codepoint
    }
}

// Measure a text in 3D. For some reason `MeasureTextEx()` just doesn't seem to work so i had to use this instead.
static Vector2 MeasureText3D(Font font, const char* text, float fontSize, float fontSpacing, float lineSpacing)
{
    int len = TextLength(text);
    int tempLen = 0; // Used to count longer text line num chars
    int lenCounter = 0;

    float tempTextWidth = 0.0f; // Used to count longer text line width

    float scale = fontSize / (float)font.baseSize;
    float textHeight = scale;
    float textWidth = 0.0f;

    int letter = 0; // Current character
    int index = 0; // Index position in sprite font

    for (int i = 0; i < len; i++) {
        lenCounter++;

        int next = 0;
        letter = GetCodepoint(&text[i], &next);
        index = GetGlyphIndex(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f)
            next = 1;
        i += next - 1;

        if (letter != '\n') {
            if (font.glyphs[index].advanceX != 0)
                textWidth += (font.glyphs[index].advanceX + fontSpacing) / (float)font.baseSize * scale;
            else
                textWidth += (font.recs[index].width + font.glyphs[index].offsetX) / (float)font.baseSize * scale;
        } else {
            if (tempTextWidth < textWidth)
                tempTextWidth = textWidth;
            lenCounter = 0;
            textWidth = 0.0f;
            textHeight += scale + lineSpacing / (float)font.baseSize * scale;
        }

        if (tempLen < lenCounter)
            tempLen = lenCounter;
    }

    if (tempTextWidth < textWidth)
        tempTextWidth = textWidth;

    Vector2 vec = { 0 };
    vec.x = tempTextWidth + (float)((tempLen - 1) * fontSpacing / (float)font.baseSize * scale); // Adds chars spacing to measure
    vec.y = textHeight / (float)font.baseSize * scale;
    
    return vec;
}


static void TextComponent_initialize(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData, void* arg)
{
    TextComponent *src = (TextComponent*) arg;
    TextComponent *dst = (TextComponent*) componentData;
    memcpy(dst, src, sizeof(*src));
    dst->text = strdup(src->text);
}

void TextComponent_setText(SceneGraph* graph, SceneComponentId textComponentId, const char *str)
{
    TextComponent *textComponent;
    if (!SceneGraph_getComponent(graph, textComponentId, (void*) &textComponent))
    {
        return;
    }
    free(textComponent->text);
    textComponent->text = strdup(str);
}

static void TextComponent_onDestroy(SceneObject* sceneObject, SceneComponentId sceneComponentId, void* componentData)
{
    TextComponent *textComponent = (TextComponent*) componentData;
    free(textComponent->text);
}

static void TextComponentDraw(Camera3D camera, SceneObject* node, SceneComponentId sceneComponentId, void* component, void* userdata)
{
    TextComponent* textComponent = (TextComponent*)component;
    Matrix m = SceneObject_getWorldMatrix(node);
    Font font = textComponent->font;
    if (font.texture.id == 0)
    {
        font = GetFontDefault();
    }
    int textLen = strlen(textComponent->text) + 1;
    char text[textLen];
    strcpy(text, textComponent->text);
    char *t = text;
    float scale = textComponent->fontSize / (float)font.baseSize;
    float lineHeight = scale + textComponent->lineSpacing * scale;
    int lineCount = 1;
    for (int i=0;i<textLen;i+=1) lineCount += text[i] == '\n';
    m.m13 += (lineHeight) * lineCount * textComponent->align.y * 2.0f;
    // m.m12 += textComponent->offset.x;
    // m.m13 += textComponent->offset.y;
    while (1)
    {
        char *next = NULL;
        for (int i=0;t[i]; i++)
        {
            if (t[i] == '\n')
            {
                t[i] = 0;
                next = t + i + 1;
                break;
            }
        }
        Vector2 dims = MeasureText3D(font, t, textComponent->fontSize, textComponent->fontSpacing, textComponent->lineSpacing);
        Matrix mm = m;
        mm.m12 += dims.x * textComponent->align.x;
        mm.m12 += textComponent->offset.x;
        mm.m14 += textComponent->offset.y;
        DrawText3D(font, t, mm, textComponent->fontSize, textComponent->fontSpacing, textComponent->lineSpacing, false, textComponent->color);
        if (next)
        {
            m.m14 -= (textComponent->lineSpacing * textComponent->fontSize / (float) font.baseSize);// ;
            t = next;
        }
        else
            break;
    }
}

void TextComponentRegister()
{
    psg.textComponentId = SceneGraph_registerComponentType(psg.sceneGraph, "TextComponent", sizeof(TextComponent),
        (SceneComponentTypeMethods) {
            .sequentialDraw = TextComponentDraw,
            .onInitialize = TextComponent_initialize,
            .onDestroy = TextComponent_onDestroy,
        });
}