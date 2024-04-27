#ifndef __SERIALIZERS_H__
#define __SERIALIZERS_H__

#include "external/cjson.h"
#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>
#include <inttypes.h>

void SerializeData_int(const char *key, int* data, cJSON *obj);
void SerializeData_Vector3(const char *key, Vector3* data, cJSON *obj);
void SerializeData_uint32_t(const char *key, uint32_t *data, cJSON *obj);
void SerializeData_int32_t(const char *key, int32_t *data, cJSON *obj);
void SerializeData_size_t(const char *key, size_t *data, cJSON *obj);

#define POST_DESERIALIAZE_CALLBACK(type, name) void name(const char *key, type* data, cJSON *obj);
#define POST_SERIALIZE_CALLBACK(type, name) void name(const char *key, type* data, cJSON *obj);
#include "serializable_file_headers.h"


#endif