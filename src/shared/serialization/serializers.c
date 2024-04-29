#include "serializers.h"

// === SERIALIZATION ===
void SerializeData_int(const char *key, int* data, cJSON *obj, void *userData) {
    cJSON_AddNumberToObject(obj, key, *data);
}
void SerializeData_Vector3(const char *key, Vector3* data, cJSON *obj, void *userData) {
    cJSON *element = cJSON_AddArrayToObject(obj, key);
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->x));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->y));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->z));
}
void SerializeData_Vector2(const char *key, Vector2* data, cJSON *obj, void *userData) {
    cJSON *element = cJSON_AddArrayToObject(obj, key);
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->x));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->y));
}
void SerializeData_Color(const char *key, Color* data, cJSON *obj, void *userData) {
    cJSON *element = cJSON_AddArrayToObject(obj, key);
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->r));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->g));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->b));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->a));
}
void SerializeData_Rectangle(const char *key, Rectangle* data, cJSON *obj, void *userData) {
    cJSON *element = cJSON_AddArrayToObject(obj, key);
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->x));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->y));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->width));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->height));
}
void SerializeData_bool(const char *key, bool* data, cJSON *obj, void *userData) {
    cJSON_AddBoolToObject(obj, key, *data);
}
void SerializeData_float(const char *key, float* data, cJSON *obj, void *userData) {
    cJSON_AddNumberToObject(obj, key, *data);
}
void SerializeData_uint32_t(const char *key, uint32_t *data, cJSON *obj, void *userData) {
    cJSON_AddNumberToObject(obj, key, *data);
}
void SerializeData_int32_t(const char *key, int32_t *data, cJSON *obj, void *userData) {
    cJSON_AddNumberToObject(obj, key, *data);
}
void SerializeData_size_t(const char *key, size_t *data, cJSON *obj, void *userData) {
    cJSON_AddNumberToObject(obj, key, *data);
}
void SerializeData_uint8_t(const char *key, uint8_t *data, cJSON *obj, void *userData) {
    cJSON_AddNumberToObject(obj, key, *data);
}
void SerializeData_int8_t(const char *key, int8_t *data, cJSON *obj, void *userData) {
    cJSON_AddNumberToObject(obj, key, *data);
}

#define SERIALIZABLE_STRUCT_START(name) void SerializeData_##name(const char *key, name* data, cJSON *obj, void *userData) {\
    cJSON *element __attribute__((unused)) = obj == NULL ? cJSON_CreateObject() : (key == NULL ? obj : cJSON_AddObjectToObject(obj, key));
#define SERIALIZABLE_FIELD(type, name) SerializeData_##type(#name, &data->name, element, userData);
#define SERIALIZABLE_CSTR(name) cJSON_AddStringToObject(element, #name, data->name);
#define SERIALIZABLE_STRUCT_LIST_ELEMENT(type, name) \
    cJSON *array##name = cJSON_AddArrayToObject(element, #name); \
    for (int i = 0; i < data->name##_count; i++) { \
        cJSON *element = cJSON_CreateObject(); \
        SerializeData_##type(NULL, &data->name[i], element, userData); \
        cJSON_AddItemToArray(array##name, element); \
    }
#define POST_SERIALIZE_CALLBACK(type, name) name(key, data, element, userData);
#define SERIALIZABLE_STRUCT_END(name) }
#include "serializable_file_headers.h"

