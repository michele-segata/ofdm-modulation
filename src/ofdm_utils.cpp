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

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ofdm_utils.h"
#include "bit_utils.h"
#include "utils.h"

int max(int a, int b) {
	return a > b ? a : b;
}

void scramble(const char *in, char *out, int size) {
	scramble_with_initial_state(in, out, size, 0x7F);
}

void scramble_with_initial_state(const char *in, char *out, int size, char initial_state) {

	//indexes for each byte and for each bit of the input
	int i, ib;
	//the bit which is xored with the input bit
	char scrambling_bit;
	//the OFDM scrambler register (state of the scrambler)
	char shift_register = initial_state;

	for (i = 0; i < size; i++) {

		out[i] = 0;

		//cycle from 7 to 0: bits must enter ordered from MSB to LSB
		for (ib = 7; ib >= 0; ib--) {

			//xor 7th bit with 4th bit
			scrambling_bit = get_bit(shift_register, 3) ^ get_bit(shift_register, 6);
			//input bit
			char in_bit = get_bit(in[i], ib);
			//output bit is the xor of input bit and the scrambling bit
			set_bit(&out[i], ib, scrambling_bit ^ in_bit);

			//now shift the register left
			shift_register = (shift_register << 1) | scrambling_bit;

		}

	}

}

void reset_tail_bits(char *scrambled_data, int size, int n_pad) {

	//index of the first bit of the TAIL field
	int tail_index;
	//index of current bit
	int i;

	tail_index = (size * 8) - n_pad - 6;

	for (i = 0; i < 6; i++) {
		int i_byte = (tail_index + i) / 8;
		int i_bit = 7 - (tail_index + i) % 8;
		set_bit(&scrambled_data[i_byte], i_bit, 0);
	}

}

void convolutional_encoding(const char *in, char *out, int size) {

	//generator polinomials, as defined in 17.3.5.5
	char g_0 = 0x5B; //133 base 8
	char g_1 = 0x79; //171 base 8

	//register of the encoder. only the first seven bits are used
	char encoder_register = 0x00;

	//index of the input bit we are currently considering (i = 0..8*size-1)
	int i;
	//index of the output bit we are currently considering
	int o;
	//indexes of the byte and the bit inside the input byte we are currently considering
	int i_in, ib;
	//indexes of the byte the the bit inside the output byte we are currently considering
	int i_o, ib_o = 0;

	for (i = 0; i < size * 8; i++) {

		//compute the index for current byte
		i_in = i / 8;
		//compute the index for current bit inside current byte
		ib = 7 - i % 8;

		//insert one bit from input
		set_bit(&encoder_register, 6, get_bit(in[i_in], ib));

		//compute the index for current output byte
		o = 2 * i;
		i_o = o / 8;
		//compute the index for current bit inside current output byte
		ib_o = 7 - o % 8;
		//compute the output of the first polynomial generator
		set_bit(&out[i_o], ib_o, get_polynomial(encoder_register, g_0, 7));

		//o = 2 * i + 1
		o++;
		//compute the index for current output byte
		i_o = o / 8;
		//compute the index for current bit inside current output byte
		ib_o = 7 - o % 8;
		//compute the output of the second polynomial generator
		set_bit(&out[i_o], ib_o, get_polynomial(encoder_register, g_1, 7));

		//shift the register
		encoder_register = encoder_register >> 1;

	}

}

void puncturing(const char *in, char *out, int size, enum CODING_RATE rate) {

	//index of current input bit
	int i;
	//index of current output bit
	int i_o = 0;
	//variable for containing the modulo result
	int mod;
	//should we drop current bit or not?
	int drop;

	//if rate is 1/2, then we have to keep all the bits produced by the encoder
	if (rate == RATE_1_2) {
		memcpy(out, in, sizeof(char) * size);
		return;
	}

	//cycle through all input bits
	for (i = 0; i < size * 8; i++) {

		//by default, bit must not be drop
		drop = 0;

		switch (rate) {

			case RATE_3_4:

				//period of code rate 3/4 is 6
				mod = i % 6;
				//and bits 3 and 4 must be dropped
				if (mod == 3 || mod == 4) {
					drop = 1;
				}
				break;

			case RATE_2_3:

				//period of code rate 2/3 is 4
				//and bit 3 must be dropped
				if (i % 4 == 3) {
					drop = 1;
				}
				break;

			default:
				assert(0);
				break;
		}

		if (!drop) {
			//if we should not drop, then just set output bit
			set_bit(&out[i_o / 8], 7 - i_o % 8, get_bit(in[i / 8], 7 - i % 8));
			//and move to next output bit
			i_o++;
		}

	}

}

