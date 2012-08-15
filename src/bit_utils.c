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

#include "bit_utils.h"

#include <stdio.h>

void print_bits(char b) {
    int i;
    for (i = 7; i >= 0; i--) {
        printf("%d", get_bit(b, i));
    }
}

void print_bits_array(const char *b, int size) {
    int i;
    for (i = 0; i < size; i++) {

    }
}

inline int get_bit(char b, int i) {
    return (b & (1 << i) ? 1 : 0);
}

inline void set_bit(char *b, int i, int bit) {
    if (bit) {
        *b |= 1 << i;
    } else {
        *b &= ~(1 << i);
    }
}

char get_polynomial(char reg, char generator, int size) {

    int i;
    char out = 0;

    for (i = 0; i < size; i++) {
        if (get_bit(generator, i)) {
            out ^= get_bit(reg, i);
        }
    }
    return out;

}
