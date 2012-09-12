/*
 * Copyright (c) 2012 Michele Segata
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Michele Segata <michele.segata@uibk.ac.at>
 * Description: utilities for OFDM
 *
 */

#include <assert.h>
#include <stdlib.h>

#include "ofdm_utils.h"
#include "bit_utils.h"

int max(int a, int b) {
    return a > b ? a : b;
}

void scramble(const char *in, char *out, int size) {
    scramble_with_initial_state(in, out, size, 0x7F);
}

void scramble_with_initial_state(const char *in, char *out, int size, char initial_state) {

    //indexes for each byte and for each bit of the input
    int i, ib;
    //the bit which is xored with the input bit
    char scrambling_bit;
    //the OFDM scrambler register (state of the scrambler)
    char shift_register = initial_state;

    for (i = 0; i < size; i++) {

        out[i] = 0;

        //cycle from 7 to 0: bits must enter ordered from MSB to LSB
        for (ib = 7; ib >= 0; ib--) {

            //xor 7th bit with 4th bit
            scrambling_bit = get_bit(shift_register, 3) ^ get_bit(shift_register, 6);
            //input bit
            char in_bit = get_bit(in[i], ib);
            //output bit is the xor of input bit and the scrambling bit
            set_bit(&out[i], ib, scrambling_bit ^ in_bit);

            //now shift the register left
            shift_register = (shift_register << 1) | scrambling_bit;

        }

    }

}

void convolutional_encoding(const char *in, char *out, int size) {

    //generator polinomials, as defined in 17.3.5.5
    char g_0 = 0x5B; //133 base 8
    char g_1 = 0x79; //171 base 8

    //register of the encoder. only the first seven bits are used
    char encoder_register = 0x00;

    //index of the input bit we are currently considering (i = 0..8*size-1)
    int i;
    //index of the output bit we are currently considering
    int o;
    //indexes of the byte and the bit inside the input byte we are currently considering
    int i_in, ib;
    //indexes of the byte the the bit inside the output byte we are currently considering
    int i_o, ib_o = 0;

    for (i = 0; i < size * 8; i++) {

        //compute the index for current byte
        i_in = i / 8;
        //compute the index for current bit inside current byte
        ib = 7 - i % 8;

        //insert one bit from input
        set_bit(&encoder_register, 6, get_bit(in[i_in], ib));

        //compute the index for current output byte
        o = 2 * i;
        i_o = o / 8;
        //compute the index for current bit inside current output byte
        ib_o = 7 - o % 8;
        //compute the output of the first polynomial generator
        set_bit(&out[i_o], ib_o, get_polynomial(encoder_register, g_0, 7));

        //o = 2 * i + 1
        o++;
        //compute the index for current output byte
        i_o = o / 8;
        //compute the index for current bit inside current output byte
        ib_o = 7 - o % 8;
        //compute the output of the second polynomial generator
        set_bit(&out[i_o], ib_o, get_polynomial(encoder_register, g_1, 7));

        //shift the register
        encoder_register = encoder_register >> 1;

    }

}

void pucturing(const char *in, char *out, int size, enum CODING_RATE rate) {

    //index of current input bit
    int i;
    //index of current output bit
    int i_o = 0;
    //variable for containing the modulo result
    int mod;
    //should we drop current bit or not?
    int drop;

    //cycle through all input bits
    for (i = 0; i < size * 8; i++) {

        //by default, bit must not be drop
        drop = 0;

        switch (rate) {
        case RATE_3_4:

            //period of code rate 3/4 is 6
            mod = i % 6;
            //and bits 3 and 4 must be dropped
            if (mod == 3 || mod == 4) {
                drop = 1;
            }
            break;

        case RATE_2_3:

            //period of code rate 2/3 is 4
            //and bit 3 must be dropped
            if (i % 4 == 3) {
                drop = 1;
            }
            break;

        default:
            assert(0);
            break;
        }

        if (!drop) {
            //if we should not drop, then just set output bit
            set_bit(&out[i_o / 8], 7 - i_o % 8, get_bit(in[i / 8], 7 - i % 8));
            //and move to next output bit
            i_o++;
        }

    }

}

