cmake_minimum_required (VERSION 2.6)
project (ofdm-modulation)

add_executable(ofdm_preamble ofdm_preamble.c)

if (FFTW_FOUND)
  include_directories(${FFTW_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${FFTW_LIBRARIES})
endif (FFTW_FOUND)

target_link_libraries(ofdm_preamble ${LIBS})

add_library(ofdm_lib bit_utils.c ofdm_utils.c)
target_link_libraries(ofdm_lib ${LIBS})
