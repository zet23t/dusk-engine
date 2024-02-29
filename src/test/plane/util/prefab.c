#include "../plane_sim_g.h"

#include <string.h>
static Vector3 VectorFromJSONArray(cJSON *array, Vector3 def)
{
    if (array == NULL || !cJSON_IsArray(array) || cJSON_GetArraySize(array) != 3) {
        return def;
    }

    cJSON* x = cJSON_GetArrayItem(array, 0);
    cJSON* y = cJSON_GetArrayItem(array, 1);
    cJSON* z = cJSON_GetArrayItem(array, 2);

    return (Vector3) {
        cJSON_IsNumber(x) ? (float) x->valuedouble : def.x,
        cJSON_IsNumber(y) ? (float) y->valuedouble : def.y,
        cJSON_IsNumber(z) ? (float) z->valuedouble : def.z,
    };
}

static Vector3 VectorFromJSONArrayN(cJSON *json, const char *key, Vector3 def)
{
    cJSON* value = cJSON_GetObjectItemCaseSensitive(json, key);
    return VectorFromJSONArray(value, def);
}

static void SceneObject_ApplyJSONValues(SceneGraph* sceneGraph, cJSON* objects, SceneObjectId nodeId, cJSON* object)
{
    TraceLog(LOG_INFO, "Applying JSON values to object %s", SceneGraph_getObjectName(sceneGraph, nodeId));
    SceneGraph_setLocalPosition(psg.sceneGraph, nodeId, VectorFromJSONArrayN(object, "position", SceneGraph_getLocalPosition(sceneGraph, nodeId)));
    SceneGraph_setLocalRotation(psg.sceneGraph, nodeId, VectorFromJSONArrayN(object, "rotation", SceneGraph_getLocalRotation(sceneGraph, nodeId)));
    SceneGraph_setLocalScale(psg.sceneGraph, nodeId, VectorFromJSONArrayN(object, "scale", SceneGraph_getLocalScale(sceneGraph, nodeId)));

    cJSON* components = cJSON_GetObjectItemCaseSensitive(object, "components");
    if (components != NULL) {
        int componentsCount = cJSON_GetArraySize(components);
        for (int i = 0; i < componentsCount; i++) {
            cJSON* reference = cJSON_GetArrayItem(components, i);
            if (!cJSON_IsString(reference)) {
                TraceLog(LOG_ERROR, "Invalid component reference in components list of object %s", SceneGraph_getObjectName(sceneGraph, nodeId));
                continue;
            }
            cJSON* component = cJSON_GetObjectItemCaseSensitive(objects, reference->valuestring);
            if (component == NULL) {
                TraceLog(LOG_ERROR, "Component %s not found in objects list (referred by %s)", reference->valuestring, SceneGraph_getObjectName(sceneGraph, nodeId));
                continue;
            }
            cJSON* type = cJSON_GetObjectItemCaseSensitive(component, "type");
            if (!cJSON_IsString(type)) {
                TraceLog(LOG_ERROR, "No 'type' string in components list of object %s", SceneGraph_getObjectName(sceneGraph, nodeId));
                continue;
            }
            // TODO: hardcoding the components here to avoid rabbit hole descent
            const char *componentType = type->valuestring;
            if (strcmp(componentType, "MeshRendererComponent") == 0) {
                cJSON* mesh = cJSON_GetObjectItemCaseSensitive(component, "mesh");
                cJSON* litAmount = cJSON_GetObjectItemCaseSensitive(component, "litAmount");
                if (cJSON_IsString(mesh)) {
                    SceneGraph_addComponent(psg.sceneGraph, nodeId, psg.meshRendererComponentId,
                        &(MeshRendererComponent) {
                            .litAmount = cJSON_IsNumber(litAmount) ? (float) litAmount->valuedouble : 0.0f,
                            .material = &psg.model.materials[1],
                            .mesh = psg.meshUiBorder,
                        });
                    Vector3 worldPos = SceneGraph_getWorldPosition(psg.sceneGraph, nodeId);
                    TraceLog(LOG_INFO, "Added MeshRendererComponent to object %s at %f %f %f", SceneGraph_getObjectName(sceneGraph, nodeId),
                        worldPos.x, worldPos.y, worldPos.z);
                }
                else
                {
                    TraceLog(LOG_ERROR, "No 'mesh' string in MeshRendererComponent of object %s", SceneGraph_getObjectName(sceneGraph, nodeId));
                }
                continue;
            }
            
            TraceLog(LOG_ERROR, "Unknown component type %s in components list of object %s", componentType, SceneGraph_getObjectName(sceneGraph, nodeId));
        }
    }

    cJSON* children = cJSON_GetObjectItemCaseSensitive(object, "children");
    if (children != NULL) {
        int childrenCount = cJSON_GetArraySize(children);
        for (int i = 0; i < childrenCount; i++) {
            cJSON* childId = cJSON_GetArrayItem(children, i);
            if (cJSON_IsString(childId)) {
                SceneObjectId childNodeId = InstantiateFromJSON(sceneGraph, objects, childId->valuestring);
                SceneGraph_setParent(sceneGraph, childNodeId, nodeId);
                continue;
            }
            if (cJSON_IsObject(childId)) {
                cJSON *prefabRef = cJSON_GetObjectItemCaseSensitive(childId, "prefab");
                if (!cJSON_IsString(prefabRef)) {
                    TraceLog(LOG_ERROR, "No 'prefab' string in children list of object %s", SceneGraph_getObjectName(sceneGraph, nodeId));
                    continue;
                }
                cJSON *prefabJSON = cJSON_GetObjectItemCaseSensitive(objects, prefabRef->valuestring);
                if (prefabJSON == NULL) {
                    TraceLog(LOG_ERROR, "Prefab %s not found in objects list (referred by %s)", prefabRef->valuestring, SceneGraph_getObjectName(sceneGraph, nodeId));
                    continue;
                }

                SceneObjectId childNodeId = InstantiateFromJSON(sceneGraph, objects, prefabRef->valuestring);
                SceneGraph_setParent(sceneGraph, childNodeId, nodeId);
                SceneObject_ApplyJSONValues(sceneGraph, objects, childNodeId, childId);

                continue;
            }

            TraceLog(LOG_ERROR, "Invalid child id in children list of object %s", SceneGraph_getObjectName(sceneGraph, nodeId));
        }
    }

}

SceneObjectId InstantiateFromJSON(SceneGraph* sceneGraph, cJSON* objects, const char* rootId)
{
    cJSON* object = cJSON_GetObjectItemCaseSensitive(objects, rootId);
    if (object == NULL) {
        TraceLog(LOG_ERROR, "Object %s not found in objects list", rootId);
        return (SceneObjectId) { 0 };
    }

    SceneObjectId nodeId = SceneGraph_createObject(sceneGraph, rootId);
    SceneObject_ApplyJSONValues(sceneGraph, objects, nodeId, object);

    return nodeId;
}