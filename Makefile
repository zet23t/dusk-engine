# PLATFORM=
#     > PLATFORM_DESKTOP
#     > PLATFORM_DESKTOP_SDL
#     > PLATFORM_WEB
#     > PLATFORM_DRM
#     > PLATFORM_ANDROID

# Makefile for Windows Mingw-bash

PLATFORM_OS          ?= WINDOWS
PLATFORM             ?= PLATFORM_DESKTOP
BUILD                ?= release
PROJECTDIR           ?= .

# vpath %.c $(SRCDIR) $(PROJECTDIR)src

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Isubmodules/raylib/src -D$(PLATFORM) -I$(PROJECTDIR)/src -Isrc -MMD -MP

# Directories
SRCDIR := src
BUILDDIR := $(PROJECTDIR)/_build/$(BUILD)/$(PLATFORM)
LIBDIR := submodules/raylib/$(BUILD)/$(PLATFORM)

LIBDIR := $(shell echo $(LIBDIR) | tr A-Z a-z)
BUILDDIR := $(shell echo $(BUILDDIR) | tr A-Z a-z)

# Source files
# SRCS := $(shell find $(SRCDIR) -name '*.c')
# SRCS := $(filter-out src/stu.c, $(SRCS))
SRCS := src/stu.c src/dll_win.c
SRCS_DLL := $(PROJECTDIR)/src/stu_dll.c
RAYLIB_LIB := raylibdll

# Output file
TARGET := $(BUILDDIR)/dusk-engine.exe

# Debug and release build options
ifeq ($(BUILD),debug)
	CFLAGS += -g -DDEBUG
else ifeq ($(BUILD),release)
	CFLAGS += -DNDEBUG
else
	$(error Build type $(BUILD) not supported by this Makefile)
endif

OBJDIR := $(BUILDDIR)



# ifeq ($(PLATFORM),PLATFORM_WEB)
# 	# Emscripten required variables
# 	EMSDK_PATH          ?= /c/tools/emsdk
# 	EMSCRIPTEN_VERSION  ?= 1.38.31
# 	CLANG_VERSION       = e$(EMSCRIPTEN_VERSION)_64bit
# 	PYTHON_VERSION      = 2.7.13.1_64bit\python-2.7.13.amd64
# 	NODE_VERSION        = 8.9.1_64bit
# 	# export PATH         = $(EMSDK_PATH);$(EMSDK_PATH)\clang\$(CLANG_VERSION);$(EMSDK_PATH)\node\$(NODE_VERSION)\bin;$(EMSDK_PATH)\python\$(PYTHON_VERSION);$(EMSDK_PATH)\emscripten\$(EMSCRIPTEN_VERSION);C:\raylib\MinGW\bin:$$(PATH)
# 	EMSCRIPTEN          = $(EMSDK_PATH)\emscripten\$(EMSCRIPTEN_VERSION)
# endif


ifeq ($(PLATFORM), PLATFORM_WEB)
    # NOTE: When using multi-threading in the user code, it requires -pthread enabled
    CFLAGS += -std=gnu99
    ifeq ($(PROFILING), TRUE)
        CFLAGS += --profiling-funcs --tracing 
    endif
    ifeq ($(CLEAR_CACHE), TRUE)
        CFLAGS += --clear-cache
    endif
else
    CFLAGS += -std=c99
endif

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        ifeq ($(CC), tcc)
            LDLIBS = -lopengl32 -lgdi32 -lwinmm -lshell32
        else
            LDLIBS = -static-libgcc -lopengl32 -lgdi32 -lwinmm
        endif
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        LDLIBS = -lGL -lc -lm -lpthread -ldl -lrt
        ifeq ($(USE_WAYLAND_DISPLAY),FALSE)
            LDLIBS += -lX11
        endif
        # TODO: On ARM 32bit arch, miniaudio requires atomics library
        #LDLIBS += -latomic
    endif
    ifeq ($(PLATFORM_OS),OSX)
        LDLIBS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
    endif
    ifeq ($(PLATFORM_OS),BSD)
        LDLIBS = -lGL -lpthread
    endif
    ifeq ($(USE_EXTERNAL_GLFW),TRUE)
        # Check the version name. If GLFW3 was built manually, it may have produced
        # a static library known as libglfw3.a. In that case, the name should be -lglfw3
        LDLIBS = -lglfw
    endif
endif
ifeq ($(PLATFORM),PLATFORM_DESKTOP_SDL)
    ifeq ($(PLATFORM_OS),WINDOWS)
        LDLIBS = -static-libgcc -lopengl32 -lgdi32
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        LDLIBS = -lGL -lc -lm -lpthread -ldl -lrt
        ifeq ($(USE_WAYLAND_DISPLAY),FALSE)
            LDLIBS += -lX11
        endif
    endif
    LDLIBS += -lSDL2 -lSDL2main
