#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"
#include "mac_utils.h"

#define TEXT    0
#define BIN     1

void copy_argument(char **to, const char *from) {
    *to = (char *) calloc(strlen(from) + 1, sizeof(char));
    strcpy(*to, from);
}

void usage(const char *argv0) {

    /**
     * s sender
     * a receiver
     * b bssid
     * n sequence number
     * c control
     * h help
     * f format (bin/text)
     * o output
     * p payload
     * r repeat
     * d data rate
     */
    printf("Usage %s: [-h] [-s sender mac address] [-r receiver mac address] [-b bssid] [-n sequence number] [-c control field] "
            "[-f format] [-o output] [-p payload] [-r]\n\n"
            "\t-h\tPrint this help and exit\n\n"
            "\t-b\tSet address1 field. If not specified, 00:60:08:cd:37:a6 is used\n\n"
            "\t\tThe format of any MAC address must be colon separated hexadecimal values\n\n"
            "\t-a\tSet address2 field. If not specified, 00:20:d6:01:3c:f1 is used\n\n"
            "\t-s\tSet address3. If not specified, 00:60:08:ad:3b:af is used.\n"
            "\t-n\tSet the sequence number. If repeat is specified, this will be used as starting value.\n"
            "\t\tBy default it is set to 0\n\n"
            "\t-c\tSet the MAC header frame control field. It must be made by two hexadecimal values.\n"
            "\t\tIf not specified, the default value 0402 will be used\n\n"
            "\t-f\tSet the format of the output, i.e., \"bin\" or \"text\". Binary format\n"
            "\t\tis useful for sending the complex time samples to a file which will be\n"
            "\t\tthen read in GNURadio and sent, for example, to an USRP device. Textual\n"
            "\t\toutput is useful for debugging or displaying purposes. By default, it is\n"
            "\t\tset to textual mode\n\n"
            "\t-o\tSet output file. If not specified, output is printed to stdout\n\n"
            "\t-p\tPayload of the MAC frame. If not specified, it will be prompted from stdin\n\n"
            "\t-r\tRepeat option. If activated, the program will continuously prompt a payload\n"
            "\t\tfrom stdin (so -p option is ignored if -r is set) and generate a new OFDM frame\n"
            "\t\teach time. This might be useful to send several packets writing them to a fifo\n"
            "\t\tfile (see mkfifo) which can be used as input by GNURadio\n\n"
            "\t-d\tData rate used for encoding. The parameter should specify the speed in Mbps\n"
            "\t\tof the 802.11a/g standards, i.e., 6, 9, 12, 18, 24, 36, 48 or 54. Notice that\n"
            "\t\tfor obtaining the speeds of standards with different bandwidths, like 802.11p,\n"
            "\t\tyou just need to change the sampling frequency before sending the frame with\n"
            "\t\tyour software defined radio. For example, by encoding a frame with a datarate\n"
            "\t\tof 6 Mbps and sending it out using a sampling frequency of 10 MHz, you will\n"
            "\t\tobtain a 3 Mbps frame. The default datarate is set to 36 Mbps\n", argv0);

}

