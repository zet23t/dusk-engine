#include "../game_g.h"
#include <string.h>

void TriggerActions(SceneGraph* sceneGraph, SceneObjectId objectId, Action* actions, int count)
{
    for (int i = 0; i < count; i++) {
        Action* action = &actions[i];
        int targetIsEmpty = action->targetName == NULL || strlen(action->targetName) == 0;
        switch (action->actionType) {
        case ACTION_TYPE_ENABLE_OBJECT: {
            SceneObject* object = targetIsEmpty ? SceneGraph_getObject(sceneGraph, objectId) :  SceneGraph_findObjectByName(sceneGraph, action->targetName, 1);
            if (object) {
                SceneGraph_setObjectEnabled(sceneGraph, object->id, 1);
            }
        } break;
        case ACTION_TYPE_DISABLE_OBJECT: {
            SceneObject* object = targetIsEmpty ? SceneGraph_getObject(sceneGraph, objectId) : SceneGraph_findObjectByName(sceneGraph, action->targetName, 1);
            if (object) {
                SceneGraph_setObjectEnabled(sceneGraph, object->id, 0);
            }
        } break;
        case ACTION_TYPE_DESTROY_OBJECT: {
            SceneObject* object = targetIsEmpty ? SceneGraph_getObject(sceneGraph, objectId) : SceneGraph_findObjectByName(sceneGraph, action->targetName, 1);
            if (object) {
                SceneGraph_destroyObject(sceneGraph, object->id);
            }
        } break;
        case ACTION_TYPE_ENABLE_COMPONENT: {
            SceneComponent* component = SceneGraph_findComponentByName(sceneGraph, action->targetName, 1);
            if (component) {
                SceneGraph_setComponentEnabled(sceneGraph, component->id, 1);
            }
        } break;
        case ACTION_TYPE_DISABLE_COMPONENT: {
            SceneComponent* component = SceneGraph_findComponentByName(sceneGraph, action->targetName, 1);
            if (component) {
                SceneGraph_setComponentEnabled(sceneGraph, component->id, 0);
            }
        } break;
        case ACTION_TYPE_DESTROY_COMPONENT: {
            SceneComponent* component = SceneGraph_findComponentByName(sceneGraph, action->targetName, 1);
            if (component) {
                SceneGraph_destroyComponent(sceneGraph, component->id);
            }
        } break;
        case ACTION_TYPE_ENABLE_CHILD_OBJECT: {
            SceneObject* object = targetIsEmpty ? SceneGraph_getObject(sceneGraph, objectId) : SceneGraph_findChildByName(sceneGraph, objectId, action->targetName, 1);
            if (object) {
                SceneGraph_setObjectEnabled(sceneGraph, object->id, 1);
            }
        } break;
        case ACTION_TYPE_DISABLE_CHILD_OBJECT: {
            SceneObject* object = targetIsEmpty ? SceneGraph_getObject(sceneGraph, objectId) : SceneGraph_findChildByName(sceneGraph, objectId, action->targetName, 1);
            if (object) {
                SceneGraph_setObjectEnabled(sceneGraph, object->id, 0);
            }
        } break;
        case ACTION_TYPE_DESTROY_CHILD_OBJECT: {
            SceneObject* object = targetIsEmpty ? SceneGraph_getObject(sceneGraph, objectId) : SceneGraph_findChildByName(sceneGraph, objectId, action->targetName, 1);
            if (object) {
                SceneGraph_destroyObject(sceneGraph, object->id);
            }
        } break;
        case ACTION_TYPE_ENABLE_CHILD_COMPONENT: {
            SceneComponent* component = SceneGraph_findChildComponentByName(sceneGraph, objectId, action->targetName, 1);
            if (component) {
                SceneGraph_setComponentEnabled(sceneGraph, component->id, 1);
            }
        } break;
        case ACTION_TYPE_DISABLE_CHILD_COMPONENT: {
            SceneComponent* component = SceneGraph_findChildComponentByName(sceneGraph, objectId, action->targetName, 1);
            if (component) {
                SceneGraph_setComponentEnabled(sceneGraph, component->id, 0);
            }
        } break;
        case ACTION_TYPE_DESTROY_CHILD_COMPONENT: {
            SceneComponent* component = SceneGraph_findChildComponentByName(sceneGraph, objectId, action->targetName, 1);
            if (component) {
                SceneGraph_destroyComponent(sceneGraph, component->id);
            }
        } break;
        default:
            TraceLog(LOG_ERROR, "Unknown action type: %d", action->actionType);
            break;
        }
    }
}

int ActionFromJSON(cJSON* actionCfg, Action* action)
{
    cJSON* type = cJSON_GetObjectItemCaseSensitive(actionCfg, "type");
    if (cJSON_IsString(type)) {
        if (strcmp(type->valuestring, "enable_object") == 0) {
            action->actionType = ACTION_TYPE_ENABLE_OBJECT;
        } else if (strcmp(type->valuestring, "disable_object") == 0) {
            action->actionType = ACTION_TYPE_DISABLE_OBJECT;
        } else if (strcmp(type->valuestring, "destroy_object") == 0) {
            action->actionType = ACTION_TYPE_DESTROY_OBJECT;
        } else if (strcmp(type->valuestring, "enable_component") == 0) {
            action->actionType = ACTION_TYPE_ENABLE_COMPONENT;
        } else if (strcmp(type->valuestring, "disable_component") == 0) {
            action->actionType = ACTION_TYPE_DISABLE_COMPONENT;
        } else if (strcmp(type->valuestring, "destroy_component") == 0) {
            action->actionType = ACTION_TYPE_DESTROY_COMPONENT;
        } else if (strcmp(type->valuestring, "enable_child_object") == 0) {
            action->actionType = ACTION_TYPE_ENABLE_CHILD_OBJECT;
        } else if (strcmp(type->valuestring, "disable_child_object") == 0) {
            action->actionType = ACTION_TYPE_DISABLE_CHILD_OBJECT;
        } else if (strcmp(type->valuestring, "destroy_child_object") == 0) {
            action->actionType = ACTION_TYPE_DESTROY_CHILD_OBJECT;
        } else if (strcmp(type->valuestring, "enable_child_component") == 0) {
            action->actionType = ACTION_TYPE_ENABLE_CHILD_COMPONENT;
        } else if (strcmp(type->valuestring, "disable_child_component") == 0) {
            action->actionType = ACTION_TYPE_DISABLE_CHILD_COMPONENT;
        } else if (strcmp(type->valuestring, "destroy_child_component") == 0) {
            action->actionType = ACTION_TYPE_DESTROY_CHILD_COMPONENT;
        } else {
            TraceLog(LOG_ERROR, "Unknown action type: %s", type->valuestring);
            return 0;
        }
    } else {
        TraceLog(LOG_ERROR, "Action type is not a string");
        return 0;
    }
    cJSON* target = cJSON_GetObjectItemCaseSensitive(actionCfg, "targetName");

    if (cJSON_IsString(target)) {
        action->targetName = strdup(target->valuestring);

    } else {
        TraceLog(LOG_ERROR, "Action targetName is not (found / a string)");
        return 0;
    }

    return 1;
}
