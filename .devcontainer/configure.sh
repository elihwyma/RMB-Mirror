#!/bin/sh

rm -rf build/
CXX=clang-19 cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -B build
