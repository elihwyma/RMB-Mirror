cmake_minimum_required(VERSION 3.26)

project(comp209)

find_package(PkgConfig REQUIRED)
add_executable(runtime)

set(TENSORFLOW_SOURCE_DIR "tensorflow_src")

  add_subdirectory(
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite"
  "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)

target_include_directories( runtime PUBLIC include )
target_include_directories( runtime PRIVATE ${TENSORFLOW_SOURCE_DIR} )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

target_sources( runtime
    PRIVATE
        src/main.cpp
)

target_link_libraries( runtime
    tensorflow-lite
)