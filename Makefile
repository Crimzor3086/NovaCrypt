CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
INCLUDES = -I./src
LDFLAGS = -pthread

SRC_DIR = src
TEST_DIR = src/tests
OBJ_DIR = obj

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/data/*.cpp) \
            $(wildcard $(SRC_DIR)/indicators/*.cpp) \
            $(wildcard $(SRC_DIR)/sentiment/*.cpp)

# Test files
TEST_FILES = $(TEST_DIR)/DataQualityTest.cpp

# Object files
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJ = $(TEST_FILES:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Main target
all: DataQualityTest

# Create object directories
$(shell mkdir -p $(OBJ_DIR)/data $(OBJ_DIR)/indicators $(OBJ_DIR)/sentiment $(OBJ_DIR)/tests)

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile test files
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link test executable
DataQualityTest: $(OBJ_FILES) $(TEST_OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) DataQualityTest

.PHONY: all clean 