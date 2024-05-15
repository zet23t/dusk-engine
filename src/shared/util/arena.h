#ifndef ARENA_H
#define ARENA_H

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Arena10KB {
    uint8_t data[10000];
    size_t used;
} Arena10KB;

void* Arena_alloc(Arena10KB* arena, size_t size);

#endif