void interleave(const char *in, char *out, int size, int n_cbps, int n_bpsc) {

	//index for the input bits
	int k;
	//index for the output bits after the first permutation
	int i, out_i;
	//index for the output bits after the second permutation
	int j, out_j;
	//helper index
	int h;
	//index of the current ofdm symbol
	int s;
	//value used for computing the second permutation
	int sp;
	//temporary storage for first permutation output
	char *first_perm;

	first_perm = (char *)calloc(sizeof(char), size);

	sp = max(n_bpsc / 2, 1);

	//run through all input bits and do the first permutation
	for (h = 0; h < size * 8; h++) {

		//compute the index of the current ofdm symbol
		s = h / n_cbps;
		//compute the index of the bit inside the current OFDM symbol
		k = h % n_cbps;

		//compute the index for the first permutation
		i = (n_cbps / 16) * (k % 16) + ((int)(k / 16));
		//i is the index w.r.t to current OFDM output symbol, but we have to put
		//the bit into the output array
		out_i = s * n_cbps + i;

		//now copy the bit
		set_bit(&first_perm[out_i / 8], 7 - out_i % 8, get_bit(in[h / 8], 7 - h % 8));

	}

	//run through all bits of the first permutation and do the second permutation
	for (h = 0; h < size * 8; h++) {

		//compute the index of the current ofdm symbol
		s = h / n_cbps;
		//compute the index of the bit inside the current OFDM symbol
		i = h % n_cbps;

		//compute the index for the first permutation
		j = sp * ((int)(i / sp)) + (i + n_cbps - ((int)(16 * i / n_cbps))) % sp;
		//j is the index w.r.t to current OFDM output symbol, but we have to put
		//the bit into the output array
		out_j = s * n_cbps + j;

		//now copy the bit
		set_bit(&out[out_j / 8], 7 - out_j % 8, get_bit(first_perm[h / 8], 7 - h % 8));

	}

	free(first_perm);

}

void modulate(const char *in, int size, enum DATA_RATE data_rate, fftw_complex *out) {

	struct OFDM_PARAMETERS p = get_ofdm_parameter(data_rate);

	int i;
	int idx;

	for (i = 0; i < size * 8; i += p.n_bpsc) {

		int in_i, in_q;

		in_i = (int)get_bit_group_value(in, size, i, max(p.n_bpsc / 2, 1));
		in_q = (int)get_bit_group_value(in, size, i + p.n_bpsc / 2, max(p.n_bpsc / 2, 1));

		idx = i / p.n_bpsc;

		switch (p.modulation) {

			case BPSK:

				assert(0 <= in_i && in_i <= 1);
				assert(0 <= in_q && in_q <= 1);

				out[idx][0] = bpsk_i[in_i];
				out[idx][1] = bpsk_q[in_q];
				break;

			case QPSK:

				assert(0 <= in_i && in_i <= 3);
				assert(0 <= in_q && in_q <= 3);

				out[idx][0] = qpsk_i[in_i];
				out[idx][1] = qpsk_q[in_q];
				break;

			case QAM16:

				assert(0 <= in_i && in_i <= 15);
				assert(0 <= in_q && in_q <= 15);

				out[idx][0] = qam16_i[in_i];
				out[idx][1] = qam16_q[in_q];
				break;

			case QAM64:

				assert(0 <= in_i && in_i <= 63);
				assert(0 <= in_q && in_q <= 63);

				out[idx][0] = qam64_i[in_i];
				out[idx][1] = qam64_q[in_q];
				break;

		}

	}

}

