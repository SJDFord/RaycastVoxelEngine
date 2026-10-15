#!/bin/bash
vulkaninfo | less

mkdir -p build
cd build
cmake -S ../ -B .
make -j 4 && make Shaders && ./RayVoxEngine
cd ..
