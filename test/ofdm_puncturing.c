#include <stdio.h>

#include "ofdm_utils.h"

#define SIZE 24

int main() {

	char in_a[SIZE];
	char in_b[SIZE];
	char out[SIZE*2];

	int i;

	for (i = 0; i < SIZE; i++) {
	    in_a[i] = 0x00;
	    in_b[i] = 0xFF;
	}

	printf("Input bit sequence A:\n");
	for (i = 0; i < SIZE; i++) {
		print_bits(in_a[i]);
	}
	printf("\n");
	printf("Input bit sequence B:\n");
	for (i = 0; i < SIZE; i++) {
	    print_bits(in_b[i]);
	}
	printf("\n");

	pucturing(in_a, in_b, out, SIZE, RATE_1_2);

	printf("Punctured bit sequence r=1/2:\n");
	for (i = 0; i < SIZE * 2 ; i++) {
		print_bits(out[i]);
	}
	printf("\n");

    pucturing(in_a, in_b, out, SIZE, RATE_3_4);

    printf("Punctured bit sequence r=3/4:\n");
    for (i = 0; i < SIZE * 4 / 3; i++) {
        print_bits(out[i]);
    }
    printf("\n");
    pucturing(in_a, in_b, out, SIZE, RATE_2_3);

    printf("Punctured bit sequence r=2/3:\n");
    for (i = 0; i < SIZE * 3 / 2; i++) {
        print_bits(out[i]);
    }
    printf("\n");


	return 0;

}
