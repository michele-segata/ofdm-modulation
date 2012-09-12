/*
 * Copyright (c) 2012 Michele Segata
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Michele Segata <michele.segata@uibk.ac.at>
 * Description: utilities for OFDM
 *
 */

#ifndef _OFDM_UTILS_H_
#define _OFDM_UTILS_H_

#include <fftw3.h>

/**
 * Define available data rates
 */
enum DATA_RATE {
    //20 MHz bandwidth: 6 to 54 Mbps
    BW_20_DR_6_MBPS,
    BW_20_DR_9_MBPS,
    BW_20_DR_12_MBPS,
    BW_20_DR_18_MBPS,
    BW_20_DR_24_MBPS,
    BW_20_DR_36_MBPS,
    BW_20_DR_48_MBPS,
    BW_20_DR_54_MBPS,
    //10 MHz bandwidth: 3 to 27 Mbps
    BW_10_DR_3_MBPS,
    BW_10_DR_4_5_MBPS,
    BW_10_DR_6_MBPS,
    BW_10_DR_9_MBPS,
    BW_10_DR_12_MBPS,
    BW_10_DR_18_MBPS,
    BW_10_DR_24_MBPS,
    BW_10_DR_27_MBPS
};

/**
 * Define coding rates for the puncturing function
 */
enum CODING_RATE {
    RATE_1_2, //r=1/2
    RATE_2_3, //r=2/3
    RATE_3_4  //r=3/4
};

/**
 * Defines available modulation schemes
 */
enum MODULATION_TYPE {
    BPSK,
    QPSK,
    QAM16,
    QAM64
};

/**
 * I Q tables for modulation schemes
 */
#define BPSK_NORMALIZATION      1.0
#define QPSK_NORMALIZATION      0.707106781
#define QAM16_NORMALIZATION     0.316227766
#define QAM64_NORMALIZATION     0.15430335

static const double bpsk_i[] = {-1 * BPSK_NORMALIZATION, 1 * BPSK_NORMALIZATION};
static const double bpsk_q[] = {0, 0};

static const double qpsk_i[] = {-1 * QPSK_NORMALIZATION, 1 * QPSK_NORMALIZATION};
static const double qpsk_q[] = {-1 * QPSK_NORMALIZATION, 1 * QPSK_NORMALIZATION};

static const double qam16_i[] = {-3 * QAM16_NORMALIZATION, -1 * QAM16_NORMALIZATION, 3 * QAM16_NORMALIZATION, 1 * QAM16_NORMALIZATION};
static const double qam16_q[] = {-3 * QAM16_NORMALIZATION, -1 * QAM16_NORMALIZATION, 3 * QAM16_NORMALIZATION, 1 * QAM16_NORMALIZATION};

static const double qam64_i[] = {-7 * QAM64_NORMALIZATION, -5 * QAM64_NORMALIZATION, -1 * QAM64_NORMALIZATION, -3 * QAM64_NORMALIZATION,
                                  7 * QAM64_NORMALIZATION,  5 * QAM64_NORMALIZATION,  1 * QAM64_NORMALIZATION,  3 * QAM64_NORMALIZATION};

static const double qam64_q[] = {-7 * QAM64_NORMALIZATION, -5 * QAM64_NORMALIZATION, -1 * QAM64_NORMALIZATION, -3 * QAM64_NORMALIZATION,
                                  7 * QAM64_NORMALIZATION,  5 * QAM64_NORMALIZATION,  1 * QAM64_NORMALIZATION,  3 * QAM64_NORMALIZATION};

/**
 * Struct containing OFDM parameters for a given
 * datarate and bandwidth
 */
struct OFDM_PARAMETERS {
    //data rate
    enum DATA_RATE          data_rate;
    //rate field of the SIGNAL header
    char                    signal_rate;
    //modulation type
    enum MODULATION_TYPE    modulation;
    //code rate
    enum CODING_RATE        coding_rate;
    //number of coded bits per sub carrier
    int                     n_bpsc;
    //number of coded bits per OFDM symbol
    int                     n_cbps;
    //number of data bits per OFDM symbol
    int                     n_dbps;
};

/**
 * Defines subcarriers polarities
 */
static const double subcarrier_polarities[] = {1,1,1,1, -1,-1,-1,1, -1,-1,-1,-1, 1,1,-1,1, -1,-1,1,1, -1,1,1,-1, 1,1,1,1, 1,1,-1,1,
                                             1,1,-1,1, 1,-1,-1,1, 1,1,-1,1, -1,-1,-1,1, -1,1,-1,-1, 1,-1,-1,1, 1,1,1,1, -1,-1,1,1,
                                             -1,-1,1,-1, 1,-1,1,1, -1,-1,-1,1, 1,-1,-1,-1, -1,1,-1,-1, 1,-1,1,1, 1,1,-1,1, -1,1,-1,1,
                                             -1,-1,-1,-1, -1,1,-1,1, 1,-1,1,-1, 1,1,1,-1, -1,1,-1,-1, -1,1,1,1, -1,-1,-1,-1, -1,-1,-1};

