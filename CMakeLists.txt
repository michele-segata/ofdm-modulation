cmake_minimum_required (VERSION 2.6)
project(ofdm-modulation)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    message(STATUS "Configuring project in debug mode. Set -DCMAKE_BUILD_TYPE=Release for release mode")
elseif(${CMAKE_BUILD_TYPE} MATCHES "Release")
    message(STATUS "Configuring project in release mode. Set -DCMAKE_BUILD_TYPE=Debug for debug mode")
else(${CMAKE_BUILD_TYPE} MATCHES "Release")
    message(STATUS "Configuring project in " ${CMAKE_BUILD_TYPE} " mode")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/build/")

find_package (fftw3 REQUIRED)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(examples)

enable_testing()

add_test(data_field_tester            ../test/tester.sh build/ofdm_data_tester                  "misc/psdu-2012.hex"       "misc/data-2012.bits")
add_test(scrambler_tester             ../test/tester.sh build/ofdm_scrambler_tester             "misc/psdu-2012.hex"      "misc/scrambled-2012.bits")
add_test(convolutional_encoder_tester ../test/tester.sh build/ofdm_convolutional_encoder_tester "misc/scrambled-2012.bits" "misc/encoded_3_4-2012.bits")
