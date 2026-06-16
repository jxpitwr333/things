CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
INCLUDES = -I/usr/local/include

# Uncomment to enable profiling
# PROFILE = 1

ifdef PROFILE
    CFLAGS += -g -fno-omit-frame-pointer
endif

# Platform detection
ifeq ($(OS),Windows_NT)
    LIBS = -L/usr/local/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    RM = rmdir /s /q
    MKDIR = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
    TARGET_EXT = .exe
    RUN_CMD = $(TARGET)
else
    LIBS = -L/usr/local/lib -lraylib -lm -lpthread -ldl -lrt -lX11
    RM = rm -rf
    MKDIR = mkdir -p $(BUILD_DIR)
    TARGET_EXT =
    RUN_CMD = ./$(TARGET)
endif

SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/game$(TARGET_EXT)

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	$(MKDIR)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	$(RUN_CMD)

profile: clean all
	perf record -g $(RUN_CMD)
	perf report
	@echo "Profile data saved to perf.data"

.PHONY: all clean run profile