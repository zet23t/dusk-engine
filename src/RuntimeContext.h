#ifndef __RUNTIME_INFO_H__
#define __RUNTIME_INFO_H__

#include "shared/resource_manager.h"

typedef struct RuntimeContext {
    const char* projectPath;
    ResourceManager resourceManager;
    void* gameData;
} RuntimeContext;

#endif