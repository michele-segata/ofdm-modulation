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

#include "ofdm_utils.h"
#include "bit_utils.h"

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

void convolutional_encoding(const char *in, char *out_a, char *out_b, int size) {

    //TODO: understand

    //register of the encoder. only the first six bits are used
    char encoder_register = 0x00;

    int i, ib;

    for (i = 0; i < size; i++) {

        for (ib = 7; ib >= 0; ib--) {



        }

    }

}
#include <stdio.h>
void pucturing(const char *in_a, const char *in_b, char *out, int size, enum CODING_RATE rate) {

    int i, ib;
    //size of the out array
    int out_size;

    switch (rate) {
    case RATE_1_2:
        out_size = size * 2;
        break;
    case RATE_2_3:
        out_size = size * 3 / 2;
        break;
    case RATE_3_4:
        out_size = size * 4 / 3;
        break;
    default:
        //TODO add assert
        return;
        break;
    }

    for (i = 0; i < size; i++) {

        for (ib = 7; ib >= 0; ib--) {

            //coding rate 1/2 just takes all the bits of the input

            int bit_a = get_bit(in_a[i], ib);
            int bit_b = get_bit(in_b[i], ib);
            //printf("i0 = %d, i = %d, ib1 = %d, ib2 = %d\n", i, i * 2  + ib * 2 / 8, ib * 2 % 8, ib * 2 % 8 + 1);
            set_bit(&out[i * 2 + ib * 2 / 8], ib * 2 % 8 + 1, bit_a);
            set_bit(&out[i * 2 + ib * 2 / 8], ib * 2 % 8, bit_b);

        }

    }

}
