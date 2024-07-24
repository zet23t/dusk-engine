#include "game/game.h"
#include <rlgl.h>
#include "RuntimeContext.h"

// typedef struct Plane {
//     Vector3 normal;
//     float d;
// } Plane;

// typedef struct Frustum {
//     Plane planes[6];
// } Frustum;

// static void CalcFrustumPlanes(Camera3D camera, Plane planes[6])
// {
//     Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    
//     Matrix matProj = MatrixIdentity();
//     int width = GetScreenWidth();
//     int height = GetScreenHeight();

//     if (camera.projection == CAMERA_PERSPECTIVE)
//     {
//         // Calculate projection matrix from perspective
//         matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)width/(double)height), camera.near, camera.far);
//     }
//     else if (camera.projection == CAMERA_ORTHOGRAPHIC)
//     {
//         double aspect = (double)width/(double)height;
//         double top = camera.fovy/2.0;
//         double right = top*aspect;

//         // Calculate projection matrix from orthographic
//         matProj = MatrixOrtho(-right, right, -top, top, 0.01, 1000.0);
//     }

//     // Concatenate view and projection matrices
//     Matrix matViewProj = MatrixMultiply(matProj, matView);

//     // Calculate the six frustum planes
//     planes[0].normal.x = matViewProj.m3 + matViewProj.m0;
//     planes[0].normal.y = matViewProj.m7 + matViewProj.m4;
//     planes[0].normal.z = matViewProj.m11 + matViewProj.m8;
//     planes[0].d = matViewProj.m15 + matViewProj.m12;

//     planes[1].normal.x = matViewProj.m3 - matViewProj.m0;
//     planes[1].normal.y = matViewProj.m7 - matViewProj.m4;
//     planes[1].normal.z = matViewProj.m11 - matViewProj.m8;
//     planes[1].d = matViewProj.m15 - matViewProj.m12;

//     planes[2].normal.x = matViewProj.m3 + matViewProj.m1;
//     planes[2].normal.y = matViewProj.m7 + matViewProj.m5;
//     planes[2].normal.z = matViewProj.m11 + matViewProj.m9;
//     planes[2].d = matViewProj.m15 + matViewProj.m13;

//     planes[3].normal.x = matViewProj.m3 - matViewProj.m1;
//     planes[3].normal.y = matViewProj.m7 - matViewProj.m5;
//     planes[3].normal.z = matViewProj.m11 - matViewProj.m9;
//     planes[3].d = matViewProj.m15 - matViewProj.m13;

//     planes[4].normal.x = matViewProj.m3 - matViewProj.m2;
//     planes[4].normal.y = matViewProj.m7 - matViewProj.m6;
//     planes[4].normal.z = matViewProj.m11 - matViewProj.m10;
//     planes[4].d = matViewProj.m15 - matViewProj.m14;

//     planes[5].normal.x = matViewProj.m3 + matViewProj.m2;
//     planes[5].normal.y = matViewProj.m7 + matViewProj.m6;
//     planes[5].normal.z = matViewProj.m11 + matViewProj.m10;
//     planes[5].d = matViewProj.m15 + matViewProj.m14;

// }

// static int GetFrustumSectorOfVertice(Camera3D camera, Vector3 v)
// {
// }

// static int SpriteRendererComponent_isCulled(Camera3D camera, SceneObject *sceneObject, SpriteRendererComponent *spriteRenderer)
// {
//     Matrix m = SceneObject_getWorldMatrix(sceneObject);
//     float w = spriteRenderer->size.x;
//     float h = spriteRenderer->size.y;

//     Vector3 r = (Vector3){-m.m0 * .5f, -m.m1 * .5f, -m.m2 * .5f};
//     Vector3 u = (Vector3){m.m4 * .5f, m.m5 * .5f, m.m6 * .5f};
//     Vector3 p = {m.m12, m.m13, m.m14};
//     float pivotX = (0.5f - spriteRenderer->pivot.x) * 2.0f;
//     float pivotY = (0.5f - spriteRenderer->pivot.y) * 2.0f;
//     p.x -= r.x * w * pivotX + u.x * h * pivotY;
//     p.y -= r.y * w * pivotX + u.y * h * pivotY;

