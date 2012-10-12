#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"

/**
 * This test application takes in input the sample psdu of the 802.11-2012
 * standard (annex J), changes the endiannes, generates the DATA field
 * (i.e., it adds 16 SERVICE bits, 6 TAIL bits and PAD bits), scrambles
 * the bits with an initial seed of 0x5D and the outputs the bits
 * on stdout. Output bits must then be checked against expected output
 * bits shown in the standard.
 */
int main(int argc, char **argv) {

    if (argc != 2) {
        printf("error: missing input file\n");
        return 1;
    }

    //psdu loaded from data file
    char psdu[1000];
    //ofdm encoding parameters
    struct OFDM_PARAMETERS params = get_ofdm_parameter(BW_20_DR_36_MBPS);
    //transmission parameter
    struct TX_PARAMETERS tx_params;
    //OFDM DATA field, and auxiliary storage
    char *data;
    //length of the DATA field
    int len;
    //scrambled data field
    char *scrambled_data;

    //read the psdu from text file
    int rb = read_hex_from_file(argv[1], psdu, 1000);

    if (rb == ERR_CANNOT_READ_FILE) {
        printf("Cannot read file \"%s\": file not found?\n", argv[1]);
        return 1;
    }
    if (rb == ERR_INVALID_FORMAT) {
        printf("Invalid file format\n");
        return 1;
    }

    //swap the endianness of the psdu
    change_array_endianness(psdu, rb, psdu);

    //generate the OFDM data field, adding service field and pad bits
    generate_data_field(psdu, rb, params.data_rate, &data, &len);

    //alloc memory for modulation steps
    scrambled_data = calloc(len, sizeof(char));

    //first step, scrambling
    scramble_with_initial_state(data, scrambled_data, len, 0x5D);

    //reset TAIL bits
    tx_params = get_tx_parameters(params.data_rate, rb);
    reset_tail_bits(scrambled_data, len, tx_params.n_pad);

    //output bits and we're done
    print_bits_array(scrambled_data, len, '\n');

    free(data);
    free(scrambled_data);

    return 0;

}
