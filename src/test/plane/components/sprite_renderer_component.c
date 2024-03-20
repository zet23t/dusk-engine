#include "../plane_sim_g.h"
#include <rlgl.h>

void SpriteRendererComponent_sequentialDraw(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
    void* componentData, void* userdata)
{
    SpriteRendererComponent* spriteRenderer = (SpriteRendererComponent*)componentData;
    Matrix m = SceneObject_getWorldMatrix(sceneObject);
    
    Vector3 r = (Vector3){m.m0 * .5f, m.m1 * .5f, m.m2 * .5f};
    Vector3 u = (Vector3){m.m4 * .5f, m.m5 * .5f, m.m6 * .5f};
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
    float ppu = spriteRenderer->pixelsPerUnit;
    float left = spriteRenderer->scale9frame.x;
    float top = spriteRenderer->scale9frame.y;
    float right = spriteRenderer->scale9frame.z;
    float bottom = spriteRenderer->scale9frame.w;
    float w = spriteRenderer->size.x;
    float h = spriteRenderer->size.y;
    if (spriteRenderer->scale9frame.x > 0.0f) {
        float colw = left / ppu;
        float toph = top / ppu;
        rlTexCoord2f((source.x + left) / width, source.y / height);
        rlVertex3f(p.x + r.x * (w-colw) + u.x * h, p.y + r.y * (w-colw) + u.y * h, p.z + r.z * (w-colw) + u.z * h);
        rlTexCoord2f(source.x / width, source.y / height);
        rlVertex3f(p.x + r.x * w + u.x * h, p.y + r.y * w + u.y * h, p.z + r.z * w + u.z * h);
        rlTexCoord2f(source.x / width, (source.y + top) / height);
        rlVertex3f(p.x + r.x * w + u.x * (h-toph), p.y + r.y * w + u.y * (h-toph), p.z + r.z * w + u.z * (h-toph));
        rlTexCoord2f((source.x + left) / width, (source.y + top) / height);
        rlVertex3f(p.x + r.x * (w-colw) + u.x * (h-toph), p.y + r.y * (w-colw) + u.y * (h-toph), p.z + r.z * (w-colw) + u.z * (h-toph));
    }

    w -= (left + right) / ppu * .5f;
    h -= (top + bottom) / ppu * .5f;
    

    rlTexCoord2f((source.x + source.width - right) / width, (source.y + source.height - top) / height);
    rlVertex3f(p.x - r.x * w - u.x * h, p.y - r.y * w - u.y * h, p.z - r.z * w - u.z * h);

    rlTexCoord2f((source.x + source.width - right) / width, (source.y + bottom) / height);
    rlVertex3f(p.x - r.x * w + u.x * h, p.y - r.y * w + u.y * h, p.z - r.z * w + u.z * h);

    rlTexCoord2f((source.x + left) / width, (source.y + bottom) / height);
    rlVertex3f(p.x + r.x * w + u.x * h, p.y + r.y * w + u.y * h, p.z + r.z * w + u.z * h);

    rlTexCoord2f((source.x + left) / width, (source.y + source.height - top) / height);
    rlVertex3f(p.x + r.x * w - u.x * h, p.y + r.y * w - u.y * h, p.z + r.z * w - u.z * h);

    rlEnd();
    rlSetTexture(0);
}