#include <raylib.h>
#include <raymath.h>

#define RAND_F_MAX 0x7fffff
float GetRandomFloat(float min, float max)
{
    return min + (max - min) * ((float)GetRandomValue(0, RAND_F_MAX) / (float)RAND_F_MAX);
}