#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"

/**
 * This test application takes in input the scrambled sample psdu of i
 * the 802.11-2012 standard (annex J), descrambles the bits, inverts them
 * into MSB -> LSB format, removes service, tail and pad bits and outputs
 * the original MSDU.
 */
int main(int argc, char **argv) {

	if (argc != 2) {
		printf("error: missing input file\n");
		return 1;
	}

	//scrambled data loaded from data file
	char scrambled_data[1000];
	//ofdm encoding parameters
	struct OFDM_PARAMETERS params = get_ofdm_parameter(BW_20_DR_36_MBPS);
	//transmission parameter
	struct TX_PARAMETERS tx_params;
	//OFDM DATA field, and auxiliary storage
	char *data;
	//length of the DATA field
	int len;
	//scrambled data field
	char *descrambled_data;

	//read the psdu from text file
	int rb = read_bits_from_file(argv[1], scrambled_data, 1000);

	if (rb == ERR_CANNOT_READ_FILE) {
		printf("Cannot read file \"%s\": file not found?\n", argv[1]);
		return 1;
	}
	if (rb == ERR_INVALID_FORMAT) {
		printf("Invalid file format\n");
		return 1;
	}

	//alloc memory for descrambled data
	descrambled_data = (char*)calloc(rb, sizeof(char));

	//descramble to obtain the psdu in LSB -> MSB
	descramble(scrambled_data, descrambled_data, rb, 0x5D);

	//swap the endianness of the psdu to be back in MSB -> LSB
	change_array_endianness(descrambled_data, rb, descrambled_data);

	//output only the MSDU, ignoring service field, tail and pad
	//(length is hardcoded as we would need to decode the signal field
	//to know the actual length)
	print_hex_array(descrambled_data + 2, rb - 8, '\n');

	free(data);
	free(descrambled_data);

	return 0;

}
