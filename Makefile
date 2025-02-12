CXX = g++
CXXFLAGS = -std=gnu++20 -g -Wall -Wextra -I include
BUILD_DIR = build
SRC_DIR = src
TARGET = $(BUILD_DIR)/chessbroccoli

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
MAIN_FILE = main.cpp

# Header files
HEADERS = $(wildcard include/*.hpp)

# Object files
SRC_OBJECTS = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/$(SRC_DIR)/%.o)
MAIN_OBJECT = $(BUILD_DIR)/$(MAIN_FILE:.cpp=.o)
OBJECTS = $(SRC_OBJECTS) $(MAIN_OBJECT)

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CXX) $(OBJECTS) -o $@

# Rule for source files in src directory
$(BUILD_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule for main.cpp in root directory
$(BUILD_DIR)/%.o: %.cpp $(HEADERS) | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean