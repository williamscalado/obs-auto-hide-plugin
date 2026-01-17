@echo off
setlocal

mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

echo Build complete!
