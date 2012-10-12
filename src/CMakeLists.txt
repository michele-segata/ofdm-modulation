cmake_minimum_required (VERSION 2.6)
project (ofdm-modulation)

if (FFTW_FOUND)
  include_directories(${FFTW_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${FFTW_LIBRARIES})
endif (FFTW_FOUND)

add_library(ofdm_lib bit_utils.c ofdm_utils.c mac_utils.c)
target_link_libraries(ofdm_lib ${LIBS})
