#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"
#include "mac_utils.h"

/**
 * This test application takes in input the sample msdu of the 802.11-2012
 * standard (annex J), and generates the MAC frame (psdu) to be given to
 * PHY for encoding.
 */
int main(int argc, char **argv) {

    if (argc != 2) {
        printf("error: missing input file\n");
        return 1;
    }

    //msdu loaded from data file
    char msdu[1000];
    //mac frame
    char *mac_frame;
    //length of the mac frame
    int len;

    //read the msdu from text file
    int rb = read_hex_from_file(argv[1], msdu, 1000);

    if (rb == ERR_CANNOT_READ_FILE) {
        printf("Cannot read file \"%s\": file not found?\n", argv[1]);
        return 0;
    }
    if (rb == ERR_INVALID_FORMAT) {
        printf("Invalid file format\n");
        return 0;
    }

    //generate mac frame with default parameters
    generate_mac_data_frame(msdu, rb, &mac_frame, &len, 0);

    //output bits and we're done
    print_hex_array(mac_frame, len, '\n');
    printf("\n");

    free(mac_frame);

    return 0;

}
