#ifdef COMPONENT

COMPONENT(AnimatorComponent)

#elif defined(SERIALIZABLE_STRUCT_START)

SERIALIZABLE_STRUCT_START(AnimatorVariable)
    SERIALIZABLE_CSTR(name)
    SERIALIZABLE_FIELD(float, value)
SERIALIZABLE_STRUCT_END(AnimatorVariable)

SERIALIZABLE_STRUCT_START(AnimatorComponent)
    SERIALIZABLE_CSTR(animationName)
    SERIALIZABLE_FIELD(AnimationId, animationId)
    SERIALIZABLE_FIELD(int, loopCount)
    SERIALIZABLE_FIELD(int, currentStateIndex)
    SERIALIZABLE_FIELD(float, currentTime)
    SERIALIZABLE_STRUCT_LIST_ELEMENT(AnimatorVariable, variables)
SERIALIZABLE_STRUCT_END(AnimatorComponent)

#elseif defined(COMPONENT_IMPLEMENTATION)

#include "AnimatorComponent.c"

#else

#ifndef _ANIMATOR_COMPONENT_H_
#define _ANIMATOR_COMPONENT_H_

#include "game/game.h"

int AnimatorComponent_getVariableIndex(AnimatorComponent *component, const char *name);
void AnimatorComponent_setVariableValue(AnimatorComponent *component, int index, float value);

#endif
#endif