void insert_pilots(fftw_complex *in, fftw_complex *out, int symbol_index) {

	int i;

	//polarity of pilot subcarriers for current symbol
	int polarity = subcarrier_polarities[symbol_index];

	//on the standard, pilots are mapped inserted into positions -21, -7, 7, 21
	//using a 0-based array, means we have to put them into positions 5, 19, 33, 47
	out[5][0] = polarity;
	out[5][1] = 0;
	out[19][0] = polarity;
	out[19][1] = 0;
	out[33][0] = polarity;
	out[33][1] = 0;
	out[47][0] = -polarity;
	out[47][1] = 0;
	//insert also DC subcarrier
	out[26][0] = 0;
	out[26][1] = 0;

	//now insert modulated I,Q values (802.11-2007, 17-23)
	for (i = 0; i < 48; i++) {

		if (0 <= i && i <= 4) {
			out[i][0] = in[i][0];
			out[i][1] = in[i][1];
			continue;
		}
		if (5 <= i && i <= 17) {
			out[i + 1][0] = in[i][0];
			out[i + 1][1] = in[i][1];
			continue;
		}
		if (18 <= i && i <= 23) {
			out[i + 2][0] = in[i][0];
			out[i + 2][1] = in[i][1];
			continue;
		}
		if (24 <= i && i <= 29) {
			out[i + 3][0] = in[i][0];
			out[i + 3][1] = in[i][1];
			continue;
		}
		if (30 <= i && i <= 42) {
			out[i + 4][0] = in[i][0];
			out[i + 4][1] = in[i][1];
			continue;
		}
		if (43 <= i && i <= 47) {
			out[i + 5][0] = in[i][0];
			out[i + 5][1] = in[i][1];
		}

	}

}

struct OFDM_PARAMETERS get_ofdm_parameter(enum DATA_RATE data_rate) {

	struct OFDM_PARAMETERS p;

	p.data_rate = data_rate;

	switch (data_rate) {

		case BW_20_DR_6_MBPS:
		case BW_10_DR_3_MBPS:

			p.modulation = BPSK;
			p.coding_rate = RATE_1_2;
			p.n_bpsc = 1;
			p.n_cbps = 48;
			p.n_dbps = 24;
			p.signal_rate = 0x0D; //0b00001101

			break;

		case BW_20_DR_9_MBPS:
		case BW_10_DR_4_5_MBPS:

			p.modulation = BPSK;
			p.coding_rate = RATE_3_4;
			p.n_bpsc = 1;
			p.n_cbps = 48;
			p.n_dbps = 36;
			p.signal_rate = 0x0F; //0b00001111;

			break;

		case BW_20_DR_12_MBPS:
		case BW_10_DR_6_MBPS:

			p.modulation = QPSK;
			p.coding_rate = RATE_1_2;
			p.n_bpsc = 2;
			p.n_cbps = 96;
			p.n_dbps = 48;
			p.signal_rate = 0x05; //0b00000101;

			break;

		case BW_20_DR_18_MBPS:
		case BW_10_DR_9_MBPS:

			p.modulation = QPSK;
			p.coding_rate = RATE_3_4;
			p.n_bpsc = 2;
			p.n_cbps = 96;
			p.n_dbps = 72;
			p.signal_rate = 0x07; //0b00000111;

			break;

		case BW_20_DR_24_MBPS:
		case BW_10_DR_12_MBPS:

			p.modulation = QAM16;
			p.coding_rate = RATE_1_2;
			p.n_bpsc = 4;
			p.n_cbps = 192;
			p.n_dbps = 96;
			p.signal_rate = 0x09; //0b00001001;

			break;

		case BW_20_DR_36_MBPS:
		case BW_10_DR_18_MBPS:

			p.modulation = QAM16;
			p.coding_rate = RATE_3_4;
			p.n_bpsc = 4;
			p.n_cbps = 192;
			p.n_dbps = 144;
			p.signal_rate = 0x0B; //0b00001011;

			break;

		case BW_20_DR_48_MBPS:
		case BW_10_DR_24_MBPS:

			p.modulation = QAM64;
			p.coding_rate = RATE_2_3;
			p.n_bpsc = 6;
			p.n_cbps = 288;
			p.n_dbps = 192;
			p.signal_rate = 0x01; //0b00000001;

			break;

		case BW_20_DR_54_MBPS:
		case BW_10_DR_27_MBPS:

			p.modulation = QAM64;
			p.coding_rate = RATE_3_4;
			p.n_bpsc = 6;
			p.n_cbps = 288;
			p.n_dbps = 216;
			p.signal_rate = 0x03; //0b00000011;

			break;

		default:

			assert(0);

			break;

	}

	return p;

}

struct TX_PARAMETERS get_tx_parameters(enum DATA_RATE data_rate, int psdu_size) {

	//returned tx parameters
	struct TX_PARAMETERS tx_params;
	//ofdm parameters
	struct OFDM_PARAMETERS ofdm_params = get_ofdm_parameter(data_rate);

