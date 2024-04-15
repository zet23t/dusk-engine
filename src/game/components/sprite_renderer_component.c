#include "../game_g.h"
#include <rlgl.h>

static void SpriteRendererComponent_sequentialDraw(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
    void* componentData, void* userdata)
{
    SpriteRendererComponent* spriteRenderer = (SpriteRendererComponent*)componentData;
    Matrix m = SceneObject_getWorldMatrix(sceneObject);

    Vector3 r = (Vector3) { m.m0 * .5f, m.m1 * .5f, m.m2 * .5f };
    Vector3 u = (Vector3) { m.m4 * .5f, m.m5 * .5f, m.m6 * .5f };
    Vector3 p = { m.m12, m.m13, m.m14 };

    int width = spriteRenderer->spriteAsset.texture.width;
    int height = spriteRenderer->spriteAsset.texture.height;
    Rectangle source = spriteRenderer->spriteAsset.source;
    rlSetTexture(spriteRenderer->spriteAsset.texture.id);
    rlBegin(RL_QUADS);
    Color tint = spriteRenderer->tint;
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(m.m9, m.m10, m.m11);

    // draw scale 9
    // grid positions
    // 00 10 20 30
    // 01 11 21 31
    // 02 12 22 32
    // 03 13 23 33

    float ppu = spriteRenderer->pixelsPerUnit;
    float left = spriteRenderer->spriteAsset.scale9frame.x;
    float top = spriteRenderer->spriteAsset.scale9frame.y;
    float right = spriteRenderer->spriteAsset.scale9frame.z;
    float bottom = spriteRenderer->spriteAsset.scale9frame.w;
    float w = spriteRenderer->size.x;
    float h = spriteRenderer->size.y;

    // inner width and height
    float iw = w - (left + right) / ppu * .5f;
    float ih = h - (top + bottom) / ppu * .5f;

    Vector3 v00 = (Vector3) { p.x + r.x * w + u.x * h, p.y + r.y * w + u.y * h, p.z + r.z * w + u.z * h };
    Vector3 t00 = (Vector3) { (source.x) / width, (source.y) / height };
    Vector3 v10 = (Vector3) { p.x + r.x * iw + u.x * h, p.y + r.y * iw + u.y * h, p.z + r.z * iw + u.z * h };
    Vector3 t10 = (Vector3) { (source.x + left) / width, (source.y) / height };
    Vector3 v20 = (Vector3) { p.x - r.x * iw + u.x * h, p.y - r.y * iw + u.y * h, p.z - r.z * iw + u.z * h };
    Vector2 t20 = (Vector2) { (source.x + source.width - right) / width, (source.y) / height };
    Vector3 v30 = (Vector3) { p.x - r.x * w + u.x * h, p.y - r.y * w + u.y * h, p.z - r.z * w + u.z * h };
    Vector2 t30 = (Vector2) { (source.x + source.width) / width, (source.y) / height };

    Vector3 v01 = (Vector3) { p.x + r.x * w + u.x * ih, p.y + r.y * w + u.y * ih, p.z + r.z * w + u.z * ih };
    Vector3 t01 = (Vector3) { (source.x) / width, (source.y + bottom) / height };
    Vector3 v11 = (Vector3) { p.x + r.x * iw + u.x * ih, p.y + r.y * iw + u.y * ih, p.z + r.z * iw + u.z * ih };
    Vector3 t11 = (Vector3) { (source.x + left) / width, (source.y + bottom) / height };
    Vector3 v21 = (Vector3) { p.x - r.x * iw + u.x * ih, p.y - r.y * iw + u.y * ih, p.z - r.z * iw + u.z * ih };
    Vector2 t21 = (Vector2) { (source.x + source.width - right) / width, (source.y + bottom) / height };
    Vector3 v31 = (Vector3) { p.x - r.x * w + u.x * ih, p.y - r.y * w + u.y * ih, p.z - r.z * w + u.z * ih };
    Vector2 t31 = (Vector2) { (source.x + source.width) / width, (source.y + bottom) / height };

    Vector3 v02 = (Vector3) { p.x + r.x * w - u.x * ih, p.y + r.y * w - u.y * ih, p.z + r.z * w - u.z * ih };
    Vector2 t02 = (Vector2) { (source.x) / width, (source.y + source.height - top) / height };
    Vector3 v12 = (Vector3) { p.x + r.x * iw - u.x * ih, p.y + r.y * iw - u.y * ih, p.z + r.z * iw - u.z * ih };
    Vector2 t12 = (Vector2) { (source.x + left) / width, (source.y + source.height - top) / height };
    Vector3 v22 = (Vector3) { p.x - r.x * iw - u.x * ih, p.y - r.y * iw - u.y * ih, p.z - r.z * iw - u.z * ih };
    Vector2 t22 = (Vector2) { (source.x + source.width - right) / width, (source.y + source.height - top) / height };
    Vector3 v32 = (Vector3) { p.x - r.x * w - u.x * ih, p.y - r.y * w - u.y * ih, p.z - r.z * w - u.z * ih };
    Vector2 t32 = (Vector2) { (source.x + source.width) / width, (source.y + source.height - top) / height };
    
    Vector3 v03 = (Vector3) { p.x + r.x * w - u.x * h, p.y + r.y * w - u.y * h, p.z + r.z * w - u.z * h };
    Vector2 t03 = (Vector2) { (source.x) / width, (source.y + source.height) / height };
    Vector3 v13 = (Vector3) { p.x + r.x * iw - u.x * h, p.y + r.y * iw - u.y * h, p.z + r.z * iw - u.z * h };
    Vector2 t13 = (Vector2) { (source.x + left) / width, (source.y + source.height) / height };
    Vector3 v23 = (Vector3) { p.x - r.x * iw - u.x * h, p.y - r.y * iw - u.y * h, p.z - r.z * iw - u.z * h };
    Vector2 t23 = (Vector2) { (source.x + source.width - right) / width, (source.y + source.height) / height };
    Vector3 v33 = (Vector3) { p.x - r.x * w - u.x * h, p.y - r.y * w - u.y * h, p.z - r.z * w - u.z * h };
    Vector2 t33 = (Vector2) { (source.x + source.width) / width, (source.y + source.height) / height };
    

#define P(XY) rlTexCoord2f(t##XY.x, t##XY.y),rlVertex3f(v##XY.x, v##XY.y, v##XY.z);
#define Q(A,B,C,D) P(A) P(B) P(C) P(D)
    // 00 10 20 30
    // 01 11 21 31
    // 02 12 22 32
    // 03 13 23 33
    Q(11, 10, 00, 01)
    Q(21, 20, 10, 11)
    Q(31, 30, 20, 21)

    Q(12, 11, 01, 02)
    Q(22, 21, 11, 12)
    Q(32, 31, 21, 22)

    Q(13, 12, 02, 03)
    Q(23, 22, 12, 13)
    Q(33, 32, 22, 23)
#undef P

    rlEnd();
    rlSetTexture(0);
}