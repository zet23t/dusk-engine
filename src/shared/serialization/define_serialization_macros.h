#ifndef SVAL
#define SVAL(...) __VA_ARGS__
#endif
#ifndef SERIALIZABLE_STRUCT_START
#define SERIALIZABLE_STRUCT_START(name)
#endif

#ifndef SERIALIZABLE_ANNOTATION
#define SERIALIZABLE_ANNOTATION(key, type, value)
#endif

#ifndef SERIALIZABLE_FIELD
#define SERIALIZABLE_FIELD(type,name)
#endif

#ifndef SERIALIZABLE_FIXED_ARRAY
#define SERIALIZABLE_FIXED_ARRAY(type,name,count)
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

#ifndef POST_DESERIALIZE_CALLBACK
#define POST_DESERIALIZE_CALLBACK(type, name)
#endif

#ifndef POST_SERIALIZE_CALLBACK
#define POST_SERIALIZE_CALLBACK(type, name)
#endif

#ifndef SERIALIZABLE_STRUCT_END
#define SERIALIZABLE_STRUCT_END(name)
#endif
