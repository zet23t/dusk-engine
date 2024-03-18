#ifndef __TOUCH_UTIL_H__
#define __TOUCH_UTIL_H__

#define TOUCH_PHASE_NONE 0
#define TOUCH_PHASE_BEGAN 1
#define TOUCH_PHASE_ACTIVE 2
#define TOUCH_PHASE_ENDED 3

int GetTouchPhase(int touchId);
Vector2 GetTouchPositionById(int touchId);
void ProcessTouches();

#endif