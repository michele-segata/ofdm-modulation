#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"

/**
 * This test application takes in input the scrambled sample psdu of the 802.11-2012
 * standard (annex J), and performs the convolutional encoding and puncturing with
 * a coding rate of 3/4, for testing the convolutional encoding step.
 */
int main(int argc, char **argv) {

	if (argc != 2) {
		printf("error: missing input file\n");
		return 1;
	}

	//scrambled psdu loaded from data file
	char scrambled_psdu[1000];
	//ofdm encoding parameters
	struct OFDM_PARAMETERS params = get_ofdm_parameter(BW_20_DR_36_MBPS);
	//transmission parameters
	struct TX_PARAMETERS tx_params;
	//encoded data field
	char *encoded_data;
	//punctured data field
	char *punctured_data;

	//read the scrambled psdu from text file
	int rb = read_bits_from_file(argv[1], scrambled_psdu, 1000);

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

	//encoding will double the size of the scrambled bit sequence
	encoded_data = (char *)calloc(rb * 2, sizeof(char));
	//for the size of the punctured data, just ask the ofdm lib :)
	punctured_data = (char *)calloc(tx_params.n_encoded_data_bytes, sizeof(char));
	//perform the convolutional encoding
	convolutional_encoding(scrambled_psdu, encoded_data, rb);
	//perform the puncturing
	puncturing(encoded_data, punctured_data, rb * 2, params.coding_rate);

	//output bits and we're done
	print_bits_array(punctured_data, tx_params.n_encoded_data_bytes, '\n');

	free(encoded_data);
	free(punctured_data);

	return 0;

}
