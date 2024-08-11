#include <raymath.h>
#include <raylib.h>

#define RANGE 0xffffff
float GetRandomFloat(float min, float max)
{
    float randFloat = GetRandomValue(-RANGE, RANGE) / (float)RANGE;
    return min + (max - min) * randFloat;
}