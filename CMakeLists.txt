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

# enable/disable electric fence library
if (ENABLE_EFENCE)
    # Add efence link
    find_library(EFENCE_LIBRARY NAMES efence PATHS /opt/local/lib/)
    set(LIBS ${LIBS} ${EFENCE_LIBRARY})
    message(STATUS "Configuring project with electric fence library")
else()
    message(STATUS "Configuring project without electric fence library. Set -DENABLE_EFENCE=ON for linking project against efence")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/build/")

find_package (fftw3 REQUIRED)

if (FFTW_FOUND)
  include_directories(${FFTW_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${FFTW_LIBRARIES})
endif (FFTW_FOUND)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set (LIBS ${LIBS} "rt")
endif()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(examples)

enable_testing()

add_test(data_field_tester            ../test/tester.sh build/ofdm_data_tester                  "misc/psdu-2012.hex"               "misc/data-2012.bits")
add_test(scrambler_tester             ../test/tester.sh build/ofdm_scrambler_tester             "misc/psdu-2012.hex"               "misc/scrambled-2012.bits")
add_test(convolutional_encoder_tester ../test/tester.sh build/ofdm_convolutional_encoder_tester "misc/scrambled-2012.bits"         "misc/encoded_3_4-2012.bits")
add_test(interleaver_tester           ../test/tester.sh build/ofdm_interleaver_tester           "misc/encoded_3_4-first-2012.bits" "misc/interleaved-first-2012.bits")
add_test(mapper_tester                ../test/tester.sh build/ofdm_mapper_tester                "misc/interleaved-first-2012.bits" "misc/mapped-first-2012.complex")
add_test(ofdm_tester                  ../test/tester.sh build/ofdm_tester                       "misc/psdu-2012.hex"               "misc/signal-2012.complex")
add_test(mac_tester                   ../test/tester.sh build/mac_frame_tester                  "misc/msdu-2012.hex"               "misc/psdu-2012.hex")
add_test(fcs_tester                   ../test/tester.sh build/mac_fcs_tester                    "misc/mac-msdu-2012.hex"           "misc/fcs-2012.hex")
