#include <stdbool.h>

#include "AnimationConditions.h"
#include "game/game.h"
#include <external/cjson.h>
#include <stdlib.h>

static void PrintIndent(int indent) {
    for (int i=0;i<indent;i++) printf("  ");
}

void Print_float(int indent, const char *key, float* obj) {
    PrintIndent(indent);
    printf("%s (float): %f\n", key, *obj);
}

void Print_int(int indent, const char *key, int* obj) {
    PrintIndent(indent);
    printf("%s (int): %d\n", key, *obj);
}

void Print_int8_t(int indent, const char *key, int8_t* obj) {
    PrintIndent(indent);
    printf("%s (int8_t): %d\n", key, *obj);
}

void Print_char(int indent, const char *key, char* obj) {
    PrintIndent(indent);
    printf("%s (char): %c\n", key, *obj);
}

void Print_int16_t(int indent, const char *key, int16_t* obj) {
    PrintIndent(indent);
    printf("%s (int16_t): %d\n", key, *obj);
}

void Print_int32_t(int indent, const char *key, int32_t* obj) {
    PrintIndent(indent);
    printf("%s (int32_t): %d\n", key, *obj);
}

void Print_int64_t(int indent, const char *key, int64_t* obj) {
    PrintIndent(indent);
    printf("%s (int64_t): %lld\n", key, *obj);
}

void Print_uint8_t(int indent, const char *key, uint8_t* obj) {
    PrintIndent(indent);
    printf("%s (uint8_t): %u\n", key, *obj);
}

void Print_uint16_t(int indent, const char *key, uint16_t* obj) {
    PrintIndent(indent);
    printf("%s (uint16_t): %u\n", key, *obj);
}

void Print_uint32_t(int indent, const char *key, uint32_t* obj) {
    PrintIndent(indent);
    printf("%s (uint32_t): %u\n", key, *obj);
}

void Print_uint64_t(int indent, const char *key, uint64_t* obj) {
    PrintIndent(indent);
    printf("%s (uint64_t): %llu\n", key, *obj);
}

void Print_bool(int indent, const char *key, bool* obj) {
    PrintIndent(indent);
    printf("%s (bool): %s\n", key, *obj ? "true" : "false");
}

void Print_Vector2(int indent, const char *key, Vector2* obj) {
    PrintIndent(indent);
    printf("%s (Vector2): { x: %f, y: %f }\n", key, obj->x, obj->y);
}

void Print_Vector3(int indent, const char *key, Vector3* obj) {
    PrintIndent(indent);
    printf("%s (Vector3): { x: %f, y: %f, z: %f }\n", key, obj->x, obj->y, obj->z);
}

void Print_Vector4(int indent, const char *key, Vector4* obj) {
    PrintIndent(indent);
    printf("%s (Vector4): { x: %f, y: %f, z: %f, w: %f }\n", key, obj->x, obj->y, obj->z, obj->w);
}

void Print_Rectangle(int indent, const char *key, Rectangle* obj) {
    PrintIndent(indent);
    printf("%s (Rectangle): { x: %f, y: %f, width: %f, height: %f }\n", key, obj->x, obj->y, obj->width, obj->height);
}

void Print_Color(int indent, const char *key, Color* obj) {
    PrintIndent(indent);
    printf("%s (Color): { r: %d, g: %d, b: %d, a: %d }\n", key, obj->r, obj->g, obj->b, obj->a);
}

void Print_size_t(int indent, const char *key, size_t* obj) {
    PrintIndent(indent);
    printf("%s (size_t): %llu\n", key, *obj);
}

#include "shared/serialization/undef_serialization_macros.h"

