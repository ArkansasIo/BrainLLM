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
JOBS=4
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    JOBS=$(sysctl -n hw.ncpu)
fi
cmake --build . -j"$JOBS"

if [ $? -eq 0 ]; then
    cd ..
    mkdir -p output/build
    echo "Packaging runtime files into output/build..."
    cp -f build/BrainLLM_API output/build/BrainLLM_API 2>/dev/null || true
    cp -f build/BrainLLM_GUI output/build/BrainLLM_GUI 2>/dev/null || true
    if [ -d build/client ]; then
        rm -rf output/build/client
        cp -R build/client output/build/client
    fi
    for folder in assets data scripts tools third_party; do
        if [ -d "build/$folder" ]; then
            rm -rf "output/build/$folder"
            cp -R "build/$folder" "output/build/$folder"
        fi
    done
    cp -f config.ini output/build/config.ini 2>/dev/null || true
    echo ""
    echo "========================================"
    echo "Build completed successfully!"
    echo "========================================"
    echo ""
    echo "Executables:"
    echo "  - GUI: ./build/BrainLLM_GUI"
    echo "  - API: ./build/BrainLLM_API"
    echo "  - Packaged API: ./output/build/BrainLLM_API"
    echo "  - Web client: http://localhost:8080/client"
    echo ""
else
    cd ..
    echo ""
    echo "========================================"
    echo "Build failed with errors!"
    echo "========================================"
fi
