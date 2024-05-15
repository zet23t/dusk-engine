#ifndef __RUNTIME_INFO_H__
#define __RUNTIME_INFO_H__

#include "shared/resource_manager.h"
#include "shared/editor/editor.h"

typedef struct RuntimeContext {
    const char* projectPath;
    ResourceManager resourceManager;
    EditorState editorState;
    void* gameData;
} RuntimeContext;

#endif