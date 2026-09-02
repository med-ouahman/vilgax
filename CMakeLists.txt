cmake_minimum_required(VERSION 3.10)
project(vilgax VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -O2")

# Add source files
set(SOURCES
    src/main.cpp
    src/config/config.cpp
    src/config/lexer.cpp
    src/config/parser.cpp
    src/core/master.cpp
    src/core/server.cpp
    src/core/worker.cpp
)

# Add header directories
set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

# Create executable
add_executable(vilgax ${SOURCES})

# Link include directories
target_include_directories(vilgax PRIVATE ${INCLUDE_DIRS})

# Set output directory
set_target_properties(vilgax PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install target
install(TARGETS vilgax DESTINATION /usr/local/bin)

# Enable testing
enable_testing()

# Google Test Framework - use FetchContent to download if not found
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

