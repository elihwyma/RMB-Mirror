#!/bin/sh

rm -rf build/
CXX=clang-17 cmake -G Ninja -B build