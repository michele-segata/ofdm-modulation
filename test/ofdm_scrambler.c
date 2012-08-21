#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"

int main() {

#define SIZE 18

    char in[SIZE];
    char out[SIZE * 2];
    char tmp[SIZE * 2];
    double **mod;
    fftw_complex * ifft = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * 64);;

    struct OFDM_PARAMETERS params = get_ofdm_parameter(BW_20_DR_36_MBPS);

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

    mod = (double **) calloc(48, sizeof(double *));
    for (i = 0; i < 48; i++) {
        mod[i] = (double *) calloc(2, sizeof(double *));
    }

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

    pucturing(tmp, out, SIZE * 2, params.coding_rate);

    printf("Punctured bit sequence rate 3/4:\n");
    for (i = 0; i < SIZE * 2 * 2 / 3; i++) {
        print_bits(out[i]);
    }
    printf("\n");

    interleave(out, tmp, SIZE * 2 * 2 / 3, params.n_cbps, params.n_bpsc);

    printf("Interleaved bit sequence:\n");
    for (i = 0; i < SIZE * 2 * 2 / 3; i++) {
        print_bits(tmp[i]);
    }
    printf("\n");

    modulate(tmp, SIZE * 2 * 2 / 3, params.data_rate, mod);
    map_ofdm_to_ifft(mod, ifft);

    printf("Modulated bits (QAM16):\n");
    for (i = 0; i < 64; i++) {
        printf("(%.4f, %.4f) ", ifft[i][0], ifft[i][1]);
    }
    printf("\n");

    for (i = 0; i < 48; i++) {
        free(mod[i]);
    }
    free(mod);

    return 0;

}
