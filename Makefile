# Makefile for Windows Mingw-bash

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Werror -Isubmodules/raylib/src -DPLATFORM_DESKTOP

# Directories
SRCDIR := src
BUILDDIR := _build
LIBDIR := submodules/raylib/src

# Source files
SRCS := $(shell find $(SRCDIR) -name '*.c')

# Output file
TARGET := dusk-engine.exe

# Debug and release build options
ifeq ($(BUILD),debug)
	CFLAGS += -g -DDEBUG
	OBJDIR := $(BUILDDIR)/debug
else
	CFLAGS += -O2
	OBJDIR := $(BUILDDIR)/release
endif

OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -L$(LIBDIR) -lraylib -lopengl32 -lgdi32 -lwinmm

# Rule to build object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean rule
clean:
	rm -rf $(BUILDDIR) $(TARGET)

.PHONY: all clean