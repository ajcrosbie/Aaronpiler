# ===================
# Configurable Paths
# ===================
SRC_DIR := src
TEST_DIR := tests
INCLUDE_DIR := include
BUILD_DIR := build

# ===================
# Compiler Settings
# ===================
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -I$(INCLUDE_DIR)
DEBUG_CXXFLAGS := $(CXXFLAGS) -g -O0 -DDEBUG
RELEASE_CXXFLAGS := $(CXXFLAGS) -O2 -DNDEBUG
LDFLAGS := 
LIBS := 

# ===================
# Build Configuration
# ===================
BUILD_TYPE ?= debug
ifeq ($(BUILD_TYPE),release)
    CXXFLAGS := $(RELEASE_CXXFLAGS)
    BUILD_SUFFIX := _release
else
    CXXFLAGS := $(DEBUG_CXXFLAGS)
    BUILD_SUFFIX := _debug
endif

# ===================
# Auto-Discovery
# ===================
SRC_FILES := $(shell find $(SRC_DIR) -name "*.cpp" -type f)
TEST_FILES := $(shell find $(TEST_DIR) -name "*.cpp" -type f)
HEADER_FILES := $(shell find $(INCLUDE_DIR) -name "*.h" -o -name "*.hpp" -type f)

# ===================
# Derived Files
# ===================
OBJ_FILES := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
TEST_OBJ_FILES := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(TEST_FILES))
TEST_EXECUTABLES := $(patsubst $(TEST_DIR)/%.cpp, $(BUILD_DIR)/test_%, $(TEST_FILES))

# ===================
# Targets
# ===================
.PHONY: all clean test test-regex

all: $(OBJ_FILES)

# Create object files for sources
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create object files for tests
$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build test binaries
$(BUILD_DIR)/test_%: $(BUILD_DIR)/$(TEST_DIR)/%.o $(OBJ_FILES)
	@mkdir -p $(dir $@)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LIBS)

# ===================
# Test Rules
# ===================
test: $(TEST_EXECUTABLES)
	@echo "Running all tests..."
	@for test_exec in $(TEST_EXECUTABLES); do \
		echo "Running $$test_exec..."; \
		./$$test_exec || exit 1; \
	done
	@echo "All tests passed."

test-regex: $(BUILD_DIR)/test_regex_tests
	@echo "Running regex tests..."
	./$(BUILD_DIR)/test_regex_tests

# ===================
# Clean
# ===================
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
