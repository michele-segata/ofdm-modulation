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
 * Description: utilities for bit level functionalities
 *
 */

#ifndef _BIT_UTILS_H_
#define _BIT_UTILS_H_

/**
 * Print the bit string composing one byte
 *
 * \param b the byte to print
 */
void print_bits(char b);

/**
 * Print the bit string composing a byte array
 *
 * \param b the byte array to print
 * \param size the length of the array
 */
void print_bits_array(const char *b, int size);

/**
 * Returns the i-th bit of a byte.
 * NOTICE that the bit 0 is the LSB, while the
 * bit 7 is the MSB
 *
 * \param b the byte
 * \param i the bit index
 * \return value of the i-th bit of b
 */
inline int get_bit(char b, int i);

/**
 * Sets the i-th bit of a byte.
 * NOTICE that the bit 0 is the LSB, while the
 * bit 7 is the MSB
 *
 * \param b the byte
 * \param i the bit index
 * \param bit value of the bit to set
 */
inline void set_bit(char *b, int i, int bit);

#endif
