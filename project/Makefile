# Name for our executable
TARGET_EXECUTABLE := xmod

# Output build files
BUILD_DIRECTORY := ./build

# Source and include directories
SOURCE_DIRECTORIES := ./src
INCLUDE_DIRECTORIES := ./include

# Compiler
CC := gcc

# Find all C source files (*.c) in our source directories
SOURCE_FILES := $(shell find $(SOURCE_DIRECTORIES) -name *.c)

# Name object files after source files (append .o)
OBJECT_FILES := $(SOURCE_FILES:%=$(BUILD_DIRECTORY)/%.o)

# Name dependencies after object files (append .d)
DEPENDENCIES := $(OBJECT_FILES:.o=.d)

# Include directories
INCLUDE_FLAGS := $(addprefix -I, $(INCLUDE_DIRECTORIES))

# Preprocessor flags
CPPFLAGS := $(INCLUDE_FLAGS) -MMD -MP

# Compiler flags
CFLAGS := -Wall -Werror

# Linker flags
# LDFLAGS := 

.DEFAULT_TARGET: all
all: $(BUILD_DIRECTORY)/$(TARGET_EXECUTABLE)

$(BUILD_DIRECTORY)/%.c.o: %.c
	@echo "Building: " $< 
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TARGET_EXECUTABLE): $(OBJECT_FILES)
	@echo "Linking: " $@
	@$(CC) $(OBJECT_FILES) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIRECTORY)
-include $(DEPENDENCIES)
