#!/bin/sh

rm -rf build/
CXX=clang-17 cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -B build
