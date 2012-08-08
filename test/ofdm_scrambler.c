#include <stdio.h>

#include "ofdm_utils.h"

int main() {

	char in[8];
	char out[8];

	in[0] = 0x00;
	in[1] = 0x00;
	in[2] = 0x20;
	in[3] = 0x40;
	in[4] = 0x00;
	in[5] = 0x74;
	in[6] = 0x00;
	in[7] = 0x06;

	int i;
	printf("Input bit sequence:\n");
	for (i = 0; i < 8; i++) {
		print_bits(in[i]);
	}
	printf("\n");

	scramble(in, out, 8);

	printf("Scrambled bit sequence:\n");
	for (i = 0; i < 8; i++) {
		print_bits(out[i]);
	}
	printf("\n");

	return 0;

}
