#include <stdio.h>
#include <stdlib.h>

#include <fftw3.h>

#include "ofdm_utils.h"
#include "bit_utils.h"

/**
 * This test application takes in input the interleaved bits of the 802.11-2012
 * standard (annex J), performs the OFDM mapping for the IFFT input and inserts the
 * pilots for the 2nd OFDM symbol (i.e., the first DATA symbol), printing as output
 * the frequency domain representation of the first OFDM DATA symbol. Output should
 * be checked against table L-20.
 */
int main(int argc, char **argv) {

	if (argc != 2) {
		printf("error: missing input file\n");
		return 1;
	}

	//interleaved psdu loaded from data file
	char interleaved_psdu[1000];
	//ofdm encoding parameters
	struct OFDM_PARAMETERS params = get_ofdm_parameter(BW_20_DR_36_MBPS);
	//frequency domain representation (48 subcarriers)
	fftw_complex *freq;
	//frequency domain with pilot inserted
	fftw_complex *pilot;

	//read the scrambled psdu from text file
	int rb = read_bits_from_file(argv[1], interleaved_psdu, 1000);

	if (rb == ERR_CANNOT_READ_FILE) {
		printf("Cannot read file \"%s\": file not found?\n", argv[1]);
		return 1;
	}
	if (rb == ERR_INVALID_FORMAT) {
		printf("Invalid file format\n");
		return 1;
	}

	//after interleaving, the size is the same after encoding
	freq = fftw_alloc_complex(N_DATA_SUBCARRIERS);
	pilot = fftw_alloc_complex(N_TOTAL_SUBCARRIERS);
	//perform ofdm mapping
	modulate(interleaved_psdu, rb, params.data_rate, freq);
	//insert pilot subcarriers. symbol index is 1, since symbol 0 is the SIGNAL header
	insert_pilots(freq, pilot, 1);

	//output and we're done
	int i, ifft_i;
	for (i = 0; i < FFT_SIZE; i++) {

		//convert the i index into carrier index
		ifft_i = i - 32;

		float iv, qv;
		//carriers from -32 to -27 and from 27 to 31 are set to 0
		if (ifft_i < -26 || ifft_i > 26) {
			iv = 0;
			qv = 0;
		}
		else {
			iv = (float)pilot[i - 6][0];
			qv = (float)pilot[i - 6][1];
		}
		printf("%d %.3f %.3f\n", ifft_i, iv, qv);

	}

	fftw_free(freq);
	fftw_free(pilot);

	return 0;

}
