CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Werror -std=c99 -O2
TARGET_NAME = game

# --- Configuration Paths (Can be overridden by user) ---
# Default search paths if not specified on command line or environment
RAYLIB_PATH ?= /usr/local

# Platform detection
ifeq ($(OS),Windows_NT)
    # Default Raylib path for w64devkit installer on Windows
    RAYLIB_PATH = C:/raylib/raylib/src
    
    # Include paths
    INCLUDES = -Isrc -I$(RAYLIB_PATH)
    
    # If using w64devkit, libs might be right in raylib path or system paths
    LIBS = -L$(RAYLIB_PATH) -lraylib -lopengl32 -lgdi32 -lwinmm
    
    RM = del /q /f
    MKDIR = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
    TARGET_EXT = .exe
    RUN_CMD = $(TARGET)
else
    INCLUDES = -Isrc -I$(RAYLIB_PATH)/include
    LIBS = -L$(RAYLIB_PATH)/lib -lraylib -lm -lpthread -ldl -lrt -lX11
    RM = rm -rf
    MKDIR = mkdir -p $(BUILD_DIR)
    TARGET_EXT =
    RUN_CMD = LD_LIBRARY_PATH=$(RAYLIB_PATH)/lib ./$(TARGET)
endif

# --- Directories and Targets ---
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/$(TARGET_NAME)$(TARGET_EXT)

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Uncomment to enable profiling
# PROFILE = 1

ifdef PROFILE
    CFLAGS += -g -fno-omit-frame-pointer
endif

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	$(MKDIR)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

run: $(TARGET)
	$(RUN_CMD)

profile: clean all
	perf record -g $(RUN_CMD)
	perf report
	@echo "Profile data saved to perf.data"

.PHONY: all clean run profile
