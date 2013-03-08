#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"

/**
 * This test takes in input the whole sample PSDU from 802.11-2012 annex L,
 * and performs all the OFDM encoding steps down to the complex time domain
 * representation of the whole signal. Output should be checked against
 * tables from L-22 to L-30.
 */
int main(int argc, char **argv) {

	if (argc != 2) {
		printf("error: missing input file\n");
		return 1;
	}

	//psdu loaded from data file
	char psdu[1000];
	//ofdm encoding parameters
	struct OFDM_PARAMETERS params = get_ofdm_parameter(BW_20_DR_36_MBPS);
	//transmission parameters
	struct TX_PARAMETERS tx_params;
	//OFDM DATA field, and auxiliary storage
	char *data;
	//length of the DATA field
	int len;
	//scrambled data field
	char *scrambled_data;
	//encoded data field
	char *encoded_data;
	//punctured data field
	char *punctured_data;
	//interleaved data field
	char *interleaved_data;
	//OFDM modulated symbol
	fftw_complex *mod;
	//symbol with pilot carriers
	fftw_complex *pil;
	//ifft inputs
	fftw_complex *ifft;
	//symbol time samples (after ifft)
	fftw_complex *time;
	//cyclically extended symbol
	fftw_complex *ext;
	//signal header
	fftw_complex *signal;
	//short training sequence
	fftw_complex *short_sequence;
	//long training sequence
	fftw_complex *long_sequence;
	//final OFDM frame
	fftw_complex *mod_samples;
	//index of data symbol under processing
	int symbol;

	//read the psdu from text file
	int rb = read_hex_from_file(argv[1], psdu, 1000);

	if (rb == ERR_CANNOT_READ_FILE) {
		printf("Cannot read file \"%s\": file not found?\n", argv[1]);
		return 0;
	}
	if (rb == ERR_INVALID_FORMAT) {
		printf("Invalid file format\n");
		return 0;
	}

	//swap the endianness of the psdu
	change_array_endianness(psdu, rb, psdu);
	//generate the OFDM data field, adding service field and pad bits
	generate_data_field(psdu, rb, params.data_rate, &data, &len);

	//get transmission params for the psdu
	tx_params = get_tx_parameters(params.data_rate, rb);

	//alloc memory for modulation steps
	scrambled_data = (char*)calloc(len, sizeof(char));
	encoded_data = (char*)calloc(len * 2, sizeof(char));
	punctured_data = (char*)calloc(tx_params.n_encoded_data_bytes, sizeof(char));
	interleaved_data = (char*)calloc(tx_params.n_encoded_data_bytes, sizeof(char));
	mod = fftw_alloc_complex(N_DATA_SUBCARRIERS);
	pil = fftw_alloc_complex(N_TOTAL_SUBCARRIERS);
	ifft = fftw_alloc_complex(FFT_SIZE);
	time = fftw_alloc_complex(FFT_SIZE);
	ext = fftw_alloc_complex(EXT_OFDM_SYMBOL_SIZE);
	signal = fftw_alloc_complex(EXT_SIGNAL_SIZE);
	short_sequence = fftw_alloc_complex(EXT_SHORT_TRAINING_SIZE);
	long_sequence = fftw_alloc_complex(EXT_LONG_TRAINING_SIZE);

	mod_samples = fftw_alloc_complex(FRAME_SIZE(tx_params.n_sym));
	zero_samples(mod_samples, FRAME_SIZE(tx_params.n_sym));

	//first step, scrambling
	scramble_with_initial_state(data, scrambled_data, len, 0x5D);
	//reset tail bits
	reset_tail_bits(scrambled_data, len, tx_params.n_pad);
	//encoding
	convolutional_encoding(scrambled_data, encoded_data, len);
	//puncturing
	puncturing(encoded_data, punctured_data, len * 2, params.coding_rate);
	//interleaving
	interleave(punctured_data, interleaved_data, tx_params.n_encoded_data_bytes, params.n_cbps, params.n_bpsc);

	//now perform modulation for each symbol
	for (symbol = 0; symbol < tx_params.n_sym; symbol++) {

		modulate(&interleaved_data[symbol * params.n_cbps / 8], params.n_cbps / 8, params.data_rate, mod);

		insert_pilots(mod, pil, symbol + 1);

		map_ofdm_to_ifft(pil, ifft);

		perform_ifft(ifft, time);
		normalize_ifft_output(time, FFT_SIZE, FFT_SIZE);

		add_cyclic_prefix(time, FFT_SIZE, ext, EXT_OFDM_SYMBOL_SIZE, CYCLIC_PREFIX_SIZE);
		apply_window_function(ext, EXT_OFDM_SYMBOL_SIZE);

		sum_samples(mod_samples, ext, EXT_OFDM_SYMBOL_SIZE, (5 + symbol) * OFDM_SYMBOL_SIZE);

	}

	//generate signal field and insert it into the frame
	generate_signal_field(signal, params.data_rate, rb);
	sum_samples(mod_samples, signal, EXT_SIGNAL_SIZE, 4 * OFDM_SYMBOL_SIZE);

	//generate short and long training sequences
	generate_short_training_sequence(short_sequence);
	generate_long_training_sequence(long_sequence);
	//insert them into the frame
	sum_samples(mod_samples, short_sequence, EXT_SHORT_TRAINING_SIZE, 0);
	sum_samples(mod_samples, long_sequence, EXT_LONG_TRAINING_SIZE, SHORT_TRAINING_SIZE);

	int i;
	//print the output frame
	for (i = 0; i < FRAME_SIZE(tx_params.n_sym); i++) {
		float iv, qv;
		iv = (float)mod_samples[i][0];
		qv = (float)mod_samples[i][1];

		//we have to check if a number is zero, and print it as positive
		//because otherwise printf will print -0.000, and the example in
		//the stantard always prints 0.000, so the unit test would fail
		//only because of formatting

		if (iv < 0 && iv > -1e-4) {
			iv = 0;
		}
		if (qv < 0 && qv > -1e-4) {
			qv = 0;
		}

		printf("%d %.3f %.3f\n", i, iv, qv);
	}

	compute_autocorrelation(mod_samples, FRAME_SIZE(tx_params.n_sym));
	detect_short_training_start(mod_samples, FRAME_SIZE(tx_params.n_sym), 1.2);
	detect_long_training_start(mod_samples, FRAME_SIZE(tx_params.n_sym));

	free(data);
	free(scrambled_data);
	free(encoded_data);
	free(punctured_data);
	free(interleaved_data);
	fftw_free(mod);
	fftw_free(pil);
	fftw_free(ifft);
	fftw_free(time);
	fftw_free(ext);
	fftw_free(signal);
	fftw_free(mod_samples);
	fftw_free(short_sequence);
	fftw_free(long_sequence);

	return 0;

}
