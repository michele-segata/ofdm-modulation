#include <fftw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 64

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef DWORD
typedef unsigned int DWORD;
#endif


double shortSymbol[][2] = {{0, 0}, {0, 0}, {1,1}, {0, 0}, {0, 0}, {0, 0}, {-1,-1}, {0, 0}, {0, 0}, {0, 0}, {1,1}, {0, 0}, {0, 0}, {0, 0}, {-1,-1}, {0, 0}, {0, 0}, {0, 0}, {-1,-1}, {0, 0}, {0, 0}, {0, 0}, {1,1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {-1,-1}, {0, 0}, {0, 0}, {0, 0}, {-1,-1}, {0, 0}, {0, 0}, {0, 0}, {1,1}, {0, 0}, {0, 0}, {0, 0}, {1,1}, {0, 0}, {0, 0}, {0, 0}, {1,1}, {0, 0}, {0, 0}, {0, 0}, {1,1}, {0, 0}, {0, 0}};
//double shortSymbol[][2] = {{1, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

double longSymbol[][2] = {{1, 0}, {1, 0}, {-1, 0}, {-1, 0}, {1, 0}, {1, 0}, {-1, 0}, {1, 0}, {-1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {-1, 0}, {-1, 0}, {1, 0}, {1, 0}, {-1, 0}, {1, 0}, {-1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {0, 0}, {1, 0}, {-1, 0}, {-1, 0}, {1, 0}, {1, 0}, {-1, 0}, {1, 0}, {-1, 0}, {1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {1, 0}, {1, 0}, {-1, 0}, {-1, 0}, {1, 0}, {-1, 0}, {1, 0}, {-1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}};

uint32_t get_fft_size(uint32_t samplingRate, double *minFreq, double *maxFreq);
void samples_to_complex(double *samples, uint32_t size, fftw_complex *complex);
void map_ofdm_to_ifft(double ofdm[][2], fftw_complex *ifft);
void multiply_by(fftw_complex *x, int size, double value);
void window_function(fftw_complex *x, int size);

int main(int argc, char **argv) {

    uint32_t nSamples;
    
    uint32_t fftSize = 64;
    double outSamples[64];

    fftw_complex *in, *out;
    fftw_plan p, p2;
    
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fftSize);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fftSize);
    p = fftw_plan_dft_1d(fftSize, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
    p2 = fftw_plan_dft_c2r_1d(fftSize, in, outSamples, FFTW_ESTIMATE);

    //memcpy(in, shortSymbol, sizeof(fftw_complex) * fftSize);
    map_ofdm_to_ifft(shortSymbol, in);
    multiply_by(in, fftSize, sqrt(13.0/6.0));

    fftw_execute(p);
    fftw_execute(p2);

    double normFactor = 64.0;
    multiply_by(out, fftSize, 1.0/normFactor);

    

    int i;

    fftw_complex *shortTrainingSeq = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * 161);
    for (i = 0; i < 161; i++) {
      shortTrainingSeq[i][0] = out[i % 64][0];  
      shortTrainingSeq[i][1] = out[i % 64][1];  
    }
    window_function(shortTrainingSeq, 161);

    //generate long training sequence
    map_ofdm_to_ifft(longSymbol, in);

    fftw_execute(p);

    multiply_by(out, fftSize, 1.0/normFactor);

    fftw_complex *longTrainingSeq = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * 161);
    //start from sample 32. first 32 symbols are the cyclic extension
    for (i = 32; i < 161; i++) {
      longTrainingSeq[i][0] = out[(i-32) % 64][0];  
      longTrainingSeq[i][1] = out[(i-32) % 64][1];  
    }
    //insert cyclic extension
    for (i = 0; i < 32; i++) {
      longTrainingSeq[i][0] = out[(i+32) % 64][0];  
      longTrainingSeq[i][1] = out[(i+32) % 64][1];  
    }
    window_function(longTrainingSeq, 161);

    for (i = 0; i < 321; i++) {

      if (i < 159) {
        //short training seq
        printf("%d %f %f \n", i, shortTrainingSeq[i][0], 0.0);
      }
      if (i == 160) {
        //overlap between short and long training seq
        printf("%d %f %f \n", i, shortTrainingSeq[i][0] + longTrainingSeq[i-160][0], 0.0);
        
      }
      if (i > 160 && i < 321) {
        printf("%d %f %f\n", i, longTrainingSeq[i-160][0], 0.0);
      }
        //printf("%d %f %f\n", i, outSamples[i], outSamples[i]);
    }


    fftw_destroy_plan(p);
    
    fftw_free(shortTrainingSeq);
    fftw_free(in);
    fftw_free(out);
    
    return 0;

}

void multiply_by(fftw_complex *x, int size, double value) {

    int i;
    for (i = 0; i < size; i++) {
        x[i][0] *= value;  
        x[i][1] *= value;  
    }
  
}

void map_ofdm_to_ifft(double ofdm[][2], fftw_complex *ifft) {

  // data(0) should be input(26) -offset 0- which is 0 so we leave as is
  // data(1..26) should be input elements(27-53) -offset 1..26-
  //data.replace_mid (1, input.mid (27, 26));
  // data (27..37) should be zero so we leave them as is
  // data (38..63) should be input 0 to 25
  //data.replace_mid (38, input.mid (0, 26));

    int i;
    ifft[0][0] = 0;
    ifft[0][1] = 0;

    for (i = 1; i <= 26; i++) {
        ifft[i][0] = ofdm[i+26][0];  
        ifft[i][1] = ofdm[i+26][1];  
    }

    for (i = 27; i <= 37; i++) {
        ifft[i][0] = 0;
        ifft[i][1] = 0;
    }

    for (i = 38; i <= 63; i++) {
        ifft[i][0] = ofdm[i-38][0];  
        ifft[i][1] = ofdm[i-38][1];  
    }

//    for (i = 27; i <= 37; i++) {
//        ifft[i][0] = 0;
//        ifft[i][1] = 0;
//    }
//
//    for (i = 0; i < 26; i++) {
//        ifft[i+1][0] = ofdm[i][0];  
//        ifft[i+1][1] = ofdm[i][1];  
//    }
//
//    for (i = 26; i < 52; i++) {
//        ifft[i+12][0] = ofdm[i][0];  
//        ifft[i+12][1] = ofdm[i][1];  
//    }
}

void samples_to_complex(double *samples, uint32_t size, fftw_complex *complex) {

    uint32_t i;
    
    for (i = 0; i < size; i++) {
        
        complex[i][0] = samples[i] / 32768.0;
        complex[i][1] = 0;
        
    }
    
}


void print_complex(fftw_complex *c, int size, int divide) {

    int i;

    //printf("[");

    for (i = 0; i < size; i++) {

        printf("%4.2f ", (c[i][1] < 0 ? -1 : 1) * c[i][1] / (divide ? size : 1));//, c[i][1] / (divide ? size : 1));

    }

    printf("\n");

}

void write_wav_sample(FILE *f, fftw_complex *fft_data, int index, int size, short bits) {

    int i, data_index;

    short sample;

    for (i = 0; i < size; i++) {

        sample = (int) fft_data[i][0];
        fwrite(&sample, bits / 8, 1, f);

    }

}

void copy_wav_sample(BYTE *wav_data, fftw_complex *fft_data, int index, int size, short bits) {

    int i, data_index;

    short sample;

    for (i = 0; i < size; i++) {

        data_index = index * size * bits / 8;

        sample = 0;

        sample = wav_data[data_index + i];
        if (bits == 16)
            sample |= wav_data[data_index + i + 1] << 8;

        fft_data[i][0] = sample;
        fft_data[i][1] = 0;

    }

}

void window_function(fftw_complex *x, int size) {

  if (size != 161) {
    fprintf(stderr, "window function: size != 161\n");
    exit(0);
  }

  x[0][0] *= 0.5;
  x[0][1] *= 0.5;
  x[size-1][0] *= 0.5;
  x[size-1][1] *= 0.5;

}
