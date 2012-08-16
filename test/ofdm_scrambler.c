#include <stdio.h>

#include "ofdm_utils.h"
#include "bit_utils.h"

int main() {

#define SIZE 18

    char in[SIZE];
    char out[SIZE * 2];
    char tmp[SIZE * 2];

    int read_bytes = read_bits_from_file("misc/data.bits", in, SIZE);

    if (read_bytes == ERR_CANNOT_READ_FILE) {
        printf("Cannot read file \"%s\": file not found?\n", "misc/data.bits");
        return 0;
    }
    if (read_bytes == ERR_INVALID_FORMAT) {
        printf("Invalid file format\n");
        return 0;
    }

    printf("Read %d bytes = %d bits\n", read_bytes, read_bytes * 8);

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

    printf("Punctured bit sequence rate 3/4:\n");
    for (i = 0; i < SIZE * 2 * 2 / 3; i++) {
        print_bits(out[i]);
    }
    printf("\n");

    interleave(out, tmp, SIZE * 2 * 2 / 3, 192, 4);

    printf("Interleaved bit sequence:\n");
    for (i = 0; i < SIZE * 2 * 2 / 3; i++) {
        print_bits(tmp[i]);
    }
    printf("\n");

    return 0;

}
