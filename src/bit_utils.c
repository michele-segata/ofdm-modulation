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

char change_endianness(char b) {

    char o;
    int i;

    for (i = 0; i <= 7; i++) {
        set_bit(&o, 7 - i, get_bit(b, i));
    }
    return o;

}

void change_array_endianness(const char *in, int size, char *out) {

    int i;

    for (i = 0; i < size; i++) {
        out[i] = change_endianness(in[i]);
    }

}

void print_bits(char b) {
    int i;
    for (i = 7; i >= 0; i--) {
        printf("%d", get_bit(b, i));
    }
}

void print_bits_array(const char *b, int size, char separator) {
    int i;
    for (i = 0; i < size; i++) {
        print_bits(b[i]);
        printf("%c", separator);
    }
}

void print_hex_array(const char *b, int size, char separator) {
    int i;
    for (i = 0; i < size; i++) {
        printf("%02x", (unsigned char)b[i]);
        if (i != size - 1) {
            printf("%c", separator);
        }
        fflush(stdout);
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

int read_hex_from_file(const char *filename, char *bytes, int size) {

    //number of read bytes
    int bytes_read = 0;
    //we are reading the first four bits (the first hex digit)
    int first_digit = 1;
    //current hex we are reading
    char hex[3];
    //parsed value
    unsigned int value;
    //character read from the file
    char in;
    //found a comment, should skip till end of line
    int comment = 0;
    //file to be read
    FILE *f = fopen(filename, "r");

    if (!f) {
        return ERR_CANNOT_READ_FILE;
    }

    hex[2] = '\0';

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

            case ' ':

                if (!first_digit) {
                    //we found a space in the middle of a hex value
                    //wrong format
                    fclose(f);
                    return ERR_INVALID_FORMAT;
                }

                break;

            case '#':

                if (!first_digit) {
                    //we found a comment character in the middle of a hex value
                    //wrong format
                    fclose(f);
                    return ERR_INVALID_FORMAT;
                }

                //set comment mode
                comment = 1;

                break;

            case '\n':

                if (!first_digit) {
                    //line breaks before ending a hex value
                    //wrong format
                    fclose(f);
                    return ERR_INVALID_FORMAT;
                }

                break;

            default:

                //check validity of the character
                if ((in >= '0' && in <= '9') || (in >= 'a' && in <= 'f') || (in >= 'A' && in <= 'F')) {
                    if (first_digit) {
                        //if it is the first digit just save it
                        hex[0] = in;
                        first_digit = 0;
                    } else {
                        //otherwise parse the value and increment number of bytes read
                        hex[1] = in;
                        first_digit = 1;
                        sscanf(hex, "%x", &value);
                        bytes[bytes_read] = (char)value;
                        bytes_read++;
                    }
                }
                else {
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

int compute_even_parity(const char *in, int base, int length) {

    //number of bits set to 1
    int count = 0;
    int i;

    for (i = 0; i < length; i++) {
        if (get_ith_bit(in, i + base)) {
            count++;
        }
    }

    return count % 2;

}
