#include "../plane_sim_g.h"
#include <rlgl.h>

void SpriteRendererComponent_sequentialDraw(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
    void* componentData, void* userdata)
{
    SpriteRendererComponent* spriteRenderer = (SpriteRendererComponent*)componentData;
    Matrix m = SceneObject_getWorldMatrix(sceneObject);

    Vector3 r = (Vector3) { m.m0 * .5f, m.m1 * .5f, m.m2 * .5f };
    Vector3 u = (Vector3) { m.m4 * .5f, m.m5 * .5f, m.m6 * .5f };
    Vector3 p = { m.m12, m.m13, m.m14 };

    int width = spriteRenderer->texture.width;
    int height = spriteRenderer->texture.height;
    Rectangle source = spriteRenderer->source;
    rlSetTexture(spriteRenderer->texture.id);
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
    float left = spriteRenderer->scale9frame.x;
    float top = spriteRenderer->scale9frame.y;
    float right = spriteRenderer->scale9frame.z;
    float bottom = spriteRenderer->scale9frame.w;
    float w = spriteRenderer->size.x;
    float h = spriteRenderer->size.y;

    // if (spriteRenderer->scale9frame.x > 0.0f) {
    //     float colw = left / ppu;
    //     if (top > 0)
    //     {
    //         // top left
    //         float toph = top / ppu;
    //         rlTexCoord2f((source.x + left) / width, source.y / height);
    //         rlVertex3f(p.x + r.x * (w-colw) + u.x * h, p.y + r.y * (w-colw) + u.y * h, p.z + r.z * (w-colw) + u.z * h);
    //         rlTexCoord2f(source.x / width, source.y / height);
    //         rlVertex3f(p.x + r.x * w + u.x * h, p.y + r.y * w + u.y * h, p.z + r.z * w + u.z * h);
    //         rlTexCoord2f(source.x / width, (source.y + top) / height);
    //         rlVertex3f(p.x + r.x * w + u.x * (h-toph), p.y + r.y * w + u.y * (h-toph), p.z + r.z * w + u.z * (h-toph));
    //         rlTexCoord2f((source.x + left) / width, (source.y + top) / height);
    //         rlVertex3f(p.x + r.x * (w-colw) + u.x * (h-toph), p.y + r.y * (w-colw) + u.y * (h-toph), p.z + r.z * (w-colw) + u.z * (h-toph));
    //     }

    //     if (bottom > 0)
    //     {
    //         // bottom left
    //         float bottomh = bottom / ppu;
    //         rlTexCoord2f((source.x + left) / width, (source.y + source.height - bottom) / height);
    //         rlVertex3f(p.x + r.x * (w-colw) + u.x * bottomh, p.y + r.y * (w-colw) + u.y * bottomh, p.z + r.z * (w-colw) + u.z * bottomh);
    //         rlTexCoord2f((source.x + left) / width, (source.y + source.height) / height);
    //         rlVertex3f(p.x + r.x * (w-colw) + u.x * h, p.y + r.y * (w-colw) + u.y * h, p.z + r.z * (w-colw) + u.z * h);
    //         rlTexCoord2f(source.x / width, (source.y + source.height) / height);
    //         rlVertex3f(p.x + r.x * w + u.x * h, p.y + r.y * w + u.y * h, p.z + r.z * w + u.z * h);
    //         rlTexCoord2f(source.x / width, (source.y + source.height - bottom) / height);
    //         rlVertex3f(p.x + r.x * w + u.x * bottomh, p.y + r.y * w + u.y * bottomh, p.z + r.z * w + u.z * bottomh);
    //     }
    // }

    float iw = w - (left + right) / ppu * .5f;
    float ih = h - (top + bottom) / ppu * .5f;

    Vector3 v11 = (Vector3) { p.x + r.x * iw + u.x * ih, p.y + r.y * iw + u.y * ih, p.z + r.z * iw + u.z * ih };
    Vector3 t11 = (Vector3) { (source.x + left) / width, (source.y + bottom) / height };
    Vector3 v12 = (Vector3) { p.x + r.x * iw - u.x * ih, p.y + r.y * iw - u.y * ih, p.z + r.z * iw - u.z * ih };
    Vector2 t12 = (Vector2) { (source.x + left) / width, (source.y + source.height - top) / height };
    Vector3 v22 = (Vector3) { p.x - r.x * iw - u.x * ih, p.y - r.y * iw - u.y * ih, p.z - r.z * iw - u.z * ih };
    Vector2 t22 = (Vector2) { (source.x + source.width - right) / width, (source.y + source.height - top) / height };
    Vector3 v21 = (Vector3) { p.x - r.x * iw + u.x * ih, p.y - r.y * iw + u.y * ih, p.z - r.z * iw + u.z * ih };
    Vector2 t21 = (Vector2) { (source.x + source.width - right) / width, (source.y + bottom) / height };
    rlTexCoord2f(t22.x, t22.y);
    rlVertex3f(v22.x, v22.y, v22.z);

    rlTexCoord2f(t21.x, t21.y);
    rlVertex3f(v21.x, v21.y, v21.z);

    rlTexCoord2f(t11.x, t11.y);
    rlVertex3f(v11.x, v11.y, v11.z);

    rlTexCoord2f(t12.x, t12.y);
    rlVertex3f(v12.x, v12.y, v12.z);

    rlEnd();
    rlSetTexture(0);
}