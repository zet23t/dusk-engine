#include "arena.h"

void* Arena_alloc(Arena10KB* arena, size_t size)
{
    if (arena->used + size > sizeof(arena->data)) {
        return NULL;
    }
    void* result = &arena->data[arena->used];
    arena->used += size;
    return result;
}