endif
ifeq ($(PLATFORM),PLATFORM_DRM)
    LDLIBS = -lGLESv2 -lEGL -ldrm -lgbm -lpthread -lrt -lm -ldl
    ifeq ($(RAYLIB_MODULE_AUDIO),TRUE)
        LDLIBS += -latomic
    endif
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    LDLIBS = -llog -landroid -lEGL -lGLESv2 -lOpenSLES -lc -lm
endif

ifeq ($(BUILD),release)
    ifeq ($(PLATFORM),PLATFORM_WEB)
        CFLAGS += -Os --preload-file $(PROJECTDIR)/assets/@/assets --preload-file assets/@/engine/assets -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY
    endif
    ifeq ($(PLATFORM),PLATFORM_DESKTOP)
        CFLAGS += -O1
    endif
    ifeq ($(PLATFORM),PLATFORM_ANDROID)
        CFLAGS += -O2
    endif
endif


$(info Building for $(PLATFORM) on $(PLATFORM_OS) in $(BUILD) mode with project directory $(PROJECTDIR))
ifeq ($(PLATFORM),PLATFORM_WEB)
    # HTML5 emscripten compiler
    CC = emcc
    AR = emar
    TARGET := $(BUILDDIR)/dusk-engine.html
    RAYLIB_LIB = raylib
    SRCS := $(SRCS) $(SRCS_DLL)
    $(info SRCS => $(SRCS))

    OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.c,%.o,$(notdir $(SRCS))))
    SRCS_DLL =
    CFLAGS += -sSTACK_SIZE=5242880
else
    OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
endif

DEPS := $(OBJS:.o=.d)

$(info SRCS = $(SRCS))
$(info OBJS = $(OBJS))

RAYLIB_MAKEFILE := submodules/raylib/src/Makefile

PLATFORMS = PLATFORM_DESKTOP PLATFORM_WEB PLATFORM_RPI
MODES = DEBUG RELEASE

# Default target
all: main

main: $(TARGET)

init-project:
	@if [ ! -d "$(PROJECTDIR)/src" ]; then \
		echo "Initializing project structure in $(PROJECTDIR)/..."; \
		mkdir -p $(PROJECTDIR)/assets; \
		mkdir -p $(PROJECTDIR)/src/game; \
		cp -r src/game $(PROJECTDIR)/src/; \
		cp src/stu_dll.c $(PROJECTDIR)/src/; \
		cp run_game.sh $(PROJECTDIR)/; \
		cp build_web.sh $(PROJECTDIR)/; \
		echo "Project initialized."; \
	fi

dll: $(SRCS_DLL)
ifeq ($(PLATFORM),PLATFORM_WEB)
	@echo "Skipping dll target for PLATFORM_WEB"
else
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -shared -o $(BUILDDIR)/game.dll $^ -L$(LIBDIR) -l$(RAYLIB_LIB) $(LDLIBS)
endif

raylib-all:
	$(MAKE) -C $(dir $(RAYLIB_MAKEFILE)) RAYLIB_LIBTYPE=SHARED PLATFORM=PLATFORM_DESKTOP RAYLIB_BUILD_MODE=DEBUG
	$(MAKE) -C $(dir $(RAYLIB_MAKEFILE)) RAYLIB_LIBTYPE=SHARED PLATFORM=PLATFORM_DESKTOP RAYLIB_BUILD_MODE=RELEASE
	$(MAKE) -C $(dir $(RAYLIB_MAKEFILE)) PLATFORM=PLATFORM_WEB RAYLIB_BUILD_MODE=DEBUG
	$(MAKE) -C $(dir $(RAYLIB_MAKEFILE)) PLATFORM=PLATFORM_WEB RAYLIB_BUILD_MODE=RELEASE
	cp submodules/raylib/release/platform_desktop/raylib.dll .

run: main $(TARGET) init-project
	$(TARGET) $(PROJECTDIR)

test:
	$(CC) $(filter-out -DNDEBUG,$(CFLAGS)) -g -o test_program src/tests.c -L$(LIBDIR) -l$(RAYLIB_LIB) $(LDLIBS)
	./test_program

# Run target
run-node: $(OUTPUT)
	@which http-server >/dev/null || npm install -g http-server
	http-server $(BUILDDIR)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -L$(LIBDIR) -l$(RAYLIB_LIB) $(LDLIBS)

-include $(DEPS)

# Rule to build object files
# $(OBJDIR)/%.o: %.c
# 	mkdir -p $(dir $@)
# 	$(CC) $(CFLAGS) -c -o $@ $<
# Explicit rules for object files
$(OBJDIR)/stu.o: src/stu.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/dll_win.o: src/dll_win.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROJECTDIR)/_build/release/platform_web/stu_dll.o: $(PROJECTDIR)/src/stu_dll.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean rule
clean:
	rm -rf $(PROJECTDIR)/_build $(TARGET)

.PHONY: all clean