#!/bin/bash
# Build script for Vilgax - configures and compiles the project
# Supports incremental builds (only recompiles changed files)
# Use './build.sh clean' to do a full rebuild

set -e  # Exit on error

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$SCRIPT_DIR/build"

# Check if user wants a clean rebuild
if [ "$1" = "clean" ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    echo "Build directory cleaned."
fi

echo "Building Vilgax..."

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Configuring CMake..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake ..
else
    cd "$BUILD_DIR"
fi

# Build the project (incremental - only rebuilds changed files)
cmake --build .

echo "Build complete! Executable at: $BUILD_DIR/bin/vilgax"
echo "Run with: ./run"