//     int width = spriteRenderer->spriteAsset.texture.width;
//     int height = spriteRenderer->spriteAsset.texture.height;

//     Vector3 v0 = (Vector3){p.x + r.x * w + u.x * h, p.y + r.y * w + u.y * h, p.z + r.z * w + u.z * h};
//     Vector3 v1 = (Vector3){p.x - r.x * w + u.x * h, p.y - r.y * w + u.y * h, p.z - r.z * w + u.z * h};
//     Vector3 v2 = (Vector3){p.x - r.x * w - u.x * h, p.y - r.y * w - u.y * h, p.z - r.z * w - u.z * h};
//     Vector3 v3 = (Vector3){p.x + r.x * w - u.x * h, p.y + r.y * w - u.y * h, p.z + r.z * w - u.z * h};

    
    
// }

static int SpriteRendererComponent_drawSprite(Camera3D camera, int activeTextureId, SceneObject *sceneObject, SpriteRendererComponent *spriteRenderer)
{
    Matrix m = SceneObject_getWorldMatrix(sceneObject);
    float w = spriteRenderer->size.x;
    float h = spriteRenderer->size.y;

    Vector3 r = (Vector3){-m.m0 * .5f, -m.m1 * .5f, -m.m2 * .5f};
    Vector3 u = (Vector3){m.m4 * .5f, m.m5 * .5f, m.m6 * .5f};
    Vector3 p = {m.m12, m.m13, m.m14};
    if (spriteRenderer->snapping > 0.0f)
    {
        p.x = (int)(p.x / spriteRenderer->snapping) * spriteRenderer->snapping;
        p.y = (int)(p.y / spriteRenderer->snapping) * spriteRenderer->snapping;
    }
    float pivotX = (0.5f - spriteRenderer->pivot.x) * 2.0f;
    float pivotY = (0.5f - spriteRenderer->pivot.y) * 2.0f;
    p.x -= r.x * w * pivotX + u.x * h * pivotY;
    p.y -= r.y * w * pivotX + u.y * h * pivotY;

    int width = spriteRenderer->spriteAsset.texture.width;
    int height = spriteRenderer->spriteAsset.texture.height;
    Rectangle source = spriteRenderer->spriteAsset.source;

    if (activeTextureId != spriteRenderer->spriteAsset.texture.id)
    {
        activeTextureId = spriteRenderer->spriteAsset.texture.id;
        rlSetTexture(spriteRenderer->spriteAsset.texture.id);
        rlBegin(RL_QUADS);
    }
    Color tint = spriteRenderer->tint;
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(m.m9, m.m10, m.m11);

    if (spriteRenderer->spriteAsset.scale9frame.x == 0 && spriteRenderer->spriteAsset.scale9frame.y == 0 &&
        spriteRenderer->spriteAsset.scale9frame.z == 0 && spriteRenderer->spriteAsset.scale9frame.w == 0)
    {
        float fx = spriteRenderer->flip.x;
        float fy = spriteRenderer->flip.y;
        Vector3 v0 = (Vector3){p.x + r.x * w + u.x * h, p.y + r.y * w + u.y * h, p.z + r.z * w + u.z * h};
        Vector3 t0 = (Vector3){(source.x) / width, (source.y) / height};
        Vector3 v1 = (Vector3){p.x - r.x * w + u.x * h, p.y - r.y * w + u.y * h, p.z - r.z * w + u.z * h};
        Vector3 t1 = (Vector3){(source.x + source.width) / width, (source.y) / height};
        Vector3 v2 = (Vector3){p.x - r.x * w - u.x * h, p.y - r.y * w - u.y * h, p.z - r.z * w - u.z * h};
        Vector3 t2 = (Vector3){(source.x + source.width) / width, (source.y + source.height) / height};
        Vector3 v3 = (Vector3){p.x + r.x * w - u.x * h, p.y + r.y * w - u.y * h, p.z + r.z * w - u.z * h};
        Vector3 t3 = (Vector3){(source.x) / width, (source.y + source.height) / height};
        if (fx < 0.0f) {
            Vector3 tmp = t0;
            t0 = t1;
            t1 = tmp;
            tmp = t2;
            t2 = t3;
            t3 = tmp;
        }

        if (fy < 0.0f) {
            Vector3 tmp = t2;
            t2 = t3;
            t3 = tmp;

            tmp = t0;
            t0 = t1;
            t1 = tmp;
        }
#define P(XY) rlTexCoord2f(t##XY.x, t##XY.y), rlVertex3f(v##XY.x, v##XY.y, v##XY.z);
        P(0)
        P(3)
        P(2)
        P(1)
#undef P
    }
    else
    {

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

        // inner width and height
        float iw = w - (left + right) / ppu * .5f;
        float ih = h - (top + bottom) / ppu * .5f;

        Vector3 v00 = (Vector3){p.x + r.x * w + u.x * h, p.y + r.y * w + u.y * h, p.z + r.z * w + u.z * h};
        Vector3 t00 = (Vector3){(source.x) / width, (source.y) / height};
        Vector3 v10 = (Vector3){p.x + r.x * iw + u.x * h, p.y + r.y * iw + u.y * h, p.z + r.z * iw + u.z * h};
        Vector3 t10 = (Vector3){(source.x + left) / width, (source.y) / height};
        Vector3 v20 = (Vector3){p.x - r.x * iw + u.x * h, p.y - r.y * iw + u.y * h, p.z - r.z * iw + u.z * h};
        Vector2 t20 = (Vector2){(source.x + source.width - right) / width, (source.y) / height};
        Vector3 v30 = (Vector3){p.x - r.x * w + u.x * h, p.y - r.y * w + u.y * h, p.z - r.z * w + u.z * h};
        Vector2 t30 = (Vector2){(source.x + source.width) / width, (source.y) / height};

        Vector3 v01 = (Vector3){p.x + r.x * w + u.x * ih, p.y + r.y * w + u.y * ih, p.z + r.z * w + u.z * ih};
        Vector3 t01 = (Vector3){(source.x) / width, (source.y + bottom) / height};
        Vector3 v11 = (Vector3){p.x + r.x * iw + u.x * ih, p.y + r.y * iw + u.y * ih, p.z + r.z * iw + u.z * ih};
        Vector3 t11 = (Vector3){(source.x + left) / width, (source.y + bottom) / height};
        Vector3 v21 = (Vector3){p.x - r.x * iw + u.x * ih, p.y - r.y * iw + u.y * ih, p.z - r.z * iw + u.z * ih};
        Vector2 t21 = (Vector2){(source.x + source.width - right) / width, (source.y + bottom) / height};
        Vector3 v31 = (Vector3){p.x - r.x * w + u.x * ih, p.y - r.y * w + u.y * ih, p.z - r.z * w + u.z * ih};
        Vector2 t31 = (Vector2){(source.x + source.width) / width, (source.y + bottom) / height};

        Vector3 v02 = (Vector3){p.x + r.x * w - u.x * ih, p.y + r.y * w - u.y * ih, p.z + r.z * w - u.z * ih};
        Vector2 t02 = (Vector2){(source.x) / width, (source.y + source.height - top) / height};
        Vector3 v12 = (Vector3){p.x + r.x * iw - u.x * ih, p.y + r.y * iw - u.y * ih, p.z + r.z * iw - u.z * ih};
        Vector2 t12 = (Vector2){(source.x + left) / width, (source.y + source.height - top) / height};
        Vector3 v22 = (Vector3){p.x - r.x * iw - u.x * ih, p.y - r.y * iw - u.y * ih, p.z - r.z * iw - u.z * ih};
        Vector2 t22 = (Vector2){(source.x + source.width - right) / width, (source.y + source.height - top) / height};
        Vector3 v32 = (Vector3){p.x - r.x * w - u.x * ih, p.y - r.y * w - u.y * ih, p.z - r.z * w - u.z * ih};
        Vector2 t32 = (Vector2){(source.x + source.width) / width, (source.y + source.height - top) / height};

        Vector3 v03 = (Vector3){p.x + r.x * w - u.x * h, p.y + r.y * w - u.y * h, p.z + r.z * w - u.z * h};
        Vector2 t03 = (Vector2){(source.x) / width, (source.y + source.height) / height};
        Vector3 v13 = (Vector3){p.x + r.x * iw - u.x * h, p.y + r.y * iw - u.y * h, p.z + r.z * iw - u.z * h};
        Vector2 t13 = (Vector2){(source.x + left) / width, (source.y + source.height) / height};
        Vector3 v23 = (Vector3){p.x - r.x * iw - u.x * h, p.y - r.y * iw - u.y * h, p.z - r.z * iw - u.z * h};
        Vector2 t23 = (Vector2){(source.x + source.width - right) / width, (source.y + source.height) / height};
        Vector3 v33 = (Vector3){p.x - r.x * w - u.x * h, p.y - r.y * w - u.y * h, p.z - r.z * w - u.z * h};
        Vector2 t33 = (Vector2){(source.x + source.width) / width, (source.y + source.height) / height};

#define P(XY) rlTexCoord2f(t##XY.x, t##XY.y), rlVertex3f(v##XY.x, v##XY.y, v##XY.z);
#define Q(A, B, C, D) P(A) P(B) P(C) P(D)
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
    }

    return activeTextureId;
}

