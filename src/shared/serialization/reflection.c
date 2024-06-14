#include <raylib.h>
#include <stdio.h>
#include <string.h>

#include "shared/serialization/serializable_structs.h"

#include "reflection.h"

static const char* readArrayIndex(const char* path, int* index)
{
    if (path[0] != '[') {
        return NULL;
    }
    int i = 1;
    for (; path[i] && path[i] != ']' && (path[i] == ' ' || (path[i] >= '0' && path[i] <= '9')); i++) { }
    if (path[i] != ']') {
        return NULL;
    }
    for (i++; path[i] && path[i] <= ' '; i++) { }
    *index = atoi(path + 1);
    return path + i;
}

#define SERIALIZABLE_STRUCT_START(name)                                                                                     \
    int Reflect_##name##_retrieve(const char* path, name* data, void** result, size_t* resultSize, const char** resultType) \
    {
#define SERIALIZABLE_FIELD(type, name)                                                                           \
    if (strncmp(path, #name, sizeof(#name) - 1) == 0) {                                                          \
        if (path[sizeof(#name) - 1] == '\0') {                                                                   \
            *result = &data->name;                                                                               \
            *resultSize = sizeof(data->name);                                                                    \
            *resultType = #type;                                                                                 \
            return REFLECT_OK;                                                                                   \
        }                                                                                                        \
        if (path[sizeof(#name) - 1] == '.') {                                                                    \
            return Reflect_##type##_retrieve(path + sizeof(#name), &data->name, result, resultSize, resultType); \
        }                                                                                                        \
    }
#define SERIALIZABLE_FIXED_ARRAY(type, name, count)                                                                 \
    if (strncmp(path, #name, sizeof(#name) - 1) == 0) {                                                             \
        if (path[sizeof(#name) - 1] == '\0') {                                                                      \
            *result = &data->name;                                                                                  \
            *resultSize = sizeof(data->name);                                                                       \
            *resultType = #type;                                                                                    \
            return REFLECT_OK;                                                                                      \
        }                                                                                                           \
        if (path[sizeof(#name) - 1] == '[') {                                                                       \
            int index;                                                                                              \
            const char* next = readArrayIndex(path + sizeof(#name) - 1, &index);                                    \
            if (next == NULL) {                                                                                     \
                return REFLECT_SYNTAX_ERROR;                                                                        \
            }                                                                                                       \
            if (index >= 0 && index < count) {                                                                      \
                if (next[0] == '\0') {                                                                              \
                    *result = &data->name[index];                                                                   \
                    *resultSize = sizeof(data->name[index]);                                                        \
                    *resultType = #type;                                                                            \
                    return REFLECT_OK;                                                                              \
                }                                                                                                   \
                if (next[0] == '.') {                                                                               \
                    return Reflect_##type##_retrieve(next + 1, &data->name[index], result, resultSize, resultType); \
                }                                                                                                   \
                return REFLECT_OK;                                                                                  \
            }                                                                                                       \
            return REFLECT_INDEX_OUT_OF_BOUNDS;                                                                     \
        }                                                                                                           \
    }
#define SERIALIZABLE_STRUCT_LIST_ELEMENT(type, name)        \
    if (strncmp(path, #name, sizeof(#name) - 1) == 0) {     \
        if (path[sizeof(#name) - 1] == '\0') {              \
            *result = &data->name;                          \
            *resultSize = sizeof(data->name);               \
            *resultType = #type;                            \
            return REFLECT_OK;                              \
        }                                                   \
        if (path[sizeof(#name) - 1] == '[') {               \
            int index = atoi(path + sizeof(#name));         \
            if (index >= 0 && index < data->name##_count) { \
                *result = &data->name[index];               \
                *resultSize = sizeof(data->name[index]);    \
                *resultType = #type;                        \
                return REFLECT_OK;                          \
            }                                               \
            return REFLECT_INDEX_OUT_OF_BOUNDS;             \
        }                                                   \
    }
#define SERIALIZABLE_STRUCT_END(name) \
    return REFLECT_INVALID_PATH;      \
    }
#include "builtin_type_declarations.h"
#include "serializable_file_headers.h"

// int Reflect_Position_retrieve(const char *path, Position *position, void **data, size_t *size, const char **type) {
//     if (strcmp(path, "x") == 0) {
//         *(float **)data = &position->x;
//         return 1;
//     }
//     if (strcmp(path, "y") == 0) {
//         *(float **)data = &position->y;
//         return 1;
//     }
//     if (strcmp(path, "z") == 0) {
//         *(float **)data = &position->z;
//         return 1;
//     }
//     return 0;
// }