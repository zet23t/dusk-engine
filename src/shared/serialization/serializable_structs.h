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
#define POST_DESERIALIAZE_CALLBACK(name)

#define NONSERIALIZED_FIELD(type, name) type name;
#include "serializable_file_headers.h"
#undef SERIALIZABLE_STRUCT_START
#undef SERIALIZABLE_STRUCT_END
#undef SERIALIZABLE_FIELD
#undef NONSERIALIZED_FIELD
#undef SERIALIZABLE_CSTR
#undef SERIALIZABLE_STRUCT_LIST_ELEMENT
#undef POST_DESERIALIAZE_CALLBACK

#endif