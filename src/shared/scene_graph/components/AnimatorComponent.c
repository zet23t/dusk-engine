#include "AnimationConditions.h"
#include "game/game.h"

#include "rlgl.h"
#include "shared/scene_graph/scene_graph.h"
#include "shared/serialization/reflection.h"
#include "shared/serialization/serializable_structs.h"

#include <string.h>

int AnimatorComponent_getVariableIndex(AnimatorComponent *component, const char *name)
{
    for (int i=0;i<component->variables_count;i++)
    {
        if (strcmp(component->variables[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

void AnimatorComponent_setVariableValue(AnimatorComponent *component, int index, float value)
{
    if (component == NULL || index < 0 || index >= component->variables_count) return;
    component->variables[index].value = value;
}

void AnimatorComponent_getVariableValue(AnimatorComponent *component, Animation *animation, int index, float *value)
{
    *value = 0;
    if (index < 0 || index >= component->variables_count) return;
    const char *name = component->variables[index].name;
    
    for (int i=0;i<animation->variables_count;i++)
    {
        if (strcmp(animation->variables[i].name, name) == 0)
        {
            *value = animation->variables[i].value;
            break;
        }
    }
    *value = component->variables[index].value;
}

void AnimatorComponent_update(SceneObject *sceneObject, SceneComponentId SceneComponent,
                                         float delta, void *componentData)
{
    AnimatorComponent *component = (AnimatorComponent *)componentData;
    if (component->animationId.generation == 0)
    {
        return;
    }

    AnimationManager *animationManager = AnimationManager_getInstance(sceneObject->graph);
    Animation *animation = AnimationManager_getAnimationById(animationManager, component->animationId);
    if (animation == NULL)
    {
        return;
    }

    component->currentTime += delta;
    if (component->currentStateIndex >= animation->states_count)
    {
        TraceLog(LOG_WARNING, "Invalid state index %d", component->currentStateIndex);
        return;
    }
    AnimationState *state = &animation->states[component->currentStateIndex];
    for (int i=0;i<state->transitions_count;i++)
    {
        AnimationStateTransition *transition = &state->transitions[i];
        int transit = 1;
        for (int j=0;j<transition->conditions_count && transit;j++)
        {
            AnimationCondition *condition = &transition->conditions[j];
            float valueA = condition->valueA;
            float valueB = condition->valueB;
            AnimatorComponent_getVariableValue(component, animation, condition->varA, &valueA);
            AnimatorComponent_getVariableValue(component, animation, condition->varB, &valueB);
            switch (condition->operation)
            {
                case ANIMATION_CONDITION_TYPE_EQUALS:
                    transit = valueA == valueB;
                    break;
                case ANIMATION_CONDITION_TYPE_NOT_EQUALS:
                    transit = valueA != valueB;
                    break;
                case ANIMATION_CONDITION_TYPE_GREATER_THAN:
                    transit = valueA > valueB;
                    break;
                case ANIMATION_CONDITION_TYPE_LESS_THAN:
                    transit = valueA < valueB;
                    break;
                case ANIMATION_CONDITION_TYPE_GREATER_THAN_OR_EQUALS:    
                    transit = valueA >= valueB;
                    break;
                case ANIMATION_CONDITION_TYPE_LESS_THAN_OR_EQUALS:
                    transit = valueA <= valueB;
                    break;
            }
        }

        if (transit && transition->target >= 0 && transition->target < animation->states_count)
        {
            // printf("Transitioning from %s to %s\n", state->name, animation->states[transition->target].name);
            component->currentStateIndex = transition->target;
            component->currentTime = 0;
        }
    }
    if (state->clipSequence_count == 0)
    {
        TraceLog(LOG_WARNING, "Invalid clip sequence count %d for state %s", state->clipSequence_count, state->name);
        return;
    }

    AnimationClip *clip = &animation->clips[state->clipSequence[0]];
    float sequenceDuration = 0;
    for (int i = 0; i < state->clipSequence_count; i++)
    {
        AnimationClip *c = &animation->clips[state->clipSequence[i]];
        sequenceDuration += c->duration;
    }
    float time = fmod(component->currentTime, sequenceDuration);
    for (int i = 0; i < state->clipSequence_count; i++)
    {
        clip = &animation->clips[state->clipSequence[i]];
        if (time < clip->duration)
        {
            break;
        }
        time -= clip->duration;
    }
    for (int i = 0; i < clip->tracks_count; i++)
    {
        AnimationTrack *track = &clip->tracks[i];
        float value = 0;
        for (int j = 0; j < track->keys_count; j++)
        {
            AnimationKey *key = &track->keys[j];
            if (key->time > time)
            {
                break;
            }
            value = key->value;
        }
        float *floatValue = NULL;
        size_t size = 0;
        const char *type = NULL;
        int result = SceneGraph_retrieve(sceneObject->graph, sceneObject->id, track->path, (void**)&floatValue, &size, &type);
        if (result == REFLECT_OK)
        {
            *floatValue = value;
        }
    }
}

void AnimatorComponent_onInitialize(SceneObject *sceneObject, SceneComponentId SceneComponent, void *componentData, void *initArg)
{
    AnimatorComponent *component = (AnimatorComponent *)componentData;
    component->animationName = NULL;
    component->currentTime = 0;
    component->currentStateIndex = 0;
    component->animationId = (AnimationId){0};
    component->variables = NULL;
    component->variables_count = 0;
    component->variables_capacity = 0;
    component->loopCount = 0;
    if (initArg == NULL) return;
    AnimatorComponent *init = (AnimatorComponent *)initArg;
    component->animationName = RL_STRDUP(init->animationName);
    component->animationId = init->animationId;
    component->currentStateIndex = init->currentStateIndex;
    component->currentTime = init->currentTime;
    component->loopCount = init->loopCount;
    component->variables = RL_MALLOC(sizeof(AnimationVariable) * init->variables_capacity);
    component->variables_capacity = init->variables_capacity;
    component->variables_count = init->variables_count;
    for (int i=0;i<component->variables_count;i++)
    {
        component->variables[i] = init->variables[i];
        component->variables[i].name = RL_STRDUP(init->variables[i].name);
    }
}

void AnimatorComponent_onDestroy(SceneObject *sceneObject, SceneComponentId SceneComponent, void *componentData)
{
    AnimatorComponent *component = (AnimatorComponent *)componentData;
    for (int i=0;i<component->variables_count;i++)
    {
        RL_FREE(component->variables[i].name);
    }
    RL_FREE(component->variables);
    RL_FREE(component->animationName);
}

BEGIN_COMPONENT_REGISTRATION(AnimatorComponent){
    .updateTick = AnimatorComponent_update,
    .onDestroy = AnimatorComponent_onDestroy,
    .onInitialize = AnimatorComponent_onInitialize,
} END_COMPONENT_REGISTRATION