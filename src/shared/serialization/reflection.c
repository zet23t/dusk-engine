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

int Reflect_bool_retrieve(const char* path, bool* data, void** result, size_t* resultSize, const char** resultType)
{
    if (path[0] == '\0') {
        *result = data;
        *resultSize = sizeof(*data);
        *resultType = "bool";
        return REFLECT_OK;
    }
    return REFLECT_INVALID_PATH;
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
                return REFLECT_INVALID_PATH;                                                                        \
            }                                                                                                       \
            return REFLECT_INDEX_OUT_OF_BOUNDS;                                                                     \
        }                                                                                                           \
    }
#define SERIALIZABLE_STRUCT_LIST_ELEMENT(type, name)                                                                \
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
            if (index >= 0 && index < data->name##_count) {                                                         \
                if (next[0] == '\0') {                                                                              \
                    *result = &data->name[index];                                                                   \
                    *resultSize = sizeof(data->name[index]);                                                        \
                    *resultType = #type;                                                                            \
                    return REFLECT_OK;                                                                              \
                }                                                                                                   \
                if (next[0] == '.') {                                                                               \
                    return Reflect_##type##_retrieve(next + 1, &data->name[index], result, resultSize, resultType); \
                }                                                                                                   \
                return REFLECT_INVALID_PATH;                                                                        \
            }                                                                                                       \
            return REFLECT_INDEX_OUT_OF_BOUNDS;                                                                     \
        }                                                                                                           \
    }
#define SERIALIZABLE_STRUCT_END(name) \
    return REFLECT_INVALID_PATH;      \
    }
#include "builtin_type_declarations.h"
#include "serializable_file_headers.h"

int Reflect_retrieve(const char* path, void* data, const char* type, void** result, size_t* resultSize, const char** resultType)
{
#define SERIALIZABLE_STRUCT_START(name) \
    if (strcmp(type, #name) == 0)       {\
        return Reflect_##name##_retrieve(path, (name*)data, result, resultSize, resultType);\
    }
#include "builtin_type_declarations.h"
#include "serializable_file_headers.h"

    return REFLECT_INVALID_TYPE;
}