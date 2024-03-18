#include "touch_util.h"
#include <stdlib.h>
#include "raylib.h"

#define MAX_TOUCHES 10
typedef struct TouchInfo {
    int touchId;
    int touchIndex;
    int startFrame;
    int activeFrame;
} TouchInfo;
static TouchInfo _touchInfos[MAX_TOUCHES];
static int _touchFrame;

static TouchInfo* GetTouchInfo(int touchId, int allocate)
{
    for (int i=0;i<MAX_TOUCHES;i+=1)
    {
        if (_touchInfos[i].touchId == touchId)
        {
            return &_touchInfos[i];
        }
    }

    if (!allocate)
    {
        return NULL;
    }

    for (int i=0;i<MAX_TOUCHES;i+=1)
    {
        if (_touchInfos[i].activeFrame == 0)
        {
            _touchInfos[i].touchId = touchId;
            _touchInfos[i].activeFrame = _touchFrame;
            _touchInfos[i].startFrame = _touchFrame;
            return &_touchInfos[i];
        }
    }

    return NULL;
}

void ProcessTouches()
{
    _touchFrame++;
    int touchCount = GetTouchPointCount();
    for (int i=0;i<touchCount;i+=1)
    {
        int touchId = GetTouchPointId(i);
        TouchInfo* touchInfo = GetTouchInfo(touchId, 1);
        if (touchInfo != NULL)
        {
            touchInfo->activeFrame = _touchFrame;
            touchInfo->touchIndex = i;
        }
    }
}

Vector2 GetTouchPositionById(int touchId)
{
    int touchCount = GetTouchPointCount();
    for (int i=0;i<touchCount;i+=1)
    {
        int id = GetTouchPointId(i);
        if (id == touchId)
        {
            return GetTouchPosition(i);
        }
    }
    return (Vector2){0};
}

int GetTouchPhase(int touchId)
{
    TouchInfo* touchInfo = GetTouchInfo(touchId, 0);
    if (touchInfo == NULL)
    {
        return TOUCH_PHASE_NONE;
    }
    if (touchInfo->startFrame == _touchFrame)
    {
        return TOUCH_PHASE_BEGAN;
    }
    if (touchInfo->activeFrame == _touchFrame)
    {
        return TOUCH_PHASE_ACTIVE;
    }

    return TOUCH_PHASE_ENDED;
}