void interleave(const char *in, char *out, int size, int n_cbps, int n_bpsc) {

    //index for the input bits
    int k;
    //index for the output bits after the first permutation
    int i, out_i;
    //index for the output bits after the second permutation
    int j, out_j;
    //helper index
    int h;
    //index of the current ofdm symbol
    int s;
    //value used for computing the second permutation
    int sp;
    //temporary storage for first permutation output
    char *first_perm;

    first_perm = (char *) calloc(sizeof(char), size);

    sp = max(n_bpsc / 2, 1);

    //run through all input bits and do the first permutation
    for (h = 0; h < size * 8; h++) {

        //compute the index of the current ofdm symbol
        s = h / n_cbps;
        //compute the index of the bit inside the current OFDM symbol
        k = h % n_cbps;

        //compute the index for the first permutation
        i = (n_cbps / 16) * (k % 16) + ((int) (k / 16));
        //i is the index w.r.t to current OFDM output symbol, but we have to put
        //the bit into the output array
        out_i = s * n_cbps + i;

        //now copy the bit
        set_bit(&first_perm[out_i / 8], 7 - out_i % 8, get_bit(in[h / 8], 7 - h % 8));

    }

    //run through all bits of the first permutation and do the second permutation
    for (h = 0; h < size * 8; h++) {

        //compute the index of the current ofdm symbol
        s = h / n_cbps;
        //compute the index of the bit inside the current OFDM symbol
        i = h % n_cbps;

        //compute the index for the first permutation
        j = sp * ((int) (i / sp)) + (i + n_cbps - ((int) (16 * i / n_cbps))) % sp;
        //j is the index w.r.t to current OFDM output symbol, but we have to put
        //the bit into the output array
        out_j = s * n_cbps + j;

        //now copy the bit
        set_bit(&out[out_j / 8], 7 - out_j % 8, get_bit(first_perm[h / 8], 7 - h % 8));

    }

    free(first_perm);

}

void modulate(const char *in, int size, enum DATA_RATE data_rate, fftw_complex *out) {

    struct OFDM_PARAMETERS p = get_ofdm_parameter(data_rate);

    int i;
    int idx;

    for (i = 0; i < size * 8; i += p.n_bpsc) {

        char in_i, in_q;

        in_i = get_bit_group_value(in, size, i, max(p.n_bpsc / 2, 1));
        in_q = get_bit_group_value(in, size, i + p.n_bpsc / 2, max(p.n_bpsc / 2, 1));

        idx = i / p.n_bpsc;

        switch (p.modulation) {

        case BPSK:

            assert(0 <= in_i && in_i <= 1);
            assert(0 <= in_q && in_q <= 1);

            out[idx][0] = bpsk_i[in_i];
            out[idx][1] = bpsk_q[in_q];
            break;

        case QPSK:

            assert(0 <= in_i && in_i <= 3);
            assert(0 <= in_q && in_q <= 3);

            out[idx][0] = qpsk_i[in_i];
            out[idx][1] = qpsk_q[in_q];
            break;

        case QAM16:

            assert(0 <= in_i && in_i <= 15);
            assert(0 <= in_q && in_q <= 15);

            out[idx][0] = qam16_i[in_i];
            out[idx][1] = qam16_q[in_q];
            break;

        case QAM64:

            assert(0 <= in_i && in_i <= 63);
            assert(0 <= in_q && in_q <= 63);

            out[idx][0] = qam64_i[in_i];
            out[idx][1] = qam64_q[in_q];
            break;

        }

    }

}

void insert_pilots(fftw_complex *in, fftw_complex *out, int symbol_index) {

    int i;

    //polarity of pilot subcarriers for current symbol
    int polarity = subcarrier_polarities[symbol_index];

    //on the standard, pilots are mapped inserted into positions -21, -7, 7, 21
    //using a 0-based array, means we have to put them into positions 5, 19, 33, 47
    out[5][0] = polarity;
    out[5][1] = 0;
    out[19][0] = polarity;
    out[19][1] = 0;
    out[33][0] = polarity;
    out[33][1] = 0;
    out[47][0] = -polarity;
    out[47][1] = 0;
    //insert also DC subcarrier
    out[26][0] = 0;
    out[26][1] = 0;

    //now insert modulated I,Q values (802.11-2007, 17-23)
    for (i = 0; i < 48; i++) {

        if (0 <= i && i <= 4) {
            out[i][0] = in[i][0];
            out[i][1] = in[i][1];
            continue;
        }
        if (5 <= i && i <= 17) {
            out[i + 1][0] = in[i][0];
            out[i + 1][1] = in[i][1];
            continue;
        }
        if (18 <= i && i <= 23) {
            out[i + 2][0] = in[i][0];
            out[i + 2][1] = in[i][1];
            continue;
        }
        if (24 <= i && i <= 29) {
            out[i + 3][0] = in[i][0];
            out[i + 3][1] = in[i][1];
            continue;
        }
        if (30 <= i && i <= 42) {
            out[i + 4][0] = in[i][0];
            out[i + 4][1] = in[i][1];
            continue;
        }
        if (43 <= i && i <= 47) {
            out[i + 5][0] = in[i][0];
            out[i + 5][1] = in[i][1];
        }

    }

}

