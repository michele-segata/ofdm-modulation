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

void pucturing(const char *in_a, const char *in_b, char *out, int size, enum CODING_RATE rate) {

    int i;
    //size of the out array
    int out_size;

    //iterators for in_a, in_b and out
    int i_a, i_b, i_o;
    /**
     * mod for in_a and in_b, i.e., when bit must be drop.
     * e.g.,
     * set to 0 = drop no bits
     * set to 1 = drop every bit
     * set to 2 = drop one every two
     */
    int mod_a, mod_b;
    /**
     * drop shifter for in_a and in_b. might be needed if you
     * want to drop one every two but start dropping from the
     * first
     */
    int shift_a, shift_b;

    switch (rate) {
    case RATE_1_2:

        mod_a = 0;
        mod_b = 0;
        shift_a = 0;
        shift_b = 0;

        out_size = size * 2;

        break;
    case RATE_2_3:

        //every bit from a
        mod_a = 0;
        //one bit every two from bit
        mod_b = 2;
        //no shifts
        shift_a = 0;
        //start dropping from second bit
        //1, 0, 1, 0, 1, 0
        shift_b = 1;

        out_size = size * 3 / 2;

        break;
    case RATE_3_4:

        //two bits every three from a
        mod_a = 3;
        //two bits every three from b
        mod_b = 3;
        //start dropping from the third
        //1, 2, 0, 1, 2, 0
        shift_a = 1;
        //start dropping from the second bit
        //2, 0, 1, 2, 0
        shift_b = 2;

        out_size = size * 4 / 3;

        break;
    default:
        assert(0);
        break;
    }

    i_a = 0;
    i_b = 0;
    i_o = 0;

    //cycle until we filled up the output array
    for (i = 0; i_b < size * 8; i++) {

        if (i % 2 == 0) {
            //we are considering bit of in_a
            if (mod_a == 0 || (i_a + shift_a) % mod_a != 0) {
                //bit must not be dropped
                //printf("out[%d][%d] = in_a[%d][%d]\n", i_o / 8, 7 - i_o % 8, i_a / 8, 7 - i_a % 8);
                set_bit(&out[i_o / 8], 7 - i_o % 8, get_bit(in_a[i_a / 8], 7 - (i_a % 8)));

                //bit not dropped, so jump to next output bit
                i_o++;
            }
            //increase index for in_a
            i_a++;
        } else {
            //we ar//we are considering bit of in_b
            if (mod_b == 0 || (i_b + shift_b) % mod_b != 0) {
                //bit must not be dropped
                //printf("out[%d][%d] = in_b[%d][%d]\n", i_o / 8, 7 - i_o % 8, i_b / 8, 7 - i_b % 8);
                set_bit(&out[i_o / 8], 7 - i_o % 8, get_bit(in_b[i_b / 8], 7 - (i_b % 8)));

                //bit not dropped, so jump to next output bit
                i_o++;
            }
            //increase index for in_b
            i_b++;
        }

    }

    //printf("ia=%d ib=%d io=%d size=%d outsize=%d\n", i_a, i_b, i_o, size * 8, out_size * 8);

    assert(i_a == size * 8);
    assert(i_b == size * 8);
    assert(i_o == out_size * 8);

}
