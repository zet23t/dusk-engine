#ifndef __RUNTIME_INFO_H__
#define __RUNTIME_INFO_H__

#ifndef RL_STRDUP
#define RL_STRDUP(s) ((s) ? strdup(s) : NULL)
#endif

#include "shared/resource_manager.h"
#include "shared/editor/editor.h"

typedef struct RuntimeContext {
    const char* projectPath;
    ResourceManager resourceManager;
    EditorState editorState;
    void* gameData;
    int runInBackground;
} RuntimeContext;

#endif