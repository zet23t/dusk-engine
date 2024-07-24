#include <raylib.h>
#include <raymath.h>

void GraphicsDrawArrow(Camera3D camera, Vector3 a, Vector3 b, float thickness, Color color)
{
    Matrix m = GetCameraMatrix(camera);
    Vector3 points[9];
    Vector3 dir = Vector3Normalize(Vector3Subtract(b, a));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(dir, (Vector3){m.m2, m.m6, m.m10}));
    Vector3 side = Vector3Scale(right, thickness * 0.5f);
    Vector3 c = Vector3Subtract(b, Vector3Scale(dir, thickness * 4.0f));
    Vector3 arrowSide = Vector3Scale(right, thickness * 2.0f);
    points[0] = Vector3Subtract(a, side);
    points[1] = Vector3Add(a, side);
    points[2] = Vector3Subtract(c, side);
    points[3] = Vector3Add(c, side);
    points[4] = points[3];
    points[5] = Vector3Add(c, arrowSide);
    points[6] = Vector3Subtract(c, arrowSide);
    points[7] = b;

    DrawTriangleStrip3D(points, 8, color);
}