int main(int argc, char **argv) {

    //sequence number in the MAC frame
    int sequence_number = 0;
    //msdu loaded from data file
    char msdu[1000];
    //generated psdu
    char *psdu = 0;
    //psdu length
    int psdu_length;
    //ofdm encoding parameters
    struct OFDM_PARAMETERS params = get_ofdm_parameter(BW_20_DR_36_MBPS);
    //transmission parameters
    struct TX_PARAMETERS tx_params;
    //OFDM DATA field, and auxiliary storage
    char *data = 0;
    //length of the DATA field
    int len;
    //scrambled data field
    char *scrambled_data = 0;
    //encoded data field
    char *encoded_data = 0;
    //punctured data field
    char *punctured_data = 0;
    //interleaved data field
    char *interleaved_data = 0;
    //OFDM modulated symbol
    fftw_complex *mod = 0;
    //symbol with pilot carriers
    fftw_complex *pil = 0;
    //ifft inputs
    fftw_complex *ifft = 0;
    //symbol time samples (after ifft)
    fftw_complex *time = 0;
    //cyclically extended symbol
    fftw_complex *ext = 0;
    //signal header
    fftw_complex *signal = 0;
    //short training sequence
    fftw_complex *short_sequence = 0;
    //long training sequence
    fftw_complex *long_sequence = 0;
    //final OFDM frame
    fftw_complex *mod_samples = 0;
    //index of data symbol under processing
    int symbol;
    //fields for the mac header
    dbyte frame_control, duration;
    //mac header
    struct MAC_DATAFRAME_HEADER header;

    //command line argument parsing

    /**
     * s address3
     * a address2
     * b address1
     * n sequence number
     * c control
     * h help
     * f format (bin/text)
     * o output
     * p payload
     * r repeat
     * d data
     */

    //sender, receiver and bssid addresses
    char *address3 = 0, *address2 = 0, *address1 = 0;
    //sequence number
    char sequence = 0;
    //control field (2 bytes)
    char control1 = 0xFF, control2 = 0xFF;
    //format 0=text 1=bin
    int format = TEXT;
    //output file
    char *outfile = 0;
    //payload
    char *payload = 0;
    //repeat option
    int repeat = 0;
    //data rate
    int data_rate = 36;

    //s r b n
    int c;
    //helper variables
    int sn;
    unsigned int v1, v2;
    //parse command line arguments
    //TODO: fix free of resources when invalid argument is specified
    while ((c = getopt(argc, argv, "ha:s:b:n:c:f:o:p:rd:")) != -1) {

        switch (c) {

        case 'h':
            usage(argv[0]);
            return 0;
            break;

        case 's':
            //set sender address
            copy_argument(&address3, optarg);
            break;

        case 'a':
            //set receiver address
            copy_argument(&address2, optarg);
            break;

        case 'b':
            //set bssid address
            copy_argument(&address1, optarg);
            break;

        case 'n':
            //set sequence number

            if (sscanf(optarg, "%d", &sn) != 1) {
                printf("Invalid sequence number %s\n", optarg);
                return 1;
            }
            sequence = (char) sn;
            break;

        case 'c':
            //set frame control field
            if (sscanf(optarg, "%2x%2x", &v1, &v2) != 2) {
                printf("Invalid frame control field %s\n", optarg);
                return 1;
            }
            control1 = (char) v1;
            control2 = (char) v2;
            break;

        case 'f':
            //set output format
            if (strcmp(optarg, "text") == 0) {
                format = TEXT;
            } else {
                if (strcmp(optarg, "bin") == 0) {
                    format = BIN;
                } else {
                    printf("Invalid output format %s. Use either \"text\" or \"bin\"\n", optarg);
                    return 1;
                }
            }
            break;

        case 'o':
            //set output file
            copy_argument(&outfile, optarg);
            break;

        case 'r':
            //set repeat flag
            repeat = 1;
            break;

        case 'p':
            //set payload
            copy_argument(&payload, optarg);
            break;

        case 'd':
            //set datarate
            if (sscanf(optarg, "%d", &data_rate) != 1) {
                printf("Invalid data rate %s\n", optarg);
                return 1;
            }

            switch (data_rate) {
            case 6:
                params = get_ofdm_parameter(BW_20_DR_6_MBPS);
                break;
            case 9:
                params = get_ofdm_parameter(BW_20_DR_9_MBPS);
                break;
            case 12:
                params = get_ofdm_parameter(BW_20_DR_12_MBPS);
                break;
            case 18:
                params = get_ofdm_parameter(BW_20_DR_18_MBPS);
                break;
            case 24:
                params = get_ofdm_parameter(BW_20_DR_24_MBPS);
                break;
            case 36:
                params = get_ofdm_parameter(BW_20_DR_36_MBPS);
                break;
            case 48:
                params = get_ofdm_parameter(BW_20_DR_48_MBPS);
                break;
            case 54:
                params = get_ofdm_parameter(BW_20_DR_54_MBPS);
                break;
            default:
                printf("Invalid data rate %s\n", optarg);
                return 1;
                break;
            }

            break;

        default:

            return 0;
            break;

        }

    }

    //init output file
    FILE *f;

    if (outfile) {
        //user wants to write to a file
        f = fopen(outfile, format == TEXT ? "w" : "wb");
        if (!f) {
            printf("Cannot open \"%s\" for write. Permission denied?\n", outfile);
            return 1;
        }
    } else {
        //user wants to write to stdout
        f = stdout;
    }

    //the size of these buffers does not depend on frame size. allocate them once
    mod = fftw_alloc_complex(N_DATA_SUBCARRIERS);
    pil = fftw_alloc_complex(N_TOTAL_SUBCARRIERS);
    ifft = fftw_alloc_complex(FFT_SIZE);
    time = fftw_alloc_complex(FFT_SIZE);
    ext = fftw_alloc_complex(EXT_OFDM_SYMBOL_SIZE);
    signal = fftw_alloc_complex(EXT_SIGNAL_SIZE);
    short_sequence = fftw_alloc_complex(EXT_SHORT_TRAINING_SIZE);
    long_sequence = fftw_alloc_complex(EXT_LONG_TRAINING_SIZE);

    //moreover, preamble is always the same for every frame
    //generate short and long training sequences
    generate_short_training_sequence(short_sequence);
    generate_long_training_sequence(long_sequence);

    //set the fields for the mac frame that won't change
    //data field
    construct_dbyte(control1, control2, &frame_control);
    //use default duration from standard
    construct_dbyte(0xFF, 0xFF, &duration);

    //print summary of arguments
    /**
         * s sender
         * a receiver
         * b bssid
         * n sequence number
         * c control
         * h help
         * f format (bin/text)
         * o output
         * p payload
         * r repeat
         * d data
         */
    header = generate_mac_header(frame_control, duration, address1, address2, address3, sequence);
    char to_from_ds;
    set_bit(&to_from_ds, 0, get_frame_control_from_ds(header.frame_control, 0));
    set_bit(&to_from_ds, 1, get_frame_control_to_ds(header.frame_control, 0));
    fprintf(stderr, "Address1:\t\t"); print_mac_address(header.address1, stderr); fprintf(stderr, " (%s)\n", STR_DATA_ADDRESSES_FUNCTIONALITY[to_from_ds][0]);
    fprintf(stderr, "Address2:\t\t"); print_mac_address(header.address2, stderr); fprintf(stderr, " (%s)\n", STR_DATA_ADDRESSES_FUNCTIONALITY[to_from_ds][1]);
    fprintf(stderr, "Address3:\t\t"); print_mac_address(header.address3, stderr); fprintf(stderr, " (%s)\n", STR_DATA_ADDRESSES_FUNCTIONALITY[to_from_ds][2]);
    fprintf(stderr, "Frame control field:\t%02hhx%02hhx\n", header.frame_control[0], header.frame_control[1]);
    print_frame_control_field(header.frame_control, stderr);
    fprintf(stderr, "Data rate:\t\t%d Mbps\n", data_rate);
    fprintf(stderr, "Sequence number:\t%d\n", (int)sequence);
    fprintf(stderr, "Output file:\t\t%s\n", outfile ? outfile : "stdout");
    fprintf(stderr, "Output format:\t\t%s\n", format == TEXT ? "textual" : "binary");
    fprintf(stderr, "Repeat:\t\t\t%s\n", repeat ? "yes" : "no");
    fprintf(stderr, "Payload:\t\t%s\n", repeat || !payload ? "read from stdin" : payload);

    //read the psdu from stdin
    char* read_result;

    //if a payload is set, and we should not repeat
    //then set the msdu as specified payload
    if (payload && !repeat) {
        strncpy(msdu, payload, 1000);
        read_result = (char *) 1;
    }

    do {

        //if we should not repeat the encoding procedure, and no payload
        //is set, take it from stdin
        if (repeat || !payload) {
            read_result = fgets(msdu, 1000, stdin);
        }

        if (read_result == NULL ) {
            break;
        }

        int rb = strlen(msdu);

        //generate custom mac header

        header = generate_mac_header(frame_control, duration, address1, address2, address3, sequence);

        //then generate the PSDU
        generate_mac_data_frame(msdu, rb, header, &psdu, &psdu_length);

        //swap the endianness of the psdu
        change_array_endianness(psdu, psdu_length, psdu);
        //generate the OFDM data field, adding service field and pad bits
        generate_data_field(psdu, psdu_length, params.data_rate, &data, &len);

        //get transmission params for the psdu
        tx_params = get_tx_parameters(params.data_rate, psdu_length);

        //alloc memory for modulation steps
        scrambled_data = calloc(len, sizeof(char));
        encoded_data = calloc(len * 2, sizeof(char));
        punctured_data = calloc(tx_params.n_encoded_data_bytes, sizeof(char));
        interleaved_data = calloc(tx_params.n_encoded_data_bytes, sizeof(char));

        mod_samples = fftw_alloc_complex(FRAME_SIZE(tx_params.n_sym));
        zero_samples(mod_samples, FRAME_SIZE(tx_params.n_sym));

        //first step, scrambling
        scramble_with_initial_state(data, scrambled_data, len, 0x5D);
        //reset tail bits
        reset_tail_bits(scrambled_data, len, tx_params.n_pad);
        //encoding
        convolutional_encoding(scrambled_data, encoded_data, len);
        //puncturing
        puncturing(encoded_data, punctured_data, len * 2, params.coding_rate);
        //interleaving
        interleave(punctured_data, interleaved_data, tx_params.n_encoded_data_bytes, params.n_cbps, params.n_bpsc);

        //now perform modulation for each symbol
        for (symbol = 0; symbol < tx_params.n_sym; symbol++) {

            modulate(&interleaved_data[symbol * params.n_cbps / 8], params.n_cbps / 8, params.data_rate, mod);

            insert_pilots(mod, pil, symbol + 1);

            map_ofdm_to_ifft(pil, ifft);

            perform_ifft(ifft, time);
            normalize_ifft_output(time, FFT_SIZE, FFT_SIZE);

            add_cyclic_prefix(time, FFT_SIZE, ext, EXT_OFDM_SYMBOL_SIZE, CYCLIC_PREFIX_SIZE);
            apply_window_function(ext, EXT_OFDM_SYMBOL_SIZE);

            sum_samples(mod_samples, ext, EXT_OFDM_SYMBOL_SIZE, (5 + symbol) * OFDM_SYMBOL_SIZE);

        }

        //generate signal field and insert it into the frame
        generate_signal_field(signal, params.data_rate, psdu_length);
        sum_samples(mod_samples, signal, EXT_SIGNAL_SIZE, 4 * OFDM_SYMBOL_SIZE);

        //insert preamble
        sum_samples(mod_samples, short_sequence, EXT_SHORT_TRAINING_SIZE, 0);
        sum_samples(mod_samples, long_sequence, EXT_LONG_TRAINING_SIZE, SHORT_TRAINING_SIZE);

        int i;
        for (i = 0; i < (5 + tx_params.n_sym) * OFDM_SYMBOL_SIZE + 1; i++) {

            if (format == BIN) {
                float val;
                val = mod_samples[i][0];
                fwrite(&val, sizeof(float), 1, f);
                val = mod_samples[i][1];
                fwrite(&val, sizeof(float), 1, f);
            } else {
                fprintf(f, "%d %f %f\n", i, mod_samples[i][0], mod_samples[i][1]);
            }
        }

        //flush the output file
        fflush(f);

        //these will be realloced at next cycle
        free(psdu);
        free(data);
        free(scrambled_data);
        free(encoded_data);
        free(punctured_data);
        free(interleaved_data);

        //increment the sequence number
        sequence_number++;

    } while (repeat);

    fclose(f);
    free(outfile);
    free(address3);
    free(address2);
    free(address1);
    free(payload);

    fftw_free(mod);
    fftw_free(pil);
    fftw_free(ifft);
    fftw_free(time);
    fftw_free(ext);
    fftw_free(signal);
    fftw_free(mod_samples);
    fftw_free(short_sequence);
    fftw_free(long_sequence);

    return 0;

}
