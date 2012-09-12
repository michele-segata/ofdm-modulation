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

    fftw_complex *ifft = fftw_alloc_complex(64);
    fftw_complex *mod = fftw_alloc_complex(48);
    fftw_complex *pil = fftw_alloc_complex(53);
    fftw_complex *time = fftw_alloc_complex(64);
    fftw_complex *ext = fftw_alloc_complex(81);
    fftw_complex *signal = fftw_alloc_complex(81);

    printf("Read %d bytes = %d bits\n", read_bytes, read_bytes * 8);

    printf("Input bit sequence:\n");
    print_bits_array(in, SIZE);
    printf("\n");

    scramble_with_initial_state(in, out, SIZE, 0x5D);

    printf("Scrambled bit sequence:\n");
    print_bits_array(out, SIZE);
    printf("\n");

    convolutional_encoding(out, tmp, SIZE);
    printf("Encoded bit sequence rate 1/2:\n");
    print_bits_array(tmp, SIZE * 2);
    printf("\n");

    pucturing(tmp, out, SIZE * 2, params.coding_rate);

    printf("Punctured bit sequence rate 3/4:\n");
    print_bits_array(out, SIZE * 4 / 3);
    printf("\n");

    interleave(out, tmp, SIZE * 2 * 2 / 3, params.n_cbps, params.n_bpsc);

    printf("Interleaved bit sequence:\n");
    print_bits_array(tmp, SIZE * 4 / 3);
    printf("\n");

    modulate(tmp, SIZE * 2 * 2 / 3, params.data_rate, mod);

    printf("Modulated bits (QAM16):\n");
    print_complex_array(mod, 48);
    printf("\n");

    insert_pilots(mod, pil, 1);

    printf("Pilot insertion:\n");
    print_complex_array(pil, 53);
    printf("\n");

    map_ofdm_to_ifft(pil, ifft);

    printf("IFFT input:\n");
    print_complex_array(ifft, 64);
    printf("\n");

    perform_ifft(ifft, time);
    normalize_ifft_output(time, 64, 64);

    printf("Time samples:\n");
    print_complex_array(time, 64);
    printf("\n");

    add_cyclic_prefix(time, 64, ext, 81, 16);
    apply_window_function(ext, 81);

    printf("Time samples i (cyclically extended):\n");
    print_complex_array(ext, 81);
    printf("\n");

    generate_signal_field(signal, BW_20_DR_36_MBPS, 100);

    printf("SIGNAL header time samples:\n");
    print_complex_array(signal, 81);
    printf("\n");

    fftw_free(mod);
    fftw_free(pil);
    fftw_free(ifft);
    fftw_free(time);
    fftw_free(ext);
    fftw_free(signal);

    return 0;

}
