
#include "../plane_sim_g.h"

void ReadMappedVariables(cJSON *map, MappedVariable *variables)
{
    for (int i = 0; variables[i].name != 0; i++) {
        MappedVariable* var = &variables[i];
        cJSON* value = cJSON_GetObjectItem(map, var->name);
        if (value == NULL) {
            TraceLog(LOG_ERROR, "%s not found in level config", var->name);
            continue;
        }

        switch (var->type) {
        case VALUE_TYPE_FLOAT:
            *var->floatValue = value->valuedouble;
            break;
        case VALUE_TYPE_INT:
            *var->intValue = value->valueint;
            break;
        case VALUE_TYPE_STRING:
            *var->stringValue = value->valuestring;
            break;
        case VALUE_TYPE_BOOL:
            *var->boolValue = value->valueint;
            break;
        }
    }
}