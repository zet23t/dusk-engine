#ifndef __SERIALIZABLE_STRUCTS_H__
#define __SERIALIZABLE_STRUCTS_H__

#include <raylib.h>
#include <inttypes.h>

#define SERIALIZABLE_STRUCT_START(name) typedef struct name {
#define SERIALIZABLE_STRUCT_END(name) } name;
#define SERIALIZABLE_FIELD(type, name) type name;
#define SERIALIZABLE_CSTR(name) char* name;
#define SERIALIZABLE_STRUCT_LIST_ELEMENT(type, name) \
    type* name;                         \
    int name##_count;                   \
    int name##_capacity;
#define NONSERIALIZABLE_STRUCT_LIST_ELEMENT(type, name) \
    type* name;                         \
    int name##_count;                   \
    int name##_capacity;
#define NONSERIALIZED_FIELD(type, name) type name;

#include "serializable_file_headers.h"

#endif