#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#include "mac_utils.h"
#include "bit_utils.h"

/**
 * This test application takes in input the sample msdu plus mac header
 * of the 802.11-2012 standard (annex J) and computes the frame check
 * sequence (FCS), i.e., the CRC32
 */
int main(int argc, char **argv) {

	if (argc != 2) {
		printf("error: missing input file\n");
		return 1;
	}

	//msdu plus mac header loaded from data file
	char msdu[1000];

	//read the psdu from text file
	int rb = read_hex_from_file(argv[1], msdu, 1000);

	if (rb == ERR_CANNOT_READ_FILE) {
		printf("Cannot read file \"%s\": file not found?\n", argv[1]);
		return 1;
	}
	if (rb == ERR_INVALID_FORMAT) {
		printf("Invalid file format\n");
		return 1;
	}

	unsigned int fcs = crc32(msdu, rb);

	print_hex_array((const char *)&fcs, sizeof(unsigned int), '\n');
	printf("\n");

	return 0;

}
