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

//number of data subcarriers
#define N_DATA_SUBCARRIERS      48
//number of pilot subcarriers
#define N_PILOT_SUBCARRIERS     4
//total number of subcarriers (+1 for DC)
#define N_TOTAL_SUBCARRIERS     (N_DATA_SUBCARRIERS + N_PILOT_SUBCARRIERS + 1)
//(I)FFT size
#define FFT_SIZE                64
//length of the cyclic prefix (samples)
#define CYCLIC_PREFIX_SIZE      16
//length of an OFDM symbol (samples)
#define OFDM_SYMBOL_SIZE        (FFT_SIZE + CYCLIC_PREFIX_SIZE)
//extended ofdm symbol size (for merging with other symbols)
#define EXT_OFDM_SYMBOL_SIZE    (OFDM_SYMBOL_SIZE + 1)
//short training sequence size
#define SHORT_TRAINING_SIZE     (2 * OFDM_SYMBOL_SIZE)
//extended short training sequence size
#define EXT_SHORT_TRAINING_SIZE (SHORT_TRAINING_SIZE + 1)
//long training sequence size
#define LONG_TRAINING_SIZE      (2 * OFDM_SYMBOL_SIZE)
//extended long training sequence size
#define EXT_LONG_TRAINING_SIZE  (LONG_TRAINING_SIZE + 1)
//length of the preamble (samples)
#define PREAMBLE_SIZE           (4 * OFDM_SYMBOL_SIZE)
//extended preamble size
#define EXT_PREAMBLE_SIZE       (PREAMBLE_SIZE + 1)
//length of the signal header (samples)
#define SIGNAL_SIZE             OFDM_SYMBOL_SIZE
//extended preamble size
#define EXT_SIGNAL_SIZE         (SIGNAL_SIZE + 1)
//number of samples for an OFDM frame (function of number of DATA symbols)
#define FRAME_SIZE(n)           (((5 + n) * OFDM_SYMBOL_SIZE) + 1)

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
 * Struct containing transmission parameters,
 * such as PSDU size, number of OFDM symbols,
 * etc...
 */