/**
 * Frequency domain representation of the short
 * training symbol
 */
static fftw_complex freq_short_symbol[] = {{0, 0}, {0, 0}, {1,1}, {0, 0}, {0, 0}, {0, 0}, {-1,-1}, {0, 0}, {0, 0}, {0, 0}, {1,1},
                                              {0, 0}, {0, 0}, {0, 0}, {-1,-1}, {0, 0}, {0, 0}, {0, 0}, {-1,-1}, {0, 0}, {0, 0}, {0, 0},
                                              {1,1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {-1,-1}, {0, 0}, {0, 0},
                                              {0, 0}, {-1,-1}, {0, 0}, {0, 0}, {0, 0}, {1,1}, {0, 0}, {0, 0}, {0, 0}, {1,1}, {0, 0},
                                              {0, 0}, {0, 0}, {1,1}, {0, 0}, {0, 0}, {0, 0}, {1,1}, {0, 0}, {0, 0}};

/**
 * Frequency domain representation of the long
 * training symbol
 */
static fftw_complex freq_long_symbol[] = {{1, 0}, {1, 0}, {-1, 0}, {-1, 0}, {1, 0}, {1, 0}, {-1, 0}, {1, 0}, {-1, 0}, {1, 0},
                                             {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {-1, 0}, {-1, 0}, {1, 0}, {1, 0}, {-1, 0},
                                             {1, 0}, {-1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {0, 0}, {1, 0}, {-1, 0}, {-1, 0},
                                             {1, 0}, {1, 0}, {-1, 0}, {1, 0}, {-1, 0}, {1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0},
                                             {-1, 0}, {1, 0}, {1, 0}, {-1, 0}, {-1, 0}, {1, 0}, {-1, 0}, {1, 0}, {-1, 0}, {1, 0},
                                             {1, 0}, {1, 0}, {1, 0}};


/**
 * Perform the scrambling of a set of bytes, as mandated by
 * 802.11-2007, 17.3.5.4.
 * The initial state of the register is set to all ones.
 * For a different initial state see scramble_with_initial_state
 *
 * \param in array of bytes to be scrambled
 * \param out array of bytes where to write scrambled bits
 * \param size size of in and out arrays
 */
void scramble(const char *in, char *out, int size);

/**
 * Perform the scrambling of a set of bytes, as mandated by
 * 802.11-2007, 17.3.5.4.
 *
 * \param in array of bytes to be scrambled
 * \param out array of bytes where to write scrambled bits
 * \param size size of in and out arrays
 * \param initial_state initial state of the 7-bit shift register
 */
void scramble_with_initial_state(const char *in, char *out, int size, char initial_state);

/**
 * Perform the convolutional encoding of a set of bytes, as
 * mandated by 802.11-2007, 17.3.5.5.
 * The encoder uses the two generator polynomial 133 and 171
 * base 8. The bits generated by the first polynomial are mapped
 * onto output bits 2 * i, while bits generated by the second
 * polynomial are mapped onto output bits 2 * i + 1.
 * To obtain a coding rate of 2/3 or 3/4, the output bits must
 * be passed to the puncturing() function.
 *
 * \param in array of bytes to be encoded
 * \param out array of bytes where to write bits of encoded
 * data (rate = 1/2)
 * \param size of the input array. Output array size must be twice
 * as much
 */
void convolutional_encoding(const char *in, char *out, int size);

/**
 * Perform the puncturing function on the output of the
 * convolutional encoder, in order to obtain the data bits
 * encoded with the desired coding rate.
 *
 * \param in array of the bits output by the convolutional encoder
 * at rate 1/2
 * \param out array of bytes where to store punctured bytes
 * \param size size of the in array. Notice that the size of the
 * out array depends on the coding rate r. For r = 2/3 it is
 * size * 3 / 4, for r = 3/4 it is size * 2 / 3
 * \param rate coding rate (i.e., 2/3 or 3/4)
 */
void pucturing(const char *in, char *out, int size, enum CODING_RATE rate);

/**
 * Perform the interleaving of a set of data bits
 *
 * \param in array of input bits
 * \param out array of bytes where to store interleaved databits
 * \param size size of the input array in bytes. The size of the output
 * array must be equal
 * \param n_cbps number of coded bits per symbol, i.e., how many bits
 * will be included into one OFDM symbol. Notice that size must be a
 * multiple of n_cbps
 * \param n_bpsc number of bits per subcarrier
 */
void interleave(const char *in, char *out, int size, int n_cbps, int n_bpsc);

/**
 * Perform the modulation of a set of data bits
 *
 * \param in array of input bits
 * \param size size of input array in bytes
 * \param data_rate desired datarate (mandates modulation parameters)
 * \param out two dimensional array containing modulated I,Q values
 */
void modulate(const char *in, int size, enum DATA_RATE data_rate, fftw_complex *out);

/**
 * Insert the pilot subcarriers into the modulated symbols
 *
 * \param in two dimensional array containing the I,Q values of modulated
 * bits. The size of this array must be of 48 I,Q pairs
 * \param out two dimensional array which will contain modulated I,Q values
 * plus 4 pilot subcarriers and the DC offset. Size of such array is of
 * 53 I,Q pairs
 * \param symbol_index index of the OFDM symbol within the whole transmission.
 * Notice that index 0 is the SIGNAL field, while index 1 is the first
 * OFDM data symbol
 */
void insert_pilots(fftw_complex *in, fftw_complex *out, int symbol_index);

/**
 * Given the desired datarate for the 20 MHz channel spacing, return
 * the set of parameters, such as N_CBPS, N_BPSC, puncturing rate,
 * etc.
 *
 * \param data_rate the desired datarate
 * \return a struct with all the OFDM parameters needed
 *
 */
struct OFDM_PARAMETERS get_ofdm_parameter(enum DATA_RATE data_rate);

/**
 * Maps the 53 frequency domain I,Q values modulated using OFDM
 * into the 64 frequency domain I,Q values to be given to the IFFT
 *
 * \param ofdm array of 53 OFDM modulated I,Q pairs
 * \param ifft array of 64 I,Q pairs where to store IFFT inputs
 */
void map_ofdm_to_ifft(fftw_complex *ofdm, fftw_complex *ifft);

/**
 * Performs the IFFT of a set of I,Q pairs. The expected input is an
 * array of 64 complex values, generated by the map_ofdm_to_ifft()
 * function. The output is a set of 64 time samples, result of the
 * IFFT.
 *
 * \param freq_in vector of 64 complex samples in frequency domain
 * \param time_out vector where the 64 complex output samples in time
 * domain are stored
 */
void perform_ifft(fftw_complex *freq_in, fftw_complex *time_out);

/**
 * Normalize the power of the IFFT output.
 *
 * \param in input array of comple time samples. normalized values will
 * overwrite values in the array
 * \param size size of the input array
 * \param fftSize size of the IFFT
 */
void normalize_ifft_output(fftw_complex *in, int size, int fftSize);

/**
 * Multiplies a set of complex samples by a real value.
 * Can be used for normalization.
 *
 * \param x input (and output) array of complex samples
 * \param size size of the input array
 * \param value real value used to multiply the array
 */
void multiply_by(fftw_complex *x, int size, double value);

/**
 * Adds the cyclic prefix (a.k.a. guard interval) to a set of complex
 * time samples.
 *
 * \param in input array of complex time samples
 * \param in_size size of the input array
 * \param out output array where to store cyclically extended samples
 * \param out_size desired size of the output array. Typically, in OFDM
 * input is 64 samples long and output is 80 samples long. Notice, however,
 * that this function can be used also to add the last time sample in order
 * to concatenate symbols when assembling the final signal. So another
 * use would be to have a 64 samples length input, an 81 samples length
 * output, with a 16 samples cyclic prefix length
 * \param cp_length length of the cyclic prefix
 */
void add_cyclic_prefix(fftw_complex *in, int in_size, fftw_complex *out, int out_size, int cp_length);

/**
 * Apply window function, i.e., multiply first and last element by 0.5,
 * so that consequent symbols can be overlapped (last element of a
 * symbol is summed with the first element of the successive)
 *
 * \param in set of complex time samples
 * \param size size of the input array
 */
void apply_window_function(fftw_complex *in, int size);

/**
 * Generates the short training sequence. Such sequence is already
 * windowed. The size if 161 sample, so that it can be concatenated
 * with the long training sequence
 *
 * \param out array of complex time samples where to store the 161
 * complex time samples
 */
void generate_short_training_sequence(fftw_complex *out);

/**
 * Generates the short training sequence. Such sequence is already
 * windowed. The size if 161 sample, so that it can be concatenated
 * with the following symbols
 *
 * \param out array of complex time samples where to store the 161
 * complex time samples
 */
void generate_long_training_sequence(fftw_complex *out);

#endif
