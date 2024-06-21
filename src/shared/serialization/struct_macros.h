#ifndef SERIALIZABLE_STRUCTS_DECLARED
#define SERIALIZABLE_STRUCTS_DECLARED

#define SERIALIZABLE_STRUCT_START(name) typedef struct name name;
#include "serializable_file_headers.h"

#define SERIALIZABLE_STRUCT_START(name) typedef struct name {
#define SERIALIZABLE_STRUCT_END(name) } name;
#define SERIALIZABLE_FIELD(type, name) type name;
#define SERIALIZABLE_FIXED_ARRAY(type, name, count) type name[count];
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

#else

#include "define_serialization_macros.h"

#endif