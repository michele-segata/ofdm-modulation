#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"

int main() {

    //psdu loaded from data file
    char psdu[1000];
    //ofdm encoding parameters
    struct OFDM_PARAMETERS params = get_ofdm_parameter(BW_20_DR_36_MBPS);
    //transmission parameters
    struct TX_PARAMETERS tx_params;
    //OFDM DATA field, and auxiliary storage
    char *data;
    //length of the DATA field
    int len;
    //scrambled data field
    char *scrambled_data;
    //encoded data field
    char *encoded_data;
    //punctured data field
    char *punctured_data;
    //interleaved data field
    char *interleaved_data;
    //OFDM modulated symbol
    fftw_complex *mod;
    //symbol with pilot carriers
    fftw_complex *pil;
    //ifft inputs
    fftw_complex *ifft;
    //symbol time samples (after ifft)
    fftw_complex *time;
    //cyclically extended symbol
    fftw_complex *ext;
    //signal header
    fftw_complex *signal;
    //short training sequence
    fftw_complex *short_sequence;
    //long training sequence
    fftw_complex *long_sequence;
    //final OFDM frame
    fftw_complex *mod_samples;
    //index of data symbol under processing
    int symbol;

    //read the psdu from text file
    int rb = read_hex_from_file("misc/psdu.hex", psdu, 1000);

    if (rb == ERR_CANNOT_READ_FILE) {
        printf("Cannot read file \"%s\": file not found?\n", "misc/data.bits");
        return 0;
    }
    if (rb == ERR_INVALID_FORMAT) {
        printf("Invalid file format\n");
        return 0;
    }

    //swap the endianless of the psdu
    change_array_endianless(psdu, rb, psdu);
    //generate the OFDM data field, adding service field and pad bits
    generate_data_field(psdu, rb, params.data_rate, &data, &len);

    //print the data field in hex format
    print_hex_array(data, len);
    printf("\n");

    //get transmission params for the psdu
    tx_params = get_tx_parameters(params.data_rate, rb);

    //alloc memory for modulation steps
    scrambled_data = calloc(len, sizeof(char));
    encoded_data = calloc(len * 2, sizeof(char));
    punctured_data = calloc(tx_params.n_encoded_data_bytes, sizeof(char));
    interleaved_data = calloc(tx_params.n_encoded_data_bytes, sizeof(char));
    mod = fftw_alloc_complex(N_DATA_SUBCARRIERS);
    pil = fftw_alloc_complex(N_TOTAL_SUBCARRIERS);
    ifft = fftw_alloc_complex(FFT_SIZE);
    time = fftw_alloc_complex(FFT_SIZE);
    ext = fftw_alloc_complex(EXT_OFDM_SYMBOL_SIZE);
    signal = fftw_alloc_complex(EXT_SIGNAL_SIZE);
    short_sequence = fftw_alloc_complex(EXT_SHORT_TRAINING_SIZE);
    long_sequence = fftw_alloc_complex(EXT_LONG_TRAINING_SIZE);

    mod_samples = fftw_alloc_complex(FRAME_SIZE(tx_params.n_sym));
    zero_samples(mod_samples, FRAME_SIZE(tx_params.n_sym));

    //first step, scrambling
    scramble_with_initial_state(data, scrambled_data, len, 0x5D);
    //encoding
    convolutional_encoding(scrambled_data, encoded_data, len);
    //puncturing
    puncturing(encoded_data, punctured_data, len * 2, params.coding_rate);
    //interleaving
    interleave(punctured_data, interleaved_data, tx_params.n_encoded_data_bytes, params.n_cbps, params.n_bpsc);

    //now perform modulation for each symbol
    for (symbol = 0; symbol < tx_params.n_sym; symbol++) {

        printf("\nModulating symbols %d of %d:\n", symbol, tx_params.n_sym);

        modulate(&interleaved_data[symbol * params.n_cbps / 8], params.n_cbps / 8, params.data_rate, mod);

        printf("Modulated bits (QAM16):\n");
        print_complex_array(mod, N_DATA_SUBCARRIERS);
        printf("\n");

        insert_pilots(mod, pil, symbol + 1);

        printf("Pilot insertion:\n");
        print_complex_array(pil, N_TOTAL_SUBCARRIERS);
        printf("\n");

        map_ofdm_to_ifft(pil, ifft);

        printf("IFFT input:\n");
        print_complex_array(ifft, FFT_SIZE);
        printf("\n");

        perform_ifft(ifft, time);
        normalize_ifft_output(time, FFT_SIZE, FFT_SIZE);

        printf("Time samples:\n");
        print_complex_array(time, FFT_SIZE);
        printf("\n");

        add_cyclic_prefix(time, FFT_SIZE, ext, EXT_OFDM_SYMBOL_SIZE, CYCLIC_PREFIX_SIZE);
        apply_window_function(ext, EXT_OFDM_SYMBOL_SIZE);

        printf("Time samples i (cyclically extended):\n");
        print_complex_array(ext, EXT_OFDM_SYMBOL_SIZE);
        printf("\n");

        sum_samples(mod_samples, ext, EXT_OFDM_SYMBOL_SIZE, (5 + symbol) * OFDM_SYMBOL_SIZE);

    }

    //generate signal field and insert it into the frame
    generate_signal_field(signal, params.data_rate, rb);
    sum_samples(mod_samples, signal, EXT_SIGNAL_SIZE, 4 * OFDM_SYMBOL_SIZE);

    printf("SIGNAL header time samples:\n");
    print_complex_array(signal, EXT_SIGNAL_SIZE);
    printf("\n");

    //generate short and long training sequences
    generate_short_training_sequence(short_sequence);
    generate_long_training_sequence(long_sequence);
    //insert them into the frame
    sum_samples(mod_samples, short_sequence, EXT_SHORT_TRAINING_SIZE, 0);
    sum_samples(mod_samples, long_sequence, EXT_LONG_TRAINING_SIZE, SHORT_TRAINING_SIZE);

    printf("OFDM frame time samples:\n");
    print_complex_array(mod_samples, FRAME_SIZE(tx_params.n_sym));
    printf("\n");

    int i;
    printf("float size: %lu\n", sizeof(float));
    for (i = 0; i < (5 + tx_params.n_sym) * OFDM_SYMBOL_SIZE; i++) {
        fprintf(stderr, "%d %f %f\n", i, mod_samples[i][0], mod_samples[i][1]);
    }


    free(data);
    free(scrambled_data);
    free(encoded_data);
    free(punctured_data);
    free(interleaved_data);
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
