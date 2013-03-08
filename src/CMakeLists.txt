cmake_minimum_required (VERSION 2.6)
project (ofdm-modulation)

if (FFTW_FOUND)
  include_directories(${FFTW_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${FFTW_LIBRARIES})
endif (FFTW_FOUND)

add_library(ofdm_lib bit_utils.cpp ofdm_utils.cpp mac_utils.cpp utils.cpp)
target_link_libraries(ofdm_lib ${LIBS})
