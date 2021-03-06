cmake_minimum_required (VERSION 2.6)
project (ofdm-modulation)

#if (FFTW_FOUND)
#  include_directories(${FFTW_INCLUDE_DIRS})
#  set(LIBS ${LIBS} ${FFTW_LIBRARIES})
#endif (FFTW_FOUND)

# DATA field tester
add_executable(ofdm_data_tester ofdm_data_tester.c)
# scrambler tester
add_executable(ofdm_scrambler_tester ofdm_scrambler_tester.c)
# convolutional encoder tester
add_executable(ofdm_convolutional_encoder_tester ofdm_convolutional_encoder_tester.c)
# interleaver tester
add_executable(ofdm_interleaver_tester ofdm_interleaver_tester.c)
# mapper tester
add_executable(ofdm_mapper_tester ofdm_mapper_tester.c)
# whole ofdm encoding procedure test
add_executable(ofdm_tester ofdm_tester.c)
# MAC framer tester
add_executable(mac_frame_tester mac_frame_tester)
# mac frame check sequence tester
add_executable(mac_fcs_tester mac_fcs_tester.c)

target_link_libraries(ofdm_data_tester ofdm_lib ${LIBS})
target_link_libraries(ofdm_scrambler_tester ofdm_lib ${LIBS})
target_link_libraries(ofdm_convolutional_encoder_tester ofdm_lib ${LIBS})
target_link_libraries(ofdm_interleaver_tester ofdm_lib ${LIBS})
target_link_libraries(ofdm_mapper_tester ofdm_lib ${LIBS})
target_link_libraries(ofdm_tester ofdm_lib ${LIBS})
target_link_libraries(mac_frame_tester ofdm_lib ${LIBS})
target_link_libraries(mac_fcs_tester ofdm_lib ${LIBS})
