CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Werror -std=c99 -O2
TARGET_NAME = game

SRC_DIR = src
BUILD_DIR = build

ALL_SRCS = $(wildcard $(SRC_DIR)/*.c)

ifeq ($(OS),Windows_NT)
    LIBS = -luser32 -lgdi32 -lm
    RM = del /q /f
    MKDIR = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
    TARGET_EXT = .exe
    RUN_CMD = $(TARGET)
    
    SRCS = $(filter-out $(SRC_DIR)/platform_x11.c, $(ALL_SRCS))
else
    LIBS = -lX11 -lXext -lm
    RM = rm -rf
    MKDIR = mkdir -p $(BUILD_DIR)
    TARGET_EXT = 
    RUN_CMD = $(TARGET)
    
    SRCS = $(filter-out $(SRC_DIR)/platform_win32.c, $(ALL_SRCS))
endif

TARGET = $(BUILD_DIR)/$(TARGET_NAME)$(TARGET_EXT)

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# uncomment to enable profiling
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
