#!/bin/bash

# Build script for macOS
mkdir -p build
cd build

# Configure
cmake .. -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt6;/opt/homebrew/opt/libobs" -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(sysctl -n hw.ncpu)

echo "Build complete!"
