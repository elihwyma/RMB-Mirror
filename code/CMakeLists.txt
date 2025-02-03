cmake_minimum_required(VERSION 3.26)

project(comp209)

find_package(PkgConfig REQUIRED)
add_executable(runtime)

target_include_directories( runtime PUBLIC include )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

target_sources( runtime
    PRIVATE
        src/main.cpp
)
