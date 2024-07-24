#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include "game/game.h"

#ifndef COMPONENT_IMPLEMENTATION
#include "PrimitiveRendererComponent.h"
#endif

void PrimitiveRenderComponent_draw(Camera3D camera, SceneObject* node, SceneComponentId id, void *componentData, void *ud)
{
    PrimitiveRendererComponent *self = (PrimitiveRendererComponent *)componentData;
    Matrix m = SceneObject_getWorldMatrix(node);
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(m));
    if (self->primitiveType == PRIMITIVE_TYPE_ORIGIN)
    {
        DrawLine3D((Vector3){-1, 0, 0}, (Vector3){1, 0, 0}, RED);
        DrawLine3D((Vector3){0, -1, 0}, (Vector3){0, 1, 0}, GREEN);
        DrawLine3D((Vector3){0, 0, -1}, (Vector3){0, 0, 1}, BLUE);
    }
    else if (self->primitiveType == PRIMITIVE_TYPE_CUBE)
    {
        if (self->isWireframe)
        {
            DrawCubeWiresV(Vector3Zero(), self->size, self->color);
        }
        else
        {
            DrawCubeV(Vector3Zero(), self->size, self->color);
        }
    }

    rlPopMatrix();
}

BEGIN_COMPONENT_REGISTRATION(PrimitiveRendererComponent) {
    .draw = PrimitiveRenderComponent_draw,
} END_COMPONENT_REGISTRATION