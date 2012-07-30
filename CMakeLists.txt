cmake_minimum_required (VERSION 2.6)
project(ofdm-modulation)
add_subdirectory(src)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/build/")