struct TX_PARAMETERS {
    //data rate used for transmission
    enum DATA_RATE          data_rate;
    //PSDU size in bytes
    int                     psdu_size;
    //number of OFDM symbols (17-11)
    int                     n_sym;
    //number of data bits in the DATA field, including service and padding (17-12)
    int                     n_data;
    //number of padding bits in the DATA field (17-13)
    int                     n_pad;
    //transmission duration in microseconds
    int                     duration;
    //number of data bytes in the DATA field, including service and padding
    int                     n_data_bytes;
    //number of data bytes after encoding
    int                     n_encoded_data_bytes;
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
 * Time domain representation of the long
 * training symbol
 */
static fftw_complex time_long_symbol[] = {{0.156250, 0.000000}, {-0.005121, -0.120325}, {0.039750, -0.111158}, {0.096832, 0.082798},
                                          {0.021112, 0.027886}, {0.059824, -0.087707}, {-0.115131, -0.055180}, {-0.038316, -0.106171},
                                          {0.097541, -0.025888}, {0.053338, 0.004076}, {0.000989, -0.115005}, {-0.136805, -0.047380},
                                          {0.024476, -0.058532}, {0.058669, -0.014939}, {-0.022483, 0.160657}, {0.119239, -0.004096},
                                          {0.062500, -0.062500}, {0.036918, 0.098344}, {-0.057206, 0.039299}, {-0.131263, 0.065227},
                                          {0.082218, 0.092357}, {0.069557, 0.014122}, {-0.060310, 0.081286}, {-0.056455, -0.021804},
                                          {-0.035041, -0.150888}, {-0.121887, -0.016566}, {-0.127324, -0.020501}, {0.075074, -0.074040},
                                          {-0.002806, 0.053774}, {-0.091888, 0.115129}, {0.091717, 0.105872}, {0.012285, 0.097600},
                                          {-0.156250, 0.000000}, {0.012285, -0.097600}, {0.091717, -0.105872}, {-0.091888, -0.115129},
                                          {-0.002806, -0.053774}, {0.075074, 0.074040}, {-0.127324, 0.020501}, {-0.121887, 0.016566},
                                          {-0.035041, 0.150888}, {-0.056455, 0.021804}, {-0.060310, -0.081286}, {0.069557, -0.014122},
                                          {0.082218, -0.092357}, {-0.131263, -0.065227}, {-0.057206, -0.039299}, {0.036918, -0.098344},
                                          {0.062500, 0.062500}, {0.119239, 0.004096}, {-0.022483, -0.160657}, {0.058669, 0.014939},
                                          {0.024476, 0.058532}, {-0.136805, 0.047380}, {0.000989, 0.115005}, {0.053338, -0.004076},
                                          {0.097541, 0.025888}, {-0.038316, 0.106171}, {-0.115131, 0.055180}, {0.059824, 0.087707},
                                          {0.021112, -0.027886}, {0.096832, -0.082798}, {0.039750, 0.111158}, {-0.005121, 0.120325}};


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
 * Set to 0 the TAIL bits AFTER the scrambling has been
 * performed, as indicated in 802.11-2007, 17.3.5.2.
 *
 * \param scrambled_data the DATA bits after scrambling. Tail
 * bits will be set to 0 "in place"
 * \param size size (in bytes) of the scrambled_data array
 * \param n_pad number of bits that have been appended
 * for padding, used to locate the six TAIL bits starting
 * from the end of the scrambled DATA field
 */
void reset_tail_bits(char *scrambled_data, int size, int n_pad);

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
void puncturing(const char *in, char *out, int size, enum CODING_RATE rate);

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
 * Given the desired datarate and the size of the PSDU, return
 * the set of transmission parameters, number of OFDM symbols,
 * transmission duration, etc.
 *
 * \param data_rate the desired datarate
 * \param psdu_size size of the PSDU in bytes
 * \return a struct with all the tx parameters
 *
 */
struct TX_PARAMETERS get_tx_parameters(enum DATA_RATE data_rate, int psdu_size);

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

/**
 * Sums a set of complex time samples over a portion of another
 * set of complex time samples. This can be useful to generate the
 * final frame. For example, starting from an array of complex values
 * set all to 0, the short training sequence can be inserted by summing
 * it to values from 0 to 160. The long training sequence can be added
 * by summing it to values from 160 to 320, and so on. In such a way,
 * the function automatically merges subsequent symbols at their
 * intersection point.
 *
 * \param in_a first input array. this array is overwritten by the
 * operation
 * \param in_b second input array. this array is only taken as input
 * and summed with the first array
 * \param size_b size of the second array
 * \param base_index index of the first array where to start to store
 * the result of the sum. the size of the first array is not needed,
 * but clearly, base_index + size_b must be less than the size of
 * the first array to avoid out-of-bounds errors
 */
void sum_samples(fftw_complex *in_a, fftw_complex *in_b, int size_b, int base_index);

/**
 * Generates the complex time samples for the SIGNAL header field.
 *
 * \param out pointer to an array of complex time samples where to store
 * the SIGNAL header. The array must have a size of 81 samples
 * \param data_rate the data rate that will be used for sending the data
 * \param length number of data bytes that are passed from the MAC layer
 * to the PHY layer, i.e., the size of the PSDU
 */
void generate_signal_field(fftw_complex *out, enum DATA_RATE data_rate, int length);

/**
 * Prepare a set of data bits (i.e., the PSDU) for being processed by
 * OFDM encoding procedures, i.e., the DATA field.
 * This function adds the 16 (zero) SERVICE bits at the beginning, 6
 * (zero) tail bits at the end plus zero padding bits to yield an
 * integer number of OFDM symbols, as indicated in 17.3.5.3
 *
 * \param psdu array of bytes containing the PSDU
 * \param length number of octets in the PSDU
 * \param data_rate the desired data rate (i.e., the coding scheme)
 * that will be used for encoding
 * \param data pointer to a non-alloced array where data field will
 * be stored. The array will be malloced by the procedure
 * \param data_length pointer to an integer where to store the size of
 * the data field, in bytes
 */
void generate_data_field(const char *psdu, int length, enum DATA_RATE data_rate, char **data, int *data_length);

/**
 * Set the content of an array of complex samples to 0
 *
 * \param samples array of complex samples
 * \param size size of the array
 */
void zero_samples(fftw_complex *samples, int size);

/**
 * Compute autocorrelation
 */
int compute_autocorrelation(fftw_complex *samples, int size);

/**
 * Computes the correlation between two sets of complex time domain samples. The complexity
 * of this method is O(size)
 *
 * \param samples first set of samples (e.g., the wireless signal coming from the radio)
 * \param known_samples second set of samples (e.g., the OFDM long training sequence)
 * \param size number of complex samples into the two sets
 * \return the correlation between samples and known_samples
 */
double compute_correlation(fftw_complex *samples, fftw_complex *known_samples, int size);

/**
 * Detects the start of the short training sequence (if any) into a set of complex time
 * samples. The function computes an autocorrelation and the returns the index of the
 * first time samples where the autocorrelation values exceeds the given threshold
 *
 * \param samples set of complex time samples
 * \param size number of complex time samples into the set
 * \param correlation_threshold correlation threshold to be used for declaring the beginning
 * of a short training sequence
 * \return the index of the first time samples exceeding the correlation threshold or -1 if
 * none is found
 */
int detect_short_training_start(fftw_complex *samples, int size, double correlation_threshold);

#endif
