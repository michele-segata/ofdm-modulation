#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"

/**
 * This test application takes in input the sample psdu of the 802.11-2012
 * standard (annex J), generates the DATA field (i.e., it adds 16 SERVICE
 * bits, 6 TAIL bits and PAD bits) and then outputs the bits on stdout.
 * Output bits must then be checked against the ones shown in the standard.
 * Notice that the endiannes is not changed, because in the standard the
 * sample DATA field is displayed MSB-LSB, but for the generation of the
 * DATA field, this has no impact, since we are adding only 0 bytes.
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
    //OFDM DATA field, and auxiliary storage
    char *data;
    //length of the DATA field
    int len;

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

    //generate the OFDM data field, adding service field and pad bits
    generate_data_field(psdu, rb, params.data_rate, &data, &len);

    //output bits and we're done
    print_bits_array(data, len, '\n');

    free(data);

    return 0;

}
