cmake_minimum_required(VERSION 3.26)

project(comp209)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
add_executable(runtime)

set(TENSORFLOW_REPO "https://github.com/tensorflow/tensorflow.git")
set(TENSORFLOW_SRC "${CMAKE_SOURCE_DIR}/tensorflow_src")
set(TENSORFLOW_VERSION "v2.18.0")

if(NOT EXISTS ${TENSORFLOW_SRC})
    message(STATUS "Cloning TensorFlow repository...")
    execute_process(
        COMMAND git clone --depth=1 --branch ${TENSORFLOW_VERSION} ${TENSORFLOW_REPO} ${TENSORFLOW_SRC}
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
target_include_directories( runtime PRIVATE ${OpenCV_INCLUDE_DIRS} )

set_target_properties(runtime
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

target_sources( runtime
    PRIVATE
        src/main.cpp
        src/interpreter.cpp
)

target_link_libraries( runtime tensorflow-lite)
target_link_libraries(runtime ${OpenCV_LIBS} ${OpenCV_LIBRARIES})
     
# Copy resources into build
file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
