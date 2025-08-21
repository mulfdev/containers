# Makefile for containers.h STB-style header-only library

CC = gcc
CFLAGS = -std=c23 -Wall -Wextra -O2
DEBUG_FLAGS = -g -O0 -DDEBUG -DCONTAINERS_DEBUG=1
SOURCE = main.c
TARGET = main
BUILD_DIR = build

# Default target
all: $(BUILD_DIR)/$(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build main
$(BUILD_DIR)/$(TARGET): $(SOURCE) containers.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(TARGET) $(SOURCE)

# Debug build
debug: $(BUILD_DIR)/$(TARGET)_debug

$(BUILD_DIR)/$(TARGET)_debug: $(SOURCE) containers.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o $(BUILD_DIR)/$(TARGET)_debug $(SOURCE)

# Run
run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET)

# Run debug version
run-debug: $(BUILD_DIR)/$(TARGET)_debug
	./$(BUILD_DIR)/$(TARGET)_debug

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Format code (if clang-format is available)
format:
	clang-format -i $(SOURCE)

# Install header (copy to system include directory)
install: containers.h
	@echo "Installing containers.h to /usr/local/include/"
	sudo cp containers.h /usr/local/include/

# Help
help:
	@echo "Available targets:"
	@echo "  all       - Build main (default)"
	@echo "  debug     - Build with debug flags"
	@echo "  run       - Build and run main"
	@echo "  run-debug - Build and run debug version"
	@echo "  clean     - Remove build directory"
	@echo "  format    - Format source code"
	@echo "  install   - Install header to system"
	@echo "  help      - Show this help"

.PHONY: all debug run run-debug clean format install help