static uint32_t *orderedIndicesCacheA = NULL;
static uint32_t *orderedIndicesCacheB = NULL;
static uint32_t orderedIndicesCacheSize = 0;
static uint32_t previousComponentCount = 0;

static int SpriteRendererComponent_sortSprites(SpriteRendererComponent *sprites, SceneGraph* graph, SceneComponentType type)
{
    if (orderedIndicesCacheSize < type.components_count)
    {
        orderedIndicesCacheA = RL_REALLOC(orderedIndicesCacheA, sizeof(uint32_t) * type.components_count);
        orderedIndicesCacheB = RL_REALLOC(orderedIndicesCacheB, sizeof(uint32_t) * type.components_count);
        orderedIndicesCacheSize = type.components_count;
        for (int i = 0; i < type.components_count; i++)
        {
            orderedIndicesCacheA[i] = i;
            orderedIndicesCacheB[i] = i;
        }
    }

    // remove entries from orderedIndicesCacheA that are not active / destroyed
    int componentCount = 0;
    for (int i = 0; i < type.components_count; i++)
    {
        if (SceneGraph_isObjectEnabled(graph, type.components[i].objectId) && SceneGraph_isComponentEnabled(graph, type.components[i].id))
        {
            SceneObjectId parent = SceneGraph_getObject(graph, type.components[i].objectId)->parent;
            int isActive = 1;
            while (parent.version != 0)
            {
                if (!SceneGraph_isObjectEnabled(graph, parent))
                {
                    isActive = 0;
                    break;
                }
                parent = SceneGraph_getObject(graph, parent)->parent;
            }
            if (isActive){
                orderedIndicesCacheB[componentCount] = i;
                orderedIndicesCacheA[componentCount++] = -1;
            }
            else
                ((SpriteRendererComponent*)type.componentData)[i].previousSortIndex = -1;
        }
    }

    // trying to restore previous order as good as possible:
    for (int i = 0; i < componentCount; i++)
    {
        int index = orderedIndicesCacheB[i];
        int previousIndex = sprites[index].previousSortIndex;
        if (previousIndex >= 0 && previousIndex < componentCount)
        {
            orderedIndicesCacheA[previousIndex] = i;
        }
        else
        {
            sprites[index].previousSortIndex = -1;
        }
    }
    int insertionIndex = 0;
    // insert elements with previous sort index = -1 into places that are -1
    for (int i = 0; i < componentCount; i++)
    {
        int index = orderedIndicesCacheB[i];
        if (sprites[index].previousSortIndex == -1)
        {
            while (orderedIndicesCacheA[insertionIndex] != -1)
            {
                insertionIndex++;
            }
            orderedIndicesCacheA[insertionIndex] = i;
        }
    }

    // debug: check if no index is -1
    for (int i = 0; i < componentCount; i++)
    {
        if (orderedIndicesCacheA[i] == -1)
        {
            printf("Sorting failed at %d: %d\n", i, orderedIndicesCacheA[i]);
            return 0;
        }
    }

    previousComponentCount = componentCount;
    if (componentCount == 0)
    {
        return 0;
    }

    // adaptive merge sort on orderedIndicesCacheA which may be partially sorted
    // based on the sortId of the spriteComponents
    // strategy:
    //  - determine length of sorted elements from starting point i as length_1
    //  - determine length of sorted elements from starting point i + length_1 as length_2
    //  - merge the two sorted lists
    //  - repeat until all elements are sorted
    int sorted = 0;
    // printf("Begin sorting %d\n", componentCount);
    int sortingStepCount = 0;
    while (!sorted)
    {
        sortingStepCount++;
        int startA = 0, endA = 0, startB = 0, endB = 0;
        int k = 0;
        // debug code
        for (int i=0;i<componentCount;i++)
        {
            orderedIndicesCacheB[i] = -1;
        }

        for (startA = 0; startA < componentCount;)
        {
            endA = startA;
            while (endA < componentCount - 1 && sprites[orderedIndicesCacheA[endA]].sortId <= sprites[orderedIndicesCacheA[endA + 1]].sortId)
            {
                endA++;
            }
            if (endA == componentCount - 1)
            {
                int i = startA;
                while (i <= endA)
                {
                    orderedIndicesCacheB[k++] = orderedIndicesCacheA[i++];
                }
                break;
            }
            startB = endA + 1;
            endB = startB;
            while (endB < componentCount - 1 && sprites[orderedIndicesCacheA[endB]].sortId <= sprites[orderedIndicesCacheA[endB + 1]].sortId)
            {
                endB++;
            }

            // for (int i = startA; i <= endA; i++)
            // {
            //     printf("  A %d / %d: %d\n", i, orderedIndicesCacheA[i], sprites[orderedIndicesCacheA[i]].sortId);
            // }
            // for (int i = startB; i <= endB; i++)
            // {
            //     printf("  B %d / %d: %d\n", i, orderedIndicesCacheA[i], sprites[orderedIndicesCacheA[i]].sortId);
            // }

            // merge A + B into orderedIndicesCacheB
            int i = startA;
            int j = startB;
            while (i <= endA && j <= endB)
            {
                if (sprites[orderedIndicesCacheA[i]].sortId <= sprites[orderedIndicesCacheA[j]].sortId)
                {
                    orderedIndicesCacheB[k++] = orderedIndicesCacheA[i++];
                }
                else
                {
                    orderedIndicesCacheB[k++] = orderedIndicesCacheA[j++];
                }
            }
            while (i <= endA)
            {
                orderedIndicesCacheB[k++] = orderedIndicesCacheA[i++];
            }
            while (j <= endB)
            {
                orderedIndicesCacheB[k++] = orderedIndicesCacheA[j++];
            }
            

            // for (int i = startA; i <= endB; i++)
            // {
            //     printf("  > %d / %d: %d\n", i, orderedIndicesCacheB[i], sprites[orderedIndicesCacheB[i]].sortId);
            // }
            // printf("  Sorting step %d; %d -> %d; %d -> %d\n", sortingStepCount, startA, endA, startB, endB);
            startA = endB + 1;
        }

        sorted = startA == 0 && endA == componentCount - 1;
        if (sorted) break;

        for (int i=0;i<componentCount;i++)
        {
            if (orderedIndicesCacheB[i] == -1)
            {
                printf(" Sorting failed at %d: %d\n", i, orderedIndicesCacheB[i]);
                for (int i=0;i<componentCount;i++)
                {
                    printf(" %3d", orderedIndicesCacheA[i]);
                }
                printf("\n");
                for (int i=0;i<componentCount;i++)
                {
                    printf(" %3d", orderedIndicesCacheB[i]);
                }
                printf("\n");
                return 0;
            }
        }



        // swap orderedIndicesCacheA and orderedIndicesCacheB
        uint32_t *tmp = orderedIndicesCacheA;
        orderedIndicesCacheA = orderedIndicesCacheB;
        orderedIndicesCacheB = tmp;
    }
    return componentCount;
}

