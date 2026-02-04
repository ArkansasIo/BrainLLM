#!/bin/bash

# Build script for BrainLLM on Linux/macOS

echo "========================================"
echo "BrainLLM Build System"
echo "========================================"

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo "Building project..."
cmake --build . -j$(nproc)

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "Build completed successfully!"
    echo "========================================"
    echo ""
    echo "Executables:"
    echo "  - GUI: ./BrainLLM_GUI"
    echo "  - API: ./BrainLLM_API"
    echo ""
else
    echo ""
    echo "========================================"
    echo "Build failed with errors!"
    echo "========================================"
fi

cd ..
