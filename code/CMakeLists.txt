cmake_minimum_required(VERSION 3.26)

project(comp209)

find_package(PkgConfig REQUIRED)
add_executable(runtime)

set(TENSORFLOW_REPO "https://github.com/tensorflow/tensorflow.git")
set(TENSORFLOW_SRC "${CMAKE_SOURCE_DIR}/tensorflow_src")

if(NOT EXISTS ${TENSORFLOW_SRC})
    message(STATUS "Cloning TensorFlow repository...")
    execute_process(
        COMMAND git clone --depth=1 ${TENSORFLOW_REPO} ${TENSORFLOW_SRC}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_CLONE_RESULT
    )
    
    if(NOT GIT_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone TensorFlow repository!")
    endif()
else()
    message(STATUS "TensorFlow source directory already exists.")
endif()

add_subdirectory(
  "${TENSORFLOW_SRC}/tensorflow/lite"
  "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)

target_include_directories( runtime PUBLIC include )
target_include_directories( runtime PRIVATE ${TENSORFLOW_SRC} )

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

# Copy resources into build
file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
