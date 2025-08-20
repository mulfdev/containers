CC = gcc
CFLAGS_DEBUG = -std=c23 -Wall -Wextra -g -O0 -DDEBUG
CFLAGS_PROD = -std=c23 -Wall -Wextra -O3 -DNDEBUG
TARGET = main
SOURCES = main.c containers.c
BUILD_DIR = build

all: debug

debug: $(BUILD_DIR)/$(TARGET)_debug

prod: $(BUILD_DIR)/$(TARGET)_prod

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET)_debug: $(SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $(BUILD_DIR)/$(TARGET)_debug $(SOURCES)

$(BUILD_DIR)/$(TARGET)_prod: $(SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS_PROD) -o $(BUILD_DIR)/$(TARGET)_prod $(SOURCES)

run: $(BUILD_DIR)/$(TARGET)_debug
	./$(BUILD_DIR)/$(TARGET)_debug

run-prod: $(BUILD_DIR)/$(TARGET)_prod
	./$(BUILD_DIR)/$(TARGET)_prod

format:
	clang-format -i *.c *.h

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all debug prod run run-prod format clean
