#include "serializers.h"

// === SERIALIZATION ===
void SerializeData_int(const char *key, int* data, cJSON *obj) {
    cJSON_AddNumberToObject(obj, key, *data);
}
void SerializeData_Vector3(const char *key, Vector3* data, cJSON *obj) {
    cJSON *element = cJSON_AddArrayToObject(obj, key);
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->x));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->y));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->z));
}
void SerializeData_uint32_t(const char *key, uint32_t *data, cJSON *obj) {
    cJSON_AddNumberToObject(obj, key, *data);
}
void SerializeData_int32_t(const char *key, int32_t *data, cJSON *obj) {
    cJSON_AddNumberToObject(obj, key, *data);
}
void SerializeData_size_t(const char *key, size_t *data, cJSON *obj) {
    cJSON_AddNumberToObject(obj, key, *data);
}

#define SERIALIZABLE_STRUCT_START(name) void SerializeData_##name(const char *key, name* data, cJSON *obj) {\
    cJSON *element __attribute__((unused)) = obj == NULL ? cJSON_CreateObject() : cJSON_AddObjectToObject(obj, key);
#define SERIALIZABLE_FIELD(type, name) SerializeData_##type(#name, &data->name, element);
#define SERIALIZABLE_CSTR(name) cJSON_AddStringToObject(element, #name, data->name);
#define SERIALIZABLE_STRUCT_LIST_ELEMENT(type, name) \
    cJSON *array##name = cJSON_AddArrayToObject(element, #name); \
    for (int i = 0; i < data->name##_count; i++) { \
        cJSON *element = cJSON_CreateObject(); \
        SerializeData_##type(NULL, &data->name[i], element); \
        cJSON_AddItemToArray(array##name, element); \
    }
#define POST_SERIALIZE_CALLBACK(type, name) name(key, data, element);
#define SERIALIZABLE_STRUCT_END(name) }
#include "serializable_file_headers.h"

