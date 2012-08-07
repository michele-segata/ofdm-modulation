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
    char shiftRegister = initial_state;

    for (i = 0; i < size; i++) {

        out[i] = 0;

        //cycle from 7 to 0: bits must enter ordered from MSB to LSB
        for (ib = 7; ib >= 0; ib--) {

            //xor 7th bit with 4th bit
            scrambling_bit = get_bit(shiftRegister, 3) ^ get_bit(shiftRegister, 6);
            //input bit
            char in_bit = get_bit(in[i], ib);
            //output bit is the xor of input bit and the scrambling bit
            set_bit(&out[i], ib, scrambling_bit ^ in_bit);

            //now shift the register left
            shiftRegister = (shiftRegister << 1) | scrambling_bit;

        }

    }

}
