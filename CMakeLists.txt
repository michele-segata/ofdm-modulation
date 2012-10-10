cmake_minimum_required (VERSION 2.6)
project(ofdm-modulation)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/build/")

find_package (fftw3 REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(examples)
