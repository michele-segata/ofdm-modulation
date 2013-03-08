#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"

/**
 * This test application takes in input the convolutionally encoded bits of the 802.11-2012
 * standard (annex J), and performs the interleaving step.
 */
int main(int argc, char **argv) {

	if (argc != 2) {
		printf("error: missing input file\n");
		return 1;
	}

	//encoded psdu loaded from data file
	char encoded_psdu[1000];
	//ofdm encoding parameters
	struct OFDM_PARAMETERS params = get_ofdm_parameter(BW_20_DR_36_MBPS);
	//transmission parameters
	struct TX_PARAMETERS tx_params;
	//encoded data field
	char *interleaved_data;

	//read the scrambled psdu from text file
	int rb = read_bits_from_file(argv[1], encoded_psdu, 1000);

	if (rb == ERR_CANNOT_READ_FILE) {
		printf("Cannot read file \"%s\": file not found?\n", argv[1]);
		return 1;
	}
	if (rb == ERR_INVALID_FORMAT) {
		printf("Invalid file format\n");
		return 1;
	}

	//the size of the psdu in the 802.11-2012 example is 100 bytes
	tx_params = get_tx_parameters(params.data_rate, 100);

	//after interleaving, the size is the same after encoding
	interleaved_data = (char *)calloc(rb, sizeof(char));
	//perform interleaving
	interleave(encoded_psdu, interleaved_data, rb, params.n_cbps, params.n_bpsc);

	//output bits and we're done
	print_bits_array(interleaved_data, rb, '\n');

	free(interleaved_data);

	return 0;

}
