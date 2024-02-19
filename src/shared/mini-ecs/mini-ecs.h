#ifndef __MINI_ECS_H__
#define __MINI_ECS_H__

typedef struct ECS_Id {
    int id;
} ECS_Id;

typedef struct ECS_ComponentInfo {
    ECS_Id id;
} ECS_ComponentInfo;

typedef struct ECS_System {
    void (*update)(void);
} ECS_System;

typedef struct ECS_Arena {
    int size;
    int capacity;
    void* data;
} ECS_Arena;

#endif