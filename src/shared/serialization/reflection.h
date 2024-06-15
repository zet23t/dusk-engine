#ifndef __REFLECTION_H__
#define __REFLECTION_H__

#define REFLECT_OK 0
#define REFLECT_SYNTAX_ERROR 1
#define REFLECT_INVALID_PATH 2
#define REFLECT_NULL_POINTER 3
#define REFLECT_INDEX_OUT_OF_BOUNDS 4
#define REFLECT_INVALID_TYPE 5

// Retrieves a value from a struct via path. "path" is a string with the format "field1.field2.field3"
// pointers are dereferenced in the process unless it's a NULL value. If the last element is
// a pointer, it returns a pointer to the pointer variable. Array elements are accessed via
// the format "field[index]". If the last element is an array, it returns a pointer to the
// array pointer. Indexing is only allowed with fixed arrays and list elements. Non serialized
// fields are ignored. 
// Returns 0 on success, 1 on syntax error, 2 on invalid path, 3 on NULL pointer, 4 on index out of bounds
#define SERIALIZABLE_STRUCT_START(name) int Reflect_##name##_retrieve(const char *path, name* data, void **result, size_t *resultSize, const char **resultType);
#include "serializable_file_headers.h"

int Reflect_retrieve(const char* path, void* data, const char* type, void** result, size_t* resultSize, const char** resultType);

#endif