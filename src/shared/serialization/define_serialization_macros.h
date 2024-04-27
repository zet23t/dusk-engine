#ifndef SERIALIZABLE_STRUCT_START
#define SERIALIZABLE_STRUCT_START(name)
#endif

#ifndef SERIALIZABLE_FIELD
#define SERIALIZABLE_FIELD(type,name)
#endif

#ifndef SERIALIZABLE_CSTR
#define SERIALIZABLE_CSTR(name)
#endif

#ifndef SERIALIZABLE_STRUCT_LIST_ELEMENT
#define SERIALIZABLE_STRUCT_LIST_ELEMENT(type,name)
#endif

#ifndef NONSERIALIZABLE_STRUCT_LIST_ELEMENT
#define NONSERIALIZABLE_STRUCT_LIST_ELEMENT(type,name)
#endif

#ifndef NONSERIALIZED_FIELD
#define NONSERIALIZED_FIELD(type,name)
#endif

#ifndef POST_DESERIALIAZE_CALLBACK
#define POST_DESERIALIAZE_CALLBACK(type, name) void name(const char *key, type* data, cJSON *obj);
#endif

#ifndef POST_SERIALIZE_CALLBACK
#define POST_SERIALIZE_CALLBACK(type, name) void name(const char *key, type* data, cJSON *obj);
#endif

#ifndef SERIALIZABLE_STRUCT_END
#define SERIALIZABLE_STRUCT_END(name)
#endif
