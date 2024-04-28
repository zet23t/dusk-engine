#undef COMPONENT_NAME
#define COMPONENT_NAME PrimitiveRendererComponent

#ifdef COMPONENT
// === COMPONENT UTILITIES ===
COMPONENT(PrimitiveRendererComponent)
#elif defined(COMPONENT_DECLARATION)

// === DECLARATIONS ===
#ifndef PRIMITIVE_TYPE_CUBE
#define PRIMITIVE_TYPE_CUBE 0
#define PRIMITIVE_TYPE_SPHERE 1
#define PRIMITIVE_TYPE_CYLINDER 2
#endif

SERIALIZABLE_STRUCT_START(PrimitiveRendererComponent)
    SERIALIZABLE_FIELD(Color, color)
    SERIALIZABLE_FIELD(Vector3, size)
    SERIALIZABLE_FIELD(int, primitiveType)
    SERIALIZABLE_FIELD(bool, isWireframe)
SERIALIZABLE_STRUCT_END(PrimitiveRendererComponent)

#else

// === DEFINITIONS ===
#include "../game_g.h"
#include <stdio.h>
#include <stdlib.h>
#include <rlgl.h>

void PrimitiveRenderComponent_draw(Camera3D camera, SceneObject* node, SceneComponentId id, void *componentData, void *ud)
{
    COMPONENT_NAME *self = (COMPONENT_NAME *)componentData;
    Matrix m = SceneObject_getWorldMatrix(node);
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(m));
    if (self->primitiveType == PRIMITIVE_TYPE_CUBE)
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

#include "../util/component_macros.h"

BEGIN_COMPONENT_REGISTRATION(PrimitiveRendererComponent) {
    .draw = PrimitiveRenderComponent_draw,
} END_COMPONENT_REGISTRATION

#endif