	tx_params.data_rate = data_rate;
	tx_params.psdu_size = psdu_size;

	//compute number of symbols (17-11)
	tx_params.n_sym = (int)ceil((16 + 8 * psdu_size + 6) / (double)ofdm_params.n_dbps);
	//compute number of bits of the data field (17-12)
	tx_params.n_data = tx_params.n_sym * ofdm_params.n_dbps;
	//compute number of padding bits (17-13)
	tx_params.n_pad = tx_params.n_data - (16 + 8 * psdu_size + 6);
	//number of data bytes
	tx_params.n_data_bytes = tx_params.n_data / 8;
	//number of data bytes after encoding and puncturing
	switch (ofdm_params.coding_rate) {
		case RATE_1_2:
			tx_params.n_encoded_data_bytes = tx_params.n_data_bytes * 2;
			break;
		case RATE_3_4:
			tx_params.n_encoded_data_bytes = tx_params.n_data_bytes * 4 / 3;
			break;
		case RATE_2_3:
			tx_params.n_encoded_data_bytes = tx_params.n_data_bytes * 3 / 2;
			break;
	}

	switch (data_rate) {

		case BW_10_DR_3_MBPS:
		case BW_10_DR_4_5_MBPS:
		case BW_10_DR_6_MBPS:
		case BW_10_DR_9_MBPS:
		case BW_10_DR_12_MBPS:
		case BW_10_DR_18_MBPS:
		case BW_10_DR_24_MBPS:
		case BW_10_DR_27_MBPS:

			//for the 10 MHz bandwidth, symbol duration is 8 microseconds
			tx_params.duration = (5 + tx_params.n_sym) * 8;

			break;

		case BW_20_DR_6_MBPS:
		case BW_20_DR_9_MBPS:
		case BW_20_DR_12_MBPS:
		case BW_20_DR_18_MBPS:
		case BW_20_DR_24_MBPS:
		case BW_20_DR_36_MBPS:
		case BW_20_DR_48_MBPS:
		case BW_20_DR_54_MBPS:

			//for the 20 MHz bandwidth, symbol duration is 4 microseconds
			tx_params.duration = (5 + tx_params.n_sym) * 4;

			break;
	}

	return tx_params;

}

void map_ofdm_to_ifft(fftw_complex *ofdm, fftw_complex *ifft) {

	// out 0        must be 0
	// out 1->26    must be 1->26, base 0 means 27->53
	// out 27->37   must be 0
	// out 38->63   must be -26->-1 base 0 means 0->25

	int i;
	ifft[0][0] = 0;
	ifft[0][1] = 0;

	for (i = 1; i <= 26; i++) {
		ifft[i][0] = ofdm[i + 26][0];
		ifft[i][1] = ofdm[i + 26][1];
	}

	for (i = 27; i <= 37; i++) {
		ifft[i][0] = 0;
		ifft[i][1] = 0;
	}

	for (i = 38; i <= 63; i++) {
		ifft[i][0] = ofdm[i - 38][0];
		ifft[i][1] = ofdm[i - 38][1];
	}

}

