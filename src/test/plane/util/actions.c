#include "../plane_sim_g.h"

void TriggerActions(SceneGraph *sceneGraph, SceneObjectId objectId, Action *actions, int count)
{
    for (int i = 0; i < count; i++)
    {
        Action *action = &actions[i];
        switch (action->actionType)
        {
        case ACTION_TYPE_ENABLE_OBJECT:
        {
            SceneObject *object = SceneGraph_findObjectByName(sceneGraph, action->targetName, 1);
            if (object)
            {
                SceneGraph_setObjectEnabled(sceneGraph, object->id, 1);
            }
        }
        break;
        case ACTION_TYPE_DISABLE_OBJECT:
        {
            SceneObject *object = SceneGraph_findObjectByName(sceneGraph, action->targetName, 1);
            if (object)
            {
                SceneGraph_setObjectEnabled(sceneGraph, object->id, 0);
            }
        }
        break;
        case ACTION_TYPE_DESTROY_OBJECT:
        {
            SceneObject *object = SceneGraph_findObjectByName(sceneGraph, action->targetName, 1);
            if (object)
            {
                SceneGraph_destroyObject(sceneGraph, object->id);
            }
        }
        break;
        case ACTION_TYPE_ENABLE_COMPONENT:
        {
            SceneComponent *component = SceneGraph_findComponentByName(sceneGraph, action->targetName, 1);
            if (component)
            {
                SceneGraph_setComponentEnabled(sceneGraph, component->id, 1);
            }
        }
        break;
        case ACTION_TYPE_DISABLE_COMPONENT:
        {
            SceneComponent *component = SceneGraph_findComponentByName(sceneGraph, action->targetName, 1);
            if (component)
            {
                SceneGraph_setComponentEnabled(sceneGraph, component->id, 0);
            }
        }
        break;
        case ACTION_TYPE_DESTROY_COMPONENT:
        {
            SceneComponent *component = SceneGraph_findComponentByName(sceneGraph, action->targetName, 1);
            if (component)
            {
                SceneGraph_destroyComponent(sceneGraph, component->id);
            }
        }
        break;
        case ACTION_TYPE_ENABLE_CHILD_OBJECT:
        {
            SceneObject *object = SceneGraph_findChildByName(sceneGraph, objectId, action->targetName, 1);
            if (object)
            {
                SceneGraph_setObjectEnabled(sceneGraph, object->id, 1);
            }
        }
        break;
        case ACTION_TYPE_DISABLE_CHILD_OBJECT:
        {
            SceneObject *object = SceneGraph_findChildByName(sceneGraph, objectId, action->targetName, 1);
            if (object)
            {
                SceneGraph_setObjectEnabled(sceneGraph, object->id, 0);
            }
        }
        break;
        case ACTION_TYPE_DESTROY_CHILD_OBJECT:
        {
            SceneObject *object = SceneGraph_findChildByName(sceneGraph, objectId, action->targetName, 1);
            if (object)
            {
                SceneGraph_destroyObject(sceneGraph, object->id);
            }
        }
        break;
        case ACTION_TYPE_ENABLE_CHILD_COMPONENT:
        {
            SceneComponent *component = SceneGraph_findChildComponentByName(sceneGraph, objectId, action->targetName, 1);
            if (component)
            {
                SceneGraph_setComponentEnabled(sceneGraph, component->id, 1);
            }
        }
        break;
        case ACTION_TYPE_DISABLE_CHILD_COMPONENT:
        {
            SceneComponent *component = SceneGraph_findChildComponentByName(sceneGraph, objectId, action->targetName, 1);
            if (component)
            {
                SceneGraph_setComponentEnabled(sceneGraph, component->id, 0);
            }
        }
        break;
        case ACTION_TYPE_DESTROY_CHILD_COMPONENT:
        {
            SceneComponent *component = SceneGraph_findChildComponentByName(sceneGraph, objectId, action->targetName, 1);
            if (component)
            {
                SceneGraph_destroyComponent(sceneGraph, component->id);
            }
        }
        break;
        default:
            TraceLog(LOG_ERROR, "Unknown action type: %d", action->actionType);
            break;
        }
    }
}

