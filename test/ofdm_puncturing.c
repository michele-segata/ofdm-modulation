#include <stdio.h>

#include "ofdm_utils.h"

int main() {

	char in_a[8];
	char in_b[8];
	char out[16];

	in_a[0] = 0x00;
	in_a[1] = 0x00;
	in_a[2] = 0x00;
	in_a[3] = 0x00;
	in_a[4] = 0x00;
	in_a[5] = 0x00;
	in_a[6] = 0x00;
	in_a[7] = 0x00;

	in_b[0] = 0xFF;
	in_b[1] = 0xFF;
	in_b[2] = 0xFF;
	in_b[3] = 0xFF;
	in_b[4] = 0xFF;
	in_b[5] = 0xFF;
	in_b[6] = 0xFF;
	in_b[7] = 0xFF;

	int i;
	printf("Input bit sequence A:\n");
	for (i = 0; i < 8; i++) {
		print_bits(in_a[i]);
	}
	printf("\n");
	printf("Input bit sequence B:\n");
	for (i = 0; i < 8; i++) {
	    print_bits(in_b[i]);
	}
	printf("\n");

	pucturing(in_a, in_b, out, 8, RATE_1_2);

	printf("Punctured bit sequence r=1/2:\n");
	for (i = 0; i < 16; i++) {
		print_bits(out[i]);
	}
	printf("\n");

	return 0;

}
