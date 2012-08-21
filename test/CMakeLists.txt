cmake_minimum_required (VERSION 2.6)
project (ofdm-modulation)

if (FFTW_FOUND)
  include_directories(${FFTW_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${FFTW_LIBRARIES})
endif (FFTW_FOUND)

add_executable(ofdm_scrambler_test ofdm_scrambler.c)
#add_executable(ofdm_puncturing_test ofdm_puncturing.c)

target_link_libraries(ofdm_scrambler_test ofdm_lib ${LIBS})
#target_link_libraries(ofdm_puncturing_test ofdm_lib)
