#ifndef __RUNTIME_INFO_H__
#define __RUNTIME_INFO_H__

#include "mem-util.h"

#include "shared/resource_manager.h"
#include "shared/editor/editor.h"


typedef struct RuntimeContext {
    const char* projectPath;
    ResourceManager resourceManager;
    EditorState editorState;
    void* gameData;
    int runInBackground;
} RuntimeContext;

#if PLATFORM_WEB
#include <emscripten/trace.h>
#define TRACE_ENTER_CONTEXT(name) emscripten_trace_enter_context(name)
#define TRACE_EXIT_CONTEXT() emscripten_trace_exit_context()
#else
#define TRACE_ENTER_CONTEXT(name)
#define TRACE_EXIT_CONTEXT()
#endif

#endif