void perform_ifft(fftw_complex *freq_in, fftw_complex *time_out) {

	fftw_plan ifft;

	fftw_complex *in = fftw_alloc_complex(64);
	fftw_complex *out = fftw_alloc_complex(64);

	int i;
	for (i = 0; i < 64; i++) {
		in[i][0] = freq_in[i][0];
		in[i][1] = freq_in[i][1];
	}

	ifft = fftw_plan_dft_1d(64, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
	fftw_execute(ifft);

	for (i = 0; i < 64; i++) {
		time_out[i][0] = out[i][0];
		time_out[i][1] = out[i][1];
	}

	fftw_destroy_plan(ifft);

	fftw_free(in);
	fftw_free(out);

}

void normalize_ifft_output(fftw_complex *in, int size, int fftSize) {

	int i;
	for (i = 0; i < size; i++) {
		in[i][0] *= 1.0 / (double)fftSize;
		in[i][1] *= 1.0 / (double)fftSize;
	}

}

void multiply_by(fftw_complex *x, int size, double value) {

	int i;
	for (i = 0; i < size; i++) {
		x[i][0] *= value;
		x[i][1] *= value;
	}

}

void add_cyclic_prefix(fftw_complex *in, int in_size, fftw_complex *out, int out_size, int cp_length) {

	//index of current sample
	int i;

	assert(cp_length < in_size);

	//0  ... 15 16 ... 79 80 ... 95 96 ... 159 160
	//48 ... 63 0  ... 63 0  ... 63 0  ... 63  0

	for (i = cp_length; i < out_size; i++) {
		out[i][0] = in[(i - cp_length) % in_size][0];
		out[i][1] = in[(i - cp_length) % in_size][1];
	}
	for (i = 0; i < cp_length; i++) {
		out[i][0] = in[(i + in_size - cp_length) % in_size][0];
		out[i][1] = in[(i + in_size - cp_length) % in_size][1];
	}

}

void apply_window_function(fftw_complex *in, int size) {
	in[0][0] *= 0.5;
	in[0][1] *= 0.5;
	in[size - 1][0] *= 0.5;
	in[size - 1][1] *= 0.5;
}

void generate_short_training_sequence(fftw_complex *out) {

	fftw_complex *ifft = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * 64);
	fftw_complex *symbol = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * 64);

	//map the OFDM frequency domain representation to IFFT inputs
	map_ofdm_to_ifft(freq_short_symbol, ifft);
	//normalize input
	multiply_by(ifft, 64, sqrt(13.0 / 6.0));
	//perform the IFFT
	perform_ifft(ifft, symbol);
	//normalize the power
	normalize_ifft_output(symbol, 64, 64);
	//cyclically repeat the symbol
	add_cyclic_prefix(symbol, 64, out, 161, 0);
	//apply the window function for merging with long training sequence
	apply_window_function(out, 161);

	fftw_free(symbol);
	fftw_free(ifft);

}

void generate_long_training_sequence(fftw_complex *out) {

	fftw_complex *ifft = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * 64);
	fftw_complex *symbol = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * 64);

	//map the OFDM frequency domain representation to IFFT inputs
	map_ofdm_to_ifft(freq_long_symbol, ifft);
	//perform the IFFT
	perform_ifft(ifft, symbol);
	//normalize the power
	normalize_ifft_output(symbol, 64, 64);
	//cyclically repeat the symbol
	add_cyclic_prefix(symbol, 64, out, 161, 32);
	//apply the window function for merging with long training sequence
	apply_window_function(out, 161);

	fftw_free(symbol);
	fftw_free(ifft);

}

void sum_samples(fftw_complex *in_a, fftw_complex *in_b, int size_b, int base_index) {

	int i;

	for (i = 0; i < size_b; i++) {

		in_a[i + base_index][0] += in_b[i][0];
		in_a[i + base_index][1] += in_b[i][1];
	}

}

