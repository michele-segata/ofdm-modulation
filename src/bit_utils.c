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
#include <assert.h>

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

void print_complex_array(fftw_complex *c, int size) {
    int i;
    for (i = 0; i < size; i++) {
        printf("(%.3f, %.3f)", c[i][0], c[i][1]);
        if (i != size - 1) {
            printf(" ");
        }
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

int read_bits_from_file(const char *filename, char *bytes, int size) {

    //number of read bytes
    int bytes_read = 0;
    //index of the current bit
    int i = 7;
    //character read from the file
    char in;
    //found a comment, should skip till end of line
    int comment = 0;
    //file to be read
    FILE *f = fopen(filename, "r");

    if (!f) {
        return ERR_CANNOT_READ_FILE;
    }

    while (fscanf(f, "%c", &in) != EOF) {

        //we have no enough space for new data. so return what we
        //have read until now
        if (bytes_read == size) {
            break;
        }

        //do all checks only if we are not in comment.
        //if we are in comment just skip the character
        //but after checking that it is not a newline
        if (!comment) {

            switch (in) {

            case '1':
            case '0':

                //write bit to array
                set_bit(&bytes[bytes_read], i, in == '1' ? 1 : 0);

                //move index bit
                i--;
                //are we at the end of a byte?
                if (i == -1) {
                    //if so, move to next byte
                    bytes_read++;
                    i = 7;
                }

                break;

            case ' ':

                if (i != 7) {
                    //we found a space in the middle of a bit sequence
                    //wrong format
                    fclose(f);
                    return ERR_INVALID_FORMAT;
                }

                break;

            case '#':

                if (i != 7) {
                    //we found a comment character in the middle of a bit sequence
                    //wrong format
                    fclose(f);
                    return ERR_INVALID_FORMAT;
                }

                //set comment mode
                comment = 1;

                break;

            case '\n':

                if (i != 7) {
                    //line breaks before ending a bit sequence
                    //wrong format
                    fclose(f);
                    return ERR_INVALID_FORMAT;
                }

                break;

            }

        } else {
            //if we are in comment mode and the character is \n
            //just exit from comment mode
            if (in == '\n') {
                comment = 0;
            }
        }

    }

    fclose(f);

    return bytes_read;

}

char get_bit_group_value(const char *bytes, int size, int a, int length) {

    //index for getting bits
    int i;
    //output value
    char out = 0;

    for (i = a; i < a + length; i++) {

        int bit = get_ith_bit(bytes, i);
        set_bit(&out, length - (i - a) - 1, bit);

    }

    return out;

}

int get_ith_bit(const char *b, int i) {

    //index of the byte
    int i_byte;
    //index of the bit inside the byte
    int i_bit;

    i_byte = i / 8;
    i_bit = 7 - i % 8;

    return get_bit(b[i_byte], i_bit);

}
