cmake_minimum_required (VERSION 2.6)
project (ofdm-modulation)
add_executable(ofdm_scrambler_test ofdm_scrambler.c)
target_link_libraries(ofdm_scrambler_test ofdm_lib)