void generate_signal_field(fftw_complex *out, enum DATA_RATE data_rate, int length) {

	struct OFDM_PARAMETERS params, header_params;
	//data bits of the signal header
	char *signal_header = (char *)malloc(sizeof(char) * 3);

	//signal header after...
	//convolutional encoding
	char *encoded_signal_header = (char *)malloc(sizeof(char) * 6);
	//interleaving
	char *interleaved_signal_header = (char *)malloc(sizeof(char) * 6);
	//BPSK modulation
	fftw_complex *modulated = fftw_alloc_complex(48);
	//pilots insertion
	fftw_complex *pilots = fftw_alloc_complex(53);
	//mapping to ifft inputs
	fftw_complex *ifft = fftw_alloc_complex(64);
	//IFFT (time domain samples)
	fftw_complex *time = fftw_alloc_complex(64);

	//get parameters for payload datarate
	params = get_ofdm_parameter(data_rate);
	//header is transmitted at lowest possible datarate
	header_params = get_ofdm_parameter(BW_20_DR_6_MBPS);

	int i;
	//first 4 bits represent the modulation and coding scheme
	set_bit(&signal_header[0], 7, get_bit(params.signal_rate, 3));
	set_bit(&signal_header[0], 6, get_bit(params.signal_rate, 2));
	set_bit(&signal_header[0], 5, get_bit(params.signal_rate, 1));
	set_bit(&signal_header[0], 4, get_bit(params.signal_rate, 0));
	//5th bit is reserved and must be set to 0
	set_bit(&signal_header[0], 3, 0);
	//then 12 bits represent the length
	set_bit(&signal_header[0], 2, get_bit(length, 0));
	set_bit(&signal_header[0], 1, get_bit(length, 1));
	set_bit(&signal_header[0], 0, get_bit(length, 2));
	set_bit(&signal_header[1], 7, get_bit(length, 3));
	set_bit(&signal_header[1], 6, get_bit(length, 4));
	set_bit(&signal_header[1], 5, get_bit(length, 5));
	set_bit(&signal_header[1], 4, get_bit(length, 6));
	set_bit(&signal_header[1], 3, get_bit(length, 7));
	set_bit(&signal_header[1], 2, get_bit(length, 8));
	set_bit(&signal_header[1], 1, get_bit(length, 9));
	set_bit(&signal_header[1], 0, get_bit(length, 10));
	set_bit(&signal_header[2], 7, get_bit(length, 11));
	//18-th bit is the parity bit for the first 17 bits
	set_bit(&signal_header[2], 6, compute_even_parity(signal_header, 0, 17));
	//last 6 bits must be set to 0
	for (i = 0; i < 6; i++) {
		set_bit(&signal_header[2], i, 0);
	}

	//now perform convolutional encoding (scrambling is not needed)
	convolutional_encoding(signal_header, encoded_signal_header, 3);
	//interleaving
	interleave(encoded_signal_header, interleaved_signal_header, 6, header_params.n_cbps, header_params.n_bpsc);
	//modulation
	modulate(interleaved_signal_header, 6, BW_20_DR_6_MBPS, modulated);
	//insert pilot sub carriers (symbol index = 0, SIGNAL header is the first OFDM symbol)
	insert_pilots(modulated, pilots, 0);
	//mapping OFDM to IFFT inputs
	map_ofdm_to_ifft(pilots, ifft);
	//perform IFFT
	perform_ifft(ifft, time);
	//normalize signal power
	normalize_ifft_output(time, 64, 64);
	//extend with cyclic prefix
	add_cyclic_prefix(time, 64, out, 81, 16);
	//apply window function
	apply_window_function(out, 81);

	fftw_free(modulated);
	fftw_free(pilots);
	fftw_free(ifft);
	fftw_free(time);
	free(signal_header);
	free(encoded_signal_header);
	free(interleaved_signal_header);

}

void generate_data_field(const char *psdu, int length, enum DATA_RATE data_rate, char **data, int *data_length) {

	//get modulation scheme parameter for computing padding size
	struct OFDM_PARAMETERS params = get_ofdm_parameter(data_rate);
	//number of OFDM symbols
	int n_sym;
	//number of bits of the data field
	int n_data;

	//compute number of symbols (17-11)
	n_sym = (int)ceil((16 + 8 * length + 6) / (double)params.n_dbps);
	//compute number of bits of the data field (17-12)
	n_data = n_sym * params.n_dbps;

	//alloc data
	*data_length = n_data / 8;
	*data = (char *)calloc(*data_length, sizeof(char));

	//calloc function already sets all elements to 0. we just need to copy psdu after first 16 service bits
	memcpy(*data + 2, psdu, length);

}

void zero_samples(fftw_complex *samples, int size) {
	int i;
	for (i = 0; i < size; i++) {
		samples[i][0] = 0;
		samples[i][1] = 0;
	}
}
inline double complex_magnitude(fftw_complex a) {
	return sqrt(pow(a[0], 2) + pow(a[1], 2));
}

inline void complex_product(fftw_complex a, fftw_complex b, fftw_complex *res) {
	(*res)[0] = a[0] * b[0] - a[1] * b[1];
	(*res)[1] = a[0] * b[1] + a[1] * b[0];
}