void SpriteRendererComponent_systemDraw(Camera3D camera, SceneGraph* graph, SceneComponentType type, void* userdata)
{
    SpriteRendererComponent *sprites = (SpriteRendererComponent*) type.componentData;
    
    TRACE_ENTER_CONTEXT("SpriteRendererComponent_sortSprites");
    int componentCount = SpriteRendererComponent_sortSprites(sprites, graph, type);
    TRACE_EXIT_CONTEXT();
    if (componentCount == 0)
    {
        return;
    }

    // debug: check if sorted
    // for (int i = 0; i < componentCount - 1; i++)
    // {
    //     if (sprites[orderedIndicesCacheA[i]].sortId > sprites[orderedIndicesCacheA[i + 1]].sortId)
    //     {
    //         printf("Sorting failed at %d: %d > %d\n", i, sprites[orderedIndicesCacheA[i]].sortId, sprites[orderedIndicesCacheA[i + 1]].sortId);
    //     }
    // }
    // for (int i=0;i<componentCount;i++)
    // {
    //     printf(" %+3d", sprites[orderedIndicesCacheA[i]].sortId);
    // }
    // printf("\n");
    // for (int i=0;i<componentCount;i++)
    // {
    //     printf(" %3d", orderedIndicesCacheA[i]);
    // }
    // printf("\n");
    // printf("End sorting after %d loops over %d\n", sortingStepCount, componentCount);

    rlDrawRenderBatchActive();
    rlDisableDepthTest();
    int activeTexture = 0;
    for (int i = 0; i < componentCount; i++)
    {
        int index = orderedIndicesCacheA[i];
        SpriteRendererComponent *spriteRenderer = &sprites[index];
        spriteRenderer->previousSortIndex = i;
        SceneObject *sceneObject = SceneGraph_getObject(graph, type.components[index].objectId);
        activeTexture = SpriteRendererComponent_drawSprite(camera, activeTexture, sceneObject, spriteRenderer);
    }
    if (activeTexture != 0)
    {
        rlEnd();
        rlSetTexture(0);
    }
    rlDrawRenderBatchActive();
    rlEnableDepthTest();

}

void SpriteRendererComponent_onInitialize(SceneObject *sceneObject, SceneComponentId sceneComponent, void *componentData, void *initData)
{
    SpriteRendererComponent *spriteRenderer = (SpriteRendererComponent *)componentData;
    SpriteRendererComponent *init = (SpriteRendererComponent *)initData;
    if (init != NULL)
    {
        *spriteRenderer = *init;
    }
    else
    {
        *spriteRenderer = (SpriteRendererComponent){0};
    }

    spriteRenderer->previousSortIndex = -1;
}

BEGIN_COMPONENT_REGISTRATION(SpriteRendererComponent) {
    // .sequentialDraw = SpriteRendererComponent_sequentialDraw,
    .onInitialize = SpriteRendererComponent_onInitialize,
    .drawSystem = SpriteRendererComponent_systemDraw,
} END_COMPONENT_REGISTRATION