struct OFDM_PARAMETERS get_ofdm_parameter(enum DATA_RATE data_rate) {

    struct OFDM_PARAMETERS p;

    p.data_rate = data_rate;

    switch (data_rate) {

    case BW_20_DR_6_MBPS:
    case BW_10_DR_3_MBPS:

        p.modulation = BPSK;
        p.coding_rate = RATE_1_2;
        p.n_bpsc = 1;
        p.n_cbps = 48;
        p.n_dbps = 24;
        p.signal_rate = 0x0D; //0b00001101

        break;

    case BW_20_DR_9_MBPS:
    case BW_10_DR_4_5_MBPS:

        p.modulation = BPSK;
        p.coding_rate = RATE_3_4;
        p.n_bpsc = 1;
        p.n_cbps = 48;
        p.n_dbps = 36;
        p.signal_rate = 0x0F; //0b00001111;

        break;

    case BW_20_DR_12_MBPS:
    case BW_10_DR_6_MBPS:

        p.modulation = QPSK;
        p.coding_rate = RATE_1_2;
        p.n_bpsc = 2;
        p.n_cbps = 96;
        p.n_dbps = 48;
        p.signal_rate = 0x05; //0b00000101;

        break;

    case BW_20_DR_18_MBPS:
    case BW_10_DR_9_MBPS:

        p.modulation = QPSK;
        p.coding_rate = RATE_3_4;
        p.n_bpsc = 2;
        p.n_cbps = 96;
        p.n_dbps = 72;
        p.signal_rate = 0x07; //0b00000111;

        break;

    case BW_20_DR_24_MBPS:
    case BW_10_DR_12_MBPS:

        p.modulation = QAM16;
        p.coding_rate = RATE_1_2;
        p.n_bpsc = 4;
        p.n_cbps = 192;
        p.n_dbps = 96;
        p.signal_rate = 0x09; //0b00001001;

        break;

    case BW_20_DR_36_MBPS:
    case BW_10_DR_18_MBPS:

        p.modulation = QAM16;
        p.coding_rate = RATE_3_4;
        p.n_bpsc = 4;
        p.n_cbps = 192;
        p.n_dbps = 144;
        p.signal_rate = 0x0B; //0b00001011;

        break;

    case BW_20_DR_48_MBPS:
    case BW_10_DR_24_MBPS:

        p.modulation = QAM64;
        p.coding_rate = RATE_2_3;
        p.n_bpsc = 6;
        p.n_cbps = 288;
        p.n_dbps = 192;
        p.signal_rate = 0x01; //0b00000001;

        break;

    case BW_20_DR_54_MBPS:
    case BW_10_DR_27_MBPS:

        p.modulation = QAM64;
        p.coding_rate = RATE_3_4;
        p.n_bpsc = 6;
        p.n_cbps = 288;
        p.n_dbps = 216;
        p.signal_rate = 0x03; //0b00000011;

        break;

    default:

        assert(0);

        break;

    }

    return p;

}

void map_ofdm_to_ifft(fftw_complex *ofdm, fftw_complex *ifft) {

    // out 0        must be 0
    // out 1->26    must be 1->26, base 0 means 27->53
    // out 27->37   must be 0
    // out 38->63   must be -26->-1 base 0 means 0->25

    int i;
    ifft[0][0] = 0;
    ifft[0][1] = 0;

    for (i = 1; i <= 26; i++) {
        ifft[i][0] = ofdm[i + 26][0];
        ifft[i][1] = ofdm[i + 26][1];
    }

    for (i = 27; i <= 37; i++) {
        ifft[i][0] = 0;
        ifft[i][1] = 0;
    }

    for (i = 38; i <= 63; i++) {
        ifft[i][0] = ofdm[i - 38][0];
        ifft[i][1] = ofdm[i - 38][1];
    }

}
