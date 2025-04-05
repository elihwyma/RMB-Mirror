#!/bin/sh

rm -rf build/

git clone --depth=1 --branch "3.8.2" "https://github.com/ROBOTIS-GIT/DynamixelSDK.git" dynamixel_src || true

cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug