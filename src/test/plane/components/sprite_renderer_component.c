#undef COMPONENT_NAME
#define COMPONENT_NAME SpriteRendererComponent

#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(SpriteRendererComponent)
#elif defined(COMPONENT_DECLARATION)
typedef struct COMPONENT_NAME {
    Texture2D texture;
    Rectangle source;
    Color tint;
    Vector2 pivot;
    Vector2 scale;
} COMPONENT_NAME;
#endif

#ifdef COMPONENT_IMPLEMENTATION
// === COMPONENT IMPLEMENTATION ===
#include "../plane_sim_g.h"
#include "../util/component_macros.h"
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

    rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
    rlVertex3f(p.x - r.x - u.x, p.y - r.y - u.y, p.z - r.z - u.z);

    rlTexCoord2f((source.x + source.width) / width, source.y / height);
    rlVertex3f(p.x - r.x + u.x, p.y - r.y + u.y, p.z - r.z + u.z);

    rlTexCoord2f(source.x / width, source.y / height);
    rlVertex3f(p.x + r.x + u.x, p.y + r.y + u.y, p.z + r.z + u.z);

    rlTexCoord2f(source.x / width, (source.y + source.height) / height);
    rlVertex3f(p.x + r.x - u.x, p.y + r.y - u.y, p.z + r.z - u.z);

    rlEnd();
    rlSetTexture(0);
}

BEGIN_COMPONENT_REGISTRATION {
    .sequentialDraw = SpriteRendererComponent_sequentialDraw,
} END_COMPONENT_REGISTRATION

#endif