
#include "../game_g.h"

void ReadMappedVariables(cJSON *map, MappedVariable *variables)
{
    for (int i = 0; variables[i].name != 0; i++) {
        MappedVariable* var = &variables[i];
        cJSON* value = cJSON_GetObjectItem(map, var->name);
        if (value == NULL) {
            if (var->isRequired)
                TraceLog(LOG_ERROR, "%s not found in level config", var->name);
            continue;
        }

        if (var->valuePresentCounter != NULL) {
            *var->valuePresentCounter += 1;
        }

        switch (var->type) {
        case VALUE_TYPE_FLOAT:
            *var->floatValue = value->valuedouble;
            break;
        case VALUE_TYPE_INT:
            *var->intValue = value->valueint;
            break;
        case VALUE_TYPE_INT32:
            *var->int32Value = (int32_t) value->valueint;
            break;
        case VALUE_TYPE_STRING:
            *var->stringValue = value->valuestring;
            break;
        case VALUE_TYPE_BOOL:
            *var->boolValue = value->valueint;
            break;
        case VALUE_TYPE_VEC2:
            var->vec2Value->x = (float) cJSON_GetArrayItem(value, 0)->valuedouble;
            var->vec2Value->y = (float) cJSON_GetArrayItem(value, 1)->valuedouble;
            break;
        case VALUE_TYPE_VEC3:
            var->vec3Value->x = (float) cJSON_GetArrayItem(value, 0)->valuedouble;
            var->vec3Value->y = (float) cJSON_GetArrayItem(value, 1)->valuedouble;
            var->vec3Value->z = (float) cJSON_GetArrayItem(value, 2)->valuedouble;
            break;
        }
    }
}