CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Werror -std=c99 #-O2
TARGET_NAME = game

ifeq ($(OS),Windows_NT)

    LIBS = -luser32 -lgdi32 -lm
    
    RM = del /q /f
    MKDIR = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
    TARGET_EXT = .exe
    RUN_CMD = $(TARGET)
else
    # INCLUDES = -Isrc -I$(RAYLIB_PATH)/include
    # LIBS = -L$(RAYLIB_PATH)/lib -lraylib -lm -lpthread -ldl -lrt -lX11
    # RM = rm -rf
    # MKDIR = mkdir -p $(BUILD_DIR)
    # TARGET_EXT =
    # RUN_CMD = LD_LIBRARY_PATH=$(RAYLIB_PATH)/lib ./$(TARGET)
endif

SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/$(TARGET_NAME)$(TARGET_EXT)

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Uncomment to enable profiling
PROFILE = 1

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
