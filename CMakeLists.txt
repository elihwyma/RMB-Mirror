cmake_minimum_required(VERSION 3.20)

project(comp209)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
pkg_check_modules(GPIOD QUIET libgpiod)
add_executable(runtime)

if(GPIOD_FOUND)
    add_definitions(-DGPIOD)
    target_include_directories( runtime PRIVATE ${GPIOD_INCLUDE_DIRS})
    target_link_libraries( runtime ${GPIOD_LIBRARIES})
endif()

# See if TensorflowLite already exists
find_library(TFL NAMES tensorflow-lite PATHS /usr/lib/)
find_library(DXL NAMES dxl PATHS /usr/lib/)

target_link_libraries( runtime ${TFL} )
target_link_libraries( runtime ${DXL} )

target_include_directories( runtime PUBLIC include )
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
        src/stepper_control.cpp
        src/servo_control.cpp
        src/face_detector.cpp
        src/landmark_extractor.cpp
)

target_link_libraries( runtime ${OpenCV_LIBS} ${OpenCV_LIBRARIES} )
target_link_libraries (
    runtime 
        absl_log_internal_message
        absl_log_internal_check_op 
        absl_log_globals 
        absl_log_internal_globals 
        absl_log_internal_format
        absl_log_sink
        absl_log_flags
        absl_log_entry
        absl_log_internal_nullguard
        absl_log_internal_proto
        absl_log_initialize)
target_link_libraries(runtime stdc++)

# target_link_libraries (runtime i2c )