# vilgax
Modern C++ HTTP server

## Building

### Prerequisites
- CMake 3.10 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)

### Build Instructions

```bash
# Create build directory
mkdir build
cd build

# Generate build files
cmake ..

# Build the project
cmake --build . --config Release

# Run the executable
./bin/vilgax [config_file]
```

If no config file is specified, it defaults to `config/default.conf`.

### Build Options
- **Debug build**: `cmake -DCMAKE_BUILD_TYPE=Debug ..`
- **Release build**: `cmake -DCMAKE_BUILD_TYPE=Release ..`
