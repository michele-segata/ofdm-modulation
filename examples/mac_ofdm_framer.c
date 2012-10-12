#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"
#include "mac_utils.h"

int main(int argc, char **argv) {

    //sequence number in the MAC frame
    int sequence_number = 0;
    //msdu loaded from data file
    char msdu[1000];
    //generated psdu
    char *psdu;
    //psdu length
    int psdu_length;
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
    //fields for the mac header
    dbyte frame_control, duration;
    //mac header
    struct MAC_DATAFRAME_HEADER header;

    //fifo output file
    FILE *f = fopen("ofdm.fifo", "ab");

    if (f == 0) {
        printf("File ofdm.fifo not found. Create it with 'mkfifo ofdm.fifo'\n");
        return 0;
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
    construct_dbyte(0x08, 0x02, &frame_control);
    //use default duration from standard
    construct_dbyte(0xFF, 0xFF, &duration);

    //read the psdu from stdin
    while (fgets(msdu, 1000, stdin) != NULL) {

        int rb = strlen(msdu);

        //generate custom mac header
        header = generate_mac_header(frame_control, duration, "aa:bb:cc:dd:ee:ff", "aa:bb:cc:dd:ee:ff", "aa:bb:cc:aa:bb:cc", (char)sequence_number);

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
            float val;
            val = mod_samples[i][0];
            fwrite(&val, sizeof(float), 1, f);
            val = mod_samples[i][1];
            fwrite(&val, sizeof(float), 1, f);
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

    }

    fclose(f);

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
