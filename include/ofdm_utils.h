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

#ifndef _OFDM_UTILS_H_
#define _OFDM_UTILS_H_

/**
 * Define coding rates for the puncturing function
 */
enum CODING_RATE {
    RATE_1_2, //r=1/2
    RATE_2_3, //r=2/3
    RATE_3_4  //r=3/4
};

/**
 * Perform the scrambling of a set of bytes, as mandated by
 * 802.11-2007, 17.3.5.4.
 * The initial state of the register is set to all ones.
 * For a different initial state see scramble_with_initial_state
 *
 * \param in array of bytes to be scrambled
 * \param out array of bytes where to write scrambled bits
 * \param size size of in and out arrays
 */
void scramble(const char *in, char *out, int size);

/**
 * Perform the scrambling of a set of bytes, as mandated by
 * 802.11-2007, 17.3.5.4.
 *
 * \param in array of bytes to be scrambled
 * \param out array of bytes where to write scrambled bits
 * \param size size of in and out arrays
 * \param initial_state initial state of the 7-bit shift register
 */
void scramble_with_initial_state(const char *in, char *out, int size, char initial_state);

/**
 * Perform the convolutional encoding of a set of bytes, as
 * mandated by 802.11-2007, 17.3.5.5.
 * The encoder uses a coding rate of 1/2 and outputs the
 * first set of bits into out_a and the second set of bits
 * into out_b. Such arrays must be passed to the punturing
 * function, to finally obtain the bits encoded with the
 * desired coding rate, i.e., 1/2, 2/3 or 3/4.
 *
 * \param in array of bytes to be encoded
 * \param out_a array of bytes where to write first bits of encoded data
 * \param out_b array of bytes where to write second bits of encoded data
 * \param size of the input array. Output array size is twice
 * as much
 */
void convolutional_encoding(const char *in, char *out_a, char *out_b, int size);

/**
 * Perform the puncturing function on the output of the
 * convolutional encoder, in order to obtain the data bits
 * encoded with the desired coding rate.
 *
 * \param in_a array of the first bits output by the convolutional encoder
 * \param in_b array of the first bits output by the convolutional encoder
 * \param out array of bytes where to store punctured bytes
 * \param size size of the in_a array (equal to in_b array). Notice that
 * the size of the out array depends on the coding rate r. For r=1/2
 * it is size * 2, for r=2/3 it is size * 3/2, for r=3/4 it is size * 4/3
 * \param rate coding rate (i.e., 1/2, 2/3 or 3/4)
 */
void pucturing(const char *in_a, const char *in_b, char *out, int size, enum CODING_RATE rate);

#endif