inline double update_conj_product(double current_product, fftw_complex sample_a, fftw_complex sample_b, int sum) {
	current_product += sum * sample_a[0] * sample_b[0];
	current_product += sum * sample_a[1] * sample_b[1];
	return current_product;
}
int compute_autocorrelation(fftw_complex *samples, int size) {

	//performance counters
	nanotimer_t startt;

	int start_seq = -1;

	//number of samples taken into account for computing the autocorrelation
	int n_autocorrelation_samples = 48;
	int L = 32;
	double *norm = calloc(size, sizeof(double));
	int i;
	double upperProduct = 0, lowerProduct = 0;
	double auto_correlation, threshold = 1.2;

	assert(size >= n_autocorrelation_samples);

	startt = start_timer();

	//accumulate auto correlation for first scanWindowSize samples
	for (i = 1; i <= L; i++) {
		upperProduct = update_conj_product(upperProduct, samples[n_autocorrelation_samples - i], samples[n_autocorrelation_samples - i - 16], 1);
		lowerProduct += pow(samples[i + 16][0], 2) + pow(samples[i + 16][1], 2);
	}

	for (i = 0; i < size; ++i) {
		norm[i] = pow(samples[i][0], 2) + pow(samples[i][1], 2);
	}

	//compute first autocorrelation value
	assert(lowerProduct != 0);
	auto_correlation = upperProduct / lowerProduct;

	if (auto_correlation > threshold) {
		//found short sequence start at first sample: stop autocorrelator and return
		start_seq = 0;
	}

	//otherwise go on samples by samples

	int start = 1;
	int max = size - n_autocorrelation_samples;
	while (start < max) {

		//insert into upper product new sample
		upperProduct = update_conj_product(upperProduct, samples[n_autocorrelation_samples + start], samples[n_autocorrelation_samples + start - 16], 1);
		//remove from upper product old sample
		upperProduct = update_conj_product(upperProduct, samples[n_autocorrelation_samples + start - L], samples[n_autocorrelation_samples + start - L - 16], -1);
		//insert into lower product norm of new sample
		lowerProduct += pow(samples[L + start + 16][0], 2) + pow(samples[L + start + 16][1], 2);
		//remove from lower product norm of old sample
		lowerProduct -= pow(samples[start + 15][0], 2) + pow(samples[start + 15][1], 2);

		assert(lowerProduct != 0);
		//compute new autocorrelation value
		auto_correlation = upperProduct / lowerProduct;

		if (auto_correlation > threshold) {
			start_seq = start;
			break;
			//found short sequence start: stop autocorrelator and return
		}

		//printf("sample %d corr %f\n", start, auto_correlation);

		start++;

	}

	nanotimer_t elapsed_ns = elapsed_nanosecond(startt);
	double elapsed = ((double)elapsed_ns) / 1e9;    //(end_time.tv_usec - start_time.tv_usec) / 1.0e6 + end_time.tv_sec - start_time.tv_sec;
//    printf("processed %d samples in %lu ns. speed: %f Msps\n", start, elapsed_ns, size / elapsed / 1e6);

	return start_seq;

}

double compute_correlation(fftw_complex *samples, fftw_complex *known_samples, int size) {

	fftw_complex correlation = {0, 0};
	double norm_factor = 0;

	int i;

	for (i = 0; i < size; i++) {
		norm_factor += pow(samples[i][0], 2) + pow(samples[i][1], 2);
	}

	for (i = 0; i < size; i++) {
		fftw_complex product;
		complex_product(samples[i], known_samples[i], &product);
		correlation[0] += product[0];
		correlation[1] += product[1];
	}

	correlation[0] *= sqrt(norm_factor);
	correlation[1] *= sqrt(norm_factor);

	return complex_magnitude(correlation) / norm_factor;

}

int detect_long_training_start(fftw_complex *samples, int size) {

	int i;

	nanotimer_t startt;
	startt = start_timer();

	for (i = 0; i < size - 64; i++) {
		if (compute_correlation(&samples[i], time_long_symbol, 64) > 2) {
			break;
		}
	}

	nanotimer_t elapsed_ns = elapsed_nanosecond(startt);
	double elapsed = ((double)elapsed_ns) / 1e9;
//    printf("processed %d samples in %lu ns. speed: %f Msps\n", i, elapsed_ns, i / elapsed / 1e6);

}

int detect_short_training_start(fftw_complex *samples, int size, double correlation_threshold) {

	int short_training_start = -1;
	int i;

	//timer for profiling the code
	nanotimer_t startt;
	startt = start_timer();

	for (i = 0; i < size - 64; i++) {
		/**
		 * This way of computing the autocorrelation might seem stupid because
		 * we are re-doing operation for each new time sample. However, if the
		 * compiler optimizes the code for SIMD instructions, this will be much
		 * faster than the smarter way of computing autocorrelation
		 */
//        compute_correlation(&samples[i], &samples[i+16], 48);
		if (compute_correlation(&samples[i], &samples[i + 16], 48) > correlation_threshold) {
			short_training_start = i;
			break;
		}
	}

	nanotimer_t elapsed_ns = elapsed_nanosecond(startt);
	double elapsed = ((double)elapsed_ns) / 1e9;
//    printf("processed %d samples in %lu ns. speed: %f Msps\n", i, elapsed_ns, i / elapsed / 1e6);

	return short_training_start;

}
