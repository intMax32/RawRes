cmake_minimum_required(VERSION 3.16)

project(RawRes VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenCV REQUIRED)

find_path(LIBRAW_INCLUDE_DIR libraw/libraw.h)
find_library(LIBRAW_LIBRARY NAMES raw_r raw)

if(NOT LIBRAW_INCLUDE_DIR OR NOT LIBRAW_LIBRARY)
    message(FATAL_ERROR "LibRaw not found. Check Homebrew install path.")
endif()

add_executable(RawRes
    src/main.cpp
    src/raw_loader.cpp
    src/isp_pipeline.cpp
)

target_include_directories(RawRes PRIVATE
    include
    ${OpenCV_INCLUDE_DIRS}
    ${LIBRAW_INCLUDE_DIR}
)

target_link_libraries(RawRes PRIVATE
    ${OpenCV_LIBS}
    ${LIBRAW_LIBRARY}
)