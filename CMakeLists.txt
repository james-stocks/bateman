cmake_minimum_required(VERSION 3.10)
project(Bateman)

find_package(BISON REQUIRED)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

# Find ZLIB
find_package(ZLIB REQUIRED)

# Generate lexer and parser
BISON_TARGET(Parser src/parser.y ${CMAKE_BINARY_DIR}/parser.cpp DEFINES_FILE ${CMAKE_BINARY_DIR}/parser.hpp)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/lexer.cpp DEPENDS src/lexer.l COMMAND flex -o ${CMAKE_BINARY_DIR}/lexer.cpp ${CMAKE_SOURCE_DIR}/src/lexer.l)

# Define executable
add_executable(bateman
    src/main.cpp
    ${BISON_Parser_OUTPUTS}
    ${CMAKE_BINARY_DIR}/lexer.cpp
    src/codegen.cpp
    src/runtime.cpp)

# Include directories
target_include_directories(bateman PRIVATE ${LLVM_INCLUDE_DIRS} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src /usr/include)

# Link libraries
llvm_map_components_to_libnames(LLVM_LIBS Core Support X86)
target_link_libraries(bateman PRIVATE ${LLVM_LIBS} ZLIB::ZLIB)

# Ensure lexer.cpp is treated as C++
set_source_files_properties(${CMAKE_BINARY_DIR}/lexer.cpp PROPERTIES LANGUAGE CXX)

# Add this line to enable Bison debugging
target_compile_definitions(bateman PRIVATE YYDEBUG=1)