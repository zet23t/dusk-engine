#include "external/cjson.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef SERIALIZABLE_STRUCT_START
#define SERIALIZABLE_STRUCT_START(name) typedef struct name {
#define SERIALIZABLE_STRUCT_END(name) } name;
#define SERIALIZABLE_FIELD(type, name) type name;
#endif
#include "test_struct.h"
#undef SERIALIZABLE_STRUCT_START
#undef SERIALIZABLE_STRUCT_END
#undef SERIALIZABLE_FIELD

void SerializeData_int(const char *key, int* data, cJSON *obj) {
    cJSON_AddNumberToObject(obj, key, *data);
}
void SerializeData_Vector3(const char *key, Vector3* data, cJSON *obj) {
    cJSON *element = cJSON_AddArrayToObject(obj, key);
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->x));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->y));
    cJSON_AddItemToArray(element, cJSON_CreateNumber(data->z));
}

#define SERIALIZABLE_STRUCT_START(name) void SerializeData_##name(const char *key, name* data, cJSON *obj) {\
    cJSON *element = obj == NULL ? cJSON_CreateObject() : cJSON_AddObjectToObject(obj, key);
#define SERIALIZABLE_FIELD(type, name) SerializeData_##type(#name, &data->name, element);
#define SERIALIZABLE_STRUCT_END(name) }
#include "test_struct.h"
#undef SERIALIZABLE_STRUCT_START
#undef SERIALIZABLE_STRUCT_END
#undef SERIALIZABLE_FIELD

void DeserializeData_int(const char *key, int* data, cJSON *obj) {
    cJSON *element = key == NULL ? obj : cJSON_GetObjectItem(obj, key);
    *data = element->valueint;
}

void DeserializeData_Vector3(const char *key, Vector3* data, cJSON *obj) {
    cJSON *element = key == NULL ? obj : cJSON_GetObjectItem(obj, key);
    data->x = cJSON_GetArrayItem(element, 0)->valuedouble;
    data->y = cJSON_GetArrayItem(element, 1)->valuedouble;
    data->z = cJSON_GetArrayItem(element, 2)->valuedouble;
}

#define SERIALIZABLE_STRUCT_START(name) void DeserializeData_##name(const char *key, name* data, cJSON *obj) {\
    cJSON *element = key == NULL ? obj : cJSON_GetObjectItem(obj, key);
#define SERIALIZABLE_FIELD(type, name) DeserializeData_##type(#name, &data->name, element);
#define SERIALIZABLE_STRUCT_END(name) }
#include "test_struct.h"
#undef SERIALIZABLE_STRUCT_START
#undef SERIALIZABLE_STRUCT_END
#undef SERIALIZABLE_FIELD

void SerializeTest()
{
    TestStruct testStruct = { 1, { 1.0f, 2.0f, 3.0f } };
    cJSON *root = cJSON_CreateObject();
    SerializeData_TestStruct("testStruct", &testStruct, root);
    char *serialized = cJSON_Print(root);
    printf("%s\n", serialized);

    TestStruct deserialized;
    DeserializeData_TestStruct("testStruct", &deserialized, root);
    printf("%d %f %f %f\n", deserialized.iValue, deserialized.v3Value.x, deserialized.v3Value.y, deserialized.v3Value.z);
    free(serialized);
    cJSON_Delete(root);
}