cmake_minimum_required(VERSION 3.20)

project(comp209)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
pkg_check_modules(GPIOD QUIET libgpiod)
add_executable(runtime)

# See if TensorflowLite already exists
find_library(TFL NAMES tensorflow-lite PATHS /usr/local/lib/)

if(TFL)
    message(STATUS "TensorFlow Lite found at ${TFL}")
    target_link_libraries( runtime ${TFL} )
else()
    message(STATUS "TensorFlow Lite not found, downloading for manual installation.")
    set(TENSORFLOW_REPO "https://github.com/tensorflow/tensorflow.git")
    set(TENSORFLOW_SRC "${CMAKE_SOURCE_DIR}/tensorflow_src")
    set(TENSORFLOW_VERSION "v2.19.0")

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
        "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite"
        EXCLUDE_FROM_ALL
    )

    target_include_directories( runtime PRIVATE ${TENSORFLOW_SRC} )
    target_link_libraries( runtime tensorflow-lite)
endif()


if(GPIOD_FOUND)
    add_definitions(-DGPIOD)
endif()

set(DYNAMIXEL_REPO "https://github.com/ROBOTIS-GIT/DynamixelSDK.git")
set(DYNAMIXEL_SRC "${CMAKE_SOURCE_DIR}/dynamixel_src")
set(DYNAMIXEL_VERSION "3.8.2")
if(NOT EXISTS ${DYNAMIXEL_SRC})
    message(STATUS "Cloning Dynamixel repository...")
    execute_process(
        COMMAND git clone --depth=1 --branch ${DYNAMIXEL_VERSION} ${DYNAMIXEL_REPO} ${DYNAMIXEL_SRC}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_CLONE_RESULT
    )
    
    if(NOT GIT_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone Dynamixel repository!")
    endif()
else()
    message(STATUS "Dynamixel source directory already exists.")
endif()

target_include_directories( runtime PUBLIC include )
target_include_directories( runtime PRIVATE ${OpenCV_INCLUDE_DIRS} )
target_include_directories( runtime PRIVATE ${DYNAMIXEL_SRC}/c++/include/dynamixel_sdk )

if(GPIOD_FOUND)
    target_include_directories( runtime PRIVATE ${GPIOD_INCLUDE_DIRS})
    target_link_libraries( runtime ${GPIOD_LIBRARIES})
endif()

set_target_properties(runtime
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

target_sources( runtime
    PRIVATE
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/group_bulk_read.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/group_bulk_write.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/group_fast_bulk_read.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/group_fast_sync_read.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/group_handler.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/group_sync_read.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/group_sync_write.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/packet_handler.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/port_handler_arduino.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/port_handler_linux.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/port_handler_windows.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/port_handler_mac.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/port_handler.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/protocol1_packet_handler.cpp
        ${DYNAMIXEL_SRC}/c++/src/dynamixel_sdk/protocol2_packet_handler.cpp
)

target_sources( runtime
    PRIVATE
        src/main.cpp
        src/stepper_control.cpp
        src/servo_control.cpp
        src/face_detector.cpp
        src/landmark_extractor.cpp
)

target_link_libraries( runtime ${OpenCV_LIBS} ${OpenCV_LIBRARIES} )
