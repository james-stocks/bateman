# Variables
CMAKE_BUILD_DIR = build
CMAKE = cmake
BUILD_TOOL = cmake --build

# Default target: build the project
all: $(CMAKE_BUILD_DIR)
	$(BUILD_TOOL) $(CMAKE_BUILD_DIR)

# Create the build directory and run CMake configuration
$(CMAKE_BUILD_DIR):
	$(CMAKE) -S . -B $(CMAKE_BUILD_DIR)

# Clean the build directory
clean:
	rm -rf $(CMAKE_BUILD_DIR)

# Rebuild the project (clean + build)
rebuild: clean all