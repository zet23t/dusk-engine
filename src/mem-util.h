
#ifndef RL_STRDUP
#define RL_STRDUP(s) ((s) ? strdup(s) : NULL)
#endif

#ifndef STRUCT_MALLOC
#define STRUCT_MALLOC(type, count) (type *)malloc(sizeof(type) * (count))
#endif

#ifndef STRUCT_REALLOC
#define STRUCT_REALLOC(ptr, type, count) (type *)realloc(ptr, sizeof(type) * (count))
#endif