#define SERIALIZABLE_STRUCT_START(name) void Print_##name(int indent, const char *key, name* obj) {\
    PrintIndent(indent++);\
    printf("%s: {\n", key);
    #define SERIALIZABLE_FIELD(type, name) Print_##type(indent, #name, &obj->name);
    #define SERIALIZABLE_FIXED_ARRAY(type, name, count) \
        PrintIndent(indent); printf(#name "[%d](" #type "):\n", count); \
        for (int i=0;i<count;i++) Print_##type(indent, #name, &obj->name[i]);
    #define SERIALIZABLE_CSTR(name) PrintIndent(indent); printf(#name " (cstr): %s\n", obj->name);
    #define SERIALIZABLE_STRUCT_LIST_ELEMENT(type, name) \
        PrintIndent(indent); printf(#name "[%d](" #type "): [\n", obj->name##_count); \
        for (int i=0;i<obj->name##_count;i++) {\
            char name[256];\
            sprintf(name, "%s[%d]", #name, i);\
            Print_##type(indent + 1, name, &obj->name[i]);\
        } \
        PrintIndent(indent); printf("]\n");
#define SERIALIZABLE_STRUCT_END(name) PrintIndent(indent); printf("}\n"); }
#include "shared/serialization/define_serialization_macros.h"
#include "shared/serialization/serializable_file_headers.h"



AnimationManager *AnimationManager_getInstance(SceneGraph *sceneGraph)
{
    AnimationManager *animationManager;
    SceneGraph_getComponentByType(sceneGraph, (SceneObjectId){0}, _componentIdMap.AnimationManagerId, (void **)&animationManager, 0);
    if (animationManager == NULL)
    {
        SceneObjectId id = SceneGraph_createObject(sceneGraph, "AnimationManager");
        SceneComponentId componentId = SceneGraph_addComponent(sceneGraph, id, _componentIdMap.AnimationManagerId, &(AnimationManager){0});
        if (SceneGraph_getComponent(sceneGraph, componentId, (void **)&animationManager) == NULL)
        {
            TraceLog(LOG_ERROR, "Failed to get AnimationManager component");
            return NULL;
        }
    }
    return animationManager;
}

static void AnimationTrack_cleanup(AnimationTrack *track)
{
    if (track->path)
    {
        RL_FREE(track->path);
        if (track->keys != NULL)
        {
            RL_FREE(track->keys);
        }
        track->keys_capacity = track->keys_count = 0;
    }
}

static void AnimationClip_cleanup(AnimationClip *clip)
{
    if (clip->name)
    {
        RL_FREE(clip->name);
    }

    if (clip->tracks != NULL)
    {
        for (int i = 0; i < clip->tracks_count; i++)
        {
            AnimationTrack_cleanup(&clip->tracks[i]);
        }
        clip->tracks_capacity = clip->tracks_count = 0;
        if (clip->tracks)
            RL_FREE(clip->tracks);
        clip->tracks = NULL;
    }
}

static void AnimationState_cleanup(AnimationState *state)
{
    if (state->name)
    {
        RL_FREE(state->name);
    }

    if (state->clipSequence != NULL)
    {
        if (state->clipSequence)
            RL_FREE(state->clipSequence);
        state->clipSequence = NULL;
        state->clipSequence_capacity = state->clipSequence_count = 0;
    }

    if (state->transitions != NULL)
    {
        state->transitions_capacity = state->transitions_count = 0;
        if (state->transitions)
            RL_FREE(state->transitions);
        state->transitions = NULL;
    }
}

static void AnimationVariable_cleanup(AnimationVariable *variable)
{
    if (variable->name)
    {
        RL_FREE(variable->name);
    }
}

static void Animation_cleanup(Animation *animation)
{
    if (animation->clips_count > 0)
    {
        for (int i = 0; i < animation->clips_count; i++)
        {
            AnimationClip_cleanup(&animation->clips[i]);
        }
        animation->clips_capacity = animation->clips_count = 0;
        if (animation->clips)
            RL_FREE(animation->clips);
        animation->clips = NULL;
    }

    if (animation->states_count > 0)
    {
        for (int i = 0; i < animation->states_count; i++)
        {
            AnimationState_cleanup(&animation->states[i]);
        }
        animation->states_capacity = animation->states_count = 0;
        if (animation->states)
            RL_FREE(animation->states);
        animation->states = NULL;
    }

    if (animation->variables_count > 0)
    {
        for (int i = 0; i < animation->variables_count; i++)
        {
            AnimationVariable_cleanup(&animation->variables[i]);
        }
        animation->variables_capacity = animation->variables_count = 0;
        if (animation->variables)
            RL_FREE(animation->variables);
        animation->variables = NULL;
    }
}


#define GET_ARRAY(obj, name)                                                                     \
    cJSON *name = cJSON_GetObjectItem(obj, #name);                                               \
    if (!cJSON_IsArray(name))                                                                    \
    {                                                                                            \
        TraceLog(LOG_ERROR, "Failed to load Animation: %s (%d)", animation->filename, __LINE__); \
        goto error;                                                                            \
    }

#define GET_OBJECT(obj, name)                                                                    \
    cJSON *name = cJSON_GetObjectItem(obj, #name);                                               \
    if (!cJSON_IsObject(name))                                                                   \
    {                                                                                            \
        TraceLog(LOG_ERROR, "Failed to load Animation: %s (%d)", animation->filename, __LINE__); \
        goto error;                                                                            \
    }

#define GET_STRING(obj, name)                                                                    \
    cJSON *strElement##name = cJSON_GetObjectItem(obj, #name);                                   \
    if (!cJSON_IsString(strElement##name))                                                       \
    {                                                                                            \
        TraceLog(LOG_ERROR, "Failed to load Animation: %s (%d)", animation->filename, __LINE__); \
        goto error;                                                                            \
    }                                                                                            \
    const char *name = strElement##name->valuestring;

#define GET_FLOAT(obj, name)                                                                     \
    cJSON *floatElement##name = cJSON_GetObjectItem(obj, #name);                                 \
    if (!cJSON_IsNumber(floatElement##name))                                                     \
    {                                                                                            \
        TraceLog(LOG_ERROR, "Failed to load Animation: %s (%d)", animation->filename, __LINE__); \
        goto error;                                                                            \
    }                                                                                            \
    float name = (float)floatElement##name->valuedouble;

static int Animation_loadVariables(Animation *animation, cJSON *variables)
{
    int variableCount = cJSON_GetArraySize(variables);
    if (variableCount > animation->variables_capacity)
    {
        animation->variables = (AnimationVariable *)RL_REALLOC(animation->variables, sizeof(AnimationVariable) * variableCount);
        for (int j=animation->variables_count;j<variableCount;j++)
        {
            animation->variables[j] = (AnimationVariable){
                .name = NULL,
                .value = 0,
            };
        }
        animation->variables_capacity = variableCount;
    }

    animation->variables_count = 0;

    for (int i = 0; i < variableCount; i++)
    {
        cJSON *variable = cJSON_GetArrayItem(variables, i);
        GET_FLOAT(variable, value);
        animation->variables[i] = (AnimationVariable){
            .name = RL_STRDUP(variable->string),
            .value = value,
        };
        animation->variables_count++;
    }

    return 0;

    error: 
    return 1;
}

static int Animation_loadClips(Animation* animation, cJSON* clips)
{
    
    int clipCount = cJSON_GetArraySize(clips);
    if (clipCount > animation->clips_capacity)
    {
        animation->clips_capacity = clipCount;
        animation->clips = (AnimationClip *)RL_REALLOC(animation->clips, sizeof(AnimationClip) * animation->clips_capacity);
    }

    for (int i = 0; i < clipCount; i++)
    {
        cJSON *clip = cJSON_GetArrayItem(clips, i);
        GET_OBJECT(clip, tracks);
        GET_STRING(clip, name);
        GET_FLOAT(clip, duration);
        int trackCount = cJSON_GetArraySize(tracks);
        AnimationClip animationClip = {
            .name = RL_STRDUP(name),
            .duration = duration,
            .tracks = STRUCT_MALLOC(AnimationTrack, trackCount),
            .tracks_count = trackCount,
        };
        animation->clips[i] = animationClip;

        for (int j = 0; j < trackCount; j++)
        {
            cJSON *track = cJSON_GetArrayItem(tracks, j);
            if (!cJSON_IsObject(track))
            {
                TraceLog(LOG_ERROR, "Failed to load Animation: %s (%d)", animation->filename, __LINE__);
                goto error;
            }
            const char *trackPath = track->string;
            GET_ARRAY(track, time);
            GET_ARRAY(track, value);
            int keyCount = cJSON_GetArraySize(time);
            int valueCount = cJSON_GetArraySize(value);
            int count = keyCount < valueCount ? keyCount : valueCount;
            AnimationTrack animationTrack = {
                .path = RL_STRDUP(trackPath),
                .keys = STRUCT_MALLOC(AnimationKey, count),
                .keys_count = count,
                .keys_capacity = count,
            };
            animationClip.tracks[j] = animationTrack;
            for (int k = 0; k < count; k++)
            {
                cJSON *timeElement = cJSON_GetArrayItem(time, k);
                cJSON *valueElement = cJSON_GetArrayItem(value, k);
                if (!cJSON_IsNumber(timeElement) || !cJSON_IsNumber(valueElement))
                {
                    TraceLog(LOG_ERROR, "Failed to load Animation: %s (%d)", animation->filename, __LINE__);
                    goto error;
                }
                animationTrack.keys[k] = (AnimationKey){
                    .time = (float)timeElement->valuedouble,
                    .value = (float)valueElement->valuedouble,
                };
            }
        }
    }

    animation->clips_count = clipCount;

    return 0;
    error:
    return 1;
}

static const char* cJSON_TypeName(int type)
{
    switch (type)
    {
    case cJSON_False:
        return "False";
    case cJSON_True:
        return "True";
    case cJSON_NULL:
        return "NULL";
    case cJSON_Number:
        return "Number";
    case cJSON_String:
        return "String";
    case cJSON_Array:
        return "Array";
    case cJSON_Object:
        return "Object";
    case cJSON_Raw:
        return "Raw";
    default:
        return "Unknown";
    }
}

static int Animation_parseCondition(Animation *animation, float *varValue, int8_t *varIndex, cJSON* var)
{
    if (cJSON_IsNumber(var))
    {
        *varValue = (float)var->valuedouble;
        *varIndex = -1;
        return 0;
    }
    else if (cJSON_IsString(var))
    {
        for (int l = 0; l < animation->variables_count; l++)
        {
            if (strcmp(animation->variables[l].name, var->valuestring) == 0)
            {
                *varIndex = (int8_t)l;
                return 0;
            }
        }
        TraceLog(LOG_ERROR, "Error in %s: Unknown variable %s (%d)", animation->filename, var->valuestring, __LINE__);
        return 1;
    }

    TraceLog(LOG_ERROR, "Error in %s: Invalid type for .%s: %s (%d)", animation->filename, var->string, cJSON_TypeName(var->type), __LINE__);
    return 1;
}

static int Animation_loadStates(Animation *animation, cJSON *states)
{
    int stateCount = cJSON_GetArraySize(states);
    if (stateCount > animation->states_capacity)
    {
        animation->states_capacity = stateCount;
        animation->states = (AnimationState *)RL_REALLOC(animation->states, sizeof(AnimationState) * animation->states_capacity);
    }

    for (int i = 0; i < stateCount; i++)
    {
        cJSON *state = cJSON_GetArrayItem(states, i);
        GET_STRING(state, name);
        GET_ARRAY(state, sequence);
        GET_ARRAY(state, transitions);
        int clipSequenceCount = cJSON_GetArraySize(sequence);
        int transitionsCount = cJSON_GetArraySize(transitions);
        AnimationState animationState = {
            .name = RL_STRDUP(name),
            .clipSequence = STRUCT_MALLOC(uint16_t, clipSequenceCount),
            .clipSequence_capacity = clipSequenceCount,
            .transitions = STRUCT_MALLOC(AnimationStateTransition, transitionsCount),
            .transitions_capacity = transitionsCount,
        };
        animation->states[i] = animationState;

        // animation->states[i].transitions_count = transitionsCount;

        for (int j = 0; j < clipSequenceCount; j++)
        {
            cJSON *clipElement = cJSON_GetArrayItem(sequence, j);
            if (!cJSON_IsString(clipElement))
            {
                TraceLog(LOG_ERROR, "Failed to load Animation: %s (%d)", animation->filename, __LINE__);
                goto error;
            }
            const char *clipName = clipElement->valuestring;
            int clipIndex = -1;
            for (int k = 0; k < animation->clips_count; k++)
            {
                if (strcmp(animation->clips[k].name, clipName) == 0)
                {
                    clipIndex = k;
                    break;
                }
            }
            if (clipIndex == -1)
            {
                TraceLog(LOG_ERROR, "Unknown clipname %s to load Animation: %s (%d)", clipName, animation->filename, __LINE__);
                goto error;
            }
            animation->states[i].clipSequence[j] = clipIndex;
            animation->states[i].clipSequence_count = j + 1;
        }

        for (int j = 0; j < transitionsCount; j++)
        {
            cJSON *transition = cJSON_GetArrayItem(transitions, j);
            GET_STRING(transition, target);
            GET_ARRAY(transition, conditions);
            int conditionsCount = cJSON_GetArraySize(conditions);
            AnimationStateTransition animationTransition = {
                .target = 0,
                .conditions = STRUCT_MALLOC(AnimationCondition, conditionsCount),
                .conditions_capacity = conditionsCount,
                .conditions_count = conditionsCount,
            };

            for (int k = 0; k < stateCount; k++)
            {
                cJSON *stateElement = cJSON_GetArrayItem(states, k);
                GET_STRING(stateElement, name);
                if (strcmp(name, target) == 0)
                {
                    animationTransition.target = k;
                    break;
                }
            }
            
            for (int k = 0; k < conditionsCount; k++)
            {
                cJSON *conditionElement = cJSON_GetArrayItem(conditions, k);
                GET_STRING(conditionElement, condition);
                cJSON *varAElement = cJSON_GetObjectItem(conditionElement, "a");
                cJSON *varBElement = cJSON_GetObjectItem(conditionElement, "b");
                animationTransition.conditions[k].varA = -1;
                animationTransition.conditions[k].varB = -1;
                if (Animation_parseCondition(animation, &animationTransition.conditions[k].valueA, &animationTransition.conditions[k].varA, varAElement) != 0)
                {
                    goto error;
                }
                if (Animation_parseCondition(animation, &animationTransition.conditions[k].valueB, &animationTransition.conditions[k].varB, varBElement) != 0)
                {
                    goto error;
                }
                if (strcmp(condition, "==") == 0)
                {
                    animationTransition.conditions[k].operation = ANIMATION_CONDITION_TYPE_EQUALS;
                }
                else if (strcmp(condition, "!=") == 0)
                {
                    animationTransition.conditions[k].operation = ANIMATION_CONDITION_TYPE_NOT_EQUALS;
                }
                else if (strcmp(condition, ">") == 0)
                {
                    animationTransition.conditions[k].operation = ANIMATION_CONDITION_TYPE_GREATER_THAN;
                }
                else if (strcmp(condition, "<") == 0)
                {
                    animationTransition.conditions[k].operation = ANIMATION_CONDITION_TYPE_LESS_THAN;
                }
                else if (strcmp(condition, ">=") == 0)
                {
                    animationTransition.conditions[k].operation = ANIMATION_CONDITION_TYPE_GREATER_THAN_OR_EQUALS;
                }
                else if (strcmp(condition, "<=") == 0)
                {
                    animationTransition.conditions[k].operation = ANIMATION_CONDITION_TYPE_LESS_THAN_OR_EQUALS;
                }
                else
                {
                    TraceLog(LOG_ERROR, "Error in %s: Invalid operation %s (%d)", animation->filename, condition, __LINE__);
                    goto error;
                }
            }
            
            animationTransition.conditions_count = conditionsCount;
            animation->states[i].transitions[j] = animationTransition;
            animation->states[i].transitions_count = j + 1;
        }
    }

    animation->states_count = stateCount;

    return 0;
    error:
    return 1;
}

void Animation_load(Animation *animation)
{
    Animation_cleanup(animation);
    printf("Loading Spritesheet Animation: %s\n", animation->filename);

    char *content = ResourceManager_loadText(_resourceManager, animation->filename);
    if (content == NULL)
    {
        TraceLog(LOG_ERROR, "Failed to load Spritesheet Animation: %s", animation->filename);
        return;
    }

    cJSON *root = cJSON_Parse(content);
    if (root == NULL)
    {
        TraceLog(LOG_ERROR, "Failed to load Animation: %s (%d)", animation->filename, __LINE__);
        goto error;
    }

    GET_ARRAY(root, clips);
    GET_ARRAY(root, states);
    GET_OBJECT(root, variables);

    if (Animation_loadVariables(animation, variables) != 0)
    {
        goto error;
    }

    if (Animation_loadClips(animation, clips) != 0)
    {
        goto error;
    }

    if (Animation_loadStates(animation, states) != 0)
    {
        goto error;
    }

    printf("Loaded Spritesheet Animation: %s\n", animation->filename);

    goto cleanup;

error:
    Animation_cleanup(animation);
cleanup:
    cJSON_Delete(root);
}

#undef GET_ARRAY
#undef GET_STRING
#undef GET_FLOAT

STRUCT_LIST_ACQUIRE_FN(AnimationManager, Animation, animations)

AnimationId AnimationManager_getAnimation(AnimationManager *manager, char *filename)
{
    for (int i = 0; i < manager->animations_count; i++)
    {
        if (strcmp(manager->animations[i].filename, filename) == 0)
        {
            return (AnimationId){i, manager->animations[i].generation};
        }
    }
    Animation *animation = AnimationManager_acquire_animations(manager);
    animation->clips_count = 0;
    animation->clips = NULL;
    animation->clips_capacity = 0;
    animation->lastModifiedTime = ResourceManager_getModHash(_resourceManager, filename);
    animation->filename = filename;
    animation->generation = ++manager->generationCounter;
    Animation_load(animation);
    return (AnimationId){manager->animations_count - 1, animation->generation};
}

Animation* AnimationManager_getAnimationById(AnimationManager *manager, AnimationId id)
{
    if (id.generation == 0)
    {
        return NULL;
    }
    if (id.animationIndex >= manager->animations_count)
    {
        return NULL;
    }
    Animation *animation = &manager->animations[id.animationIndex];
    if (animation->generation != id.generation)
    {
        return NULL;
    }
    return animation;
}

static void AnimationManager_update(SceneObject *sceneObject, SceneComponentId SceneComponent,
                                    float delta, void *componentData)
{
    AnimationManager *component = (AnimationManager *)componentData;
    for (int i = 0; i < component->animations_count; i++)
    {
        Animation *animation = &component->animations[i];
        
        int modTime = ResourceManager_getModHash(_resourceManager, animation->filename);
        if (modTime != animation->lastModifiedTime)
        {
            animation->lastModifiedTime = modTime;
            Animation_load(animation);
        }
    }
}

BEGIN_COMPONENT_REGISTRATION(AnimationManager){
    .updateTick = AnimationManager_update,
} END_COMPONENT_REGISTRATION