#include <stdio.h>

#include "ofdm_utils.h"

int main() {

#define SIZE 12

    char in[SIZE];
    char out[SIZE*2];
    char out_a[SIZE];
    char out_b[SIZE];
    char tmp[SIZE*2];

    in[0]  = 0x00;
    in[1]  = 0x00;
    in[2]  = 0x20;
    in[3]  = 0x40;
    in[4]  = 0x00;
    in[5]  = 0x74;
    in[6]  = 0x00;
    in[7]  = 0x06;
    in[8]  = 0x08;
    in[9]  = 0xB3;
    in[10] = 0xEC;
    in[11] = 0x65;

    int i;
    printf("Input bit sequence:\n");
    for (i = 0; i < SIZE; i++) {
        print_bits(in[i]);
    }
    printf("\n");

    scramble_with_initial_state(in, out, SIZE, 0x5D);

    printf("Scrambled bit sequence:\n");
    for (i = 0; i < SIZE; i++) {
        print_bits(out[i]);
    }
    printf("\n");

    convolutional_encoding(out, tmp, SIZE);
    printf("Encoded bit sequence rate 1/2:\n");
    for (i = 0; i < SIZE * 2; i++) {
        print_bits(tmp[i]);
    }
    printf("\n");

    pucturing(tmp, out, SIZE * 2, RATE_3_4);

    printf("Punctured bit sequences:\n");
    for (i = 0; i < SIZE * 2 * 2 / 3; i++) {
        print_bits(out[i]);
    }
    printf("\n");

    return 0;

}
