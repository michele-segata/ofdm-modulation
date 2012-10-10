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
 * Description:
 *
 */

#include "mac_utils.h"

#include <stdlib.h>
#include <string.h>

//copy-pasted crc32... TODO: change it if needed
unsigned int update_crc32(unsigned int crc, const char *data, size_t len) {
    int j;
    unsigned int byte, mask;
    static unsigned int table[256];
    /* Set up the table if necesary */
    if (table[1] == 0) {
        for (byte = 0; byte <= 255; byte++) {
            crc = byte;
            for (j = 7; j >= 0; j--) {
                mask = -(crc & 1);
                crc = (crc >> 1) ^ (0xEDB88320 & mask);
            }
            table[byte] = crc;
        }
    }

    /* Calculate the CRC32*/
    size_t i = 0;
    crc = 0xFFFFFFFF;
    for (i = 0; i < len; i++) {
        byte = data[i];    //Get next byte
        crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
    }
    unsigned int crc_reversed;
    crc_reversed = 0x00000000;
    for (j = 31; j >= 0; j--) {
        crc_reversed |= ((crc >> j) & 1) << (31 - j);
    }
    return crc;
}

unsigned int crc32(const char *buf, size_t len) {
    return update_crc32(0xffffffff, buf, len) ^ 0xffffffff;
}

//#define ENDLSS 1

#ifdef ENDLSS
#define ENDIANESS(x) change_endianless(x)
#else
#define ENDIANESS(x) x
#endif

void generate_mac_data_frame(const char *msdu, int msdu_size, char **psdu, int *psdu_size, char seq) {

    //mac frame
    struct MAC_DATAFRAME_HEADER header;
    //frame check sequence
    unsigned int fcs;

    header.frame_control[0] = ENDIANESS(0x08);//0x20; //00000100 00100000  indicate DATA frame (LOL, not), to_ds, from_ds, more frag, retry
    header.frame_control[1] = ENDIANESS(0x02);//0x40; //all other flags, unutilized //00000010 01000000

    header.duration[0] = ENDIANESS(0x00); //TODO: set accordingly to payload
    header.duration[1] = ENDIANESS(0x2e); //0x74; //2e = 00101110 -> 01110100 -> 74

    header.address1[0] = ENDIANESS(0x00);
    header.address1[1] = ENDIANESS(0x60); //change_endianless(0x60);
    header.address1[2] = ENDIANESS(0x08); //change_endianless(0x08);
    header.address1[3] = ENDIANESS(0xcd); //change_endianless(0xcd);
    header.address1[4] = ENDIANESS(0x37); //change_endianless(0x37);
    header.address1[5] = ENDIANESS(0xa6); //change_endianless(0xa6);

    header.address2[0] = ENDIANESS(0x00); //change_endianless(0x00);
    header.address2[1] = ENDIANESS(0x20); //change_endianless(0x20);
    header.address2[2] = ENDIANESS(0xd6); //change_endianless(0xd6);
    header.address2[3] = ENDIANESS(0x01); //change_endianless(0x01);
    header.address2[4] = ENDIANESS(0x3c); //change_endianless(0x3c);
    header.address2[5] = ENDIANESS(0xf1); //change_endianless(0xf1);

    header.address3[0] = ENDIANESS(0x00); //change_endianless(0x00);
    header.address3[1] = ENDIANESS(0x60); //change_endianless(0x60);
    header.address3[2] = ENDIANESS(0x08); //change_endianless(0x08);
    header.address3[3] = ENDIANESS(0xad); //change_endianless(0xad);
    header.address3[4] = ENDIANESS(0x3b); //change_endianless(0x3b);
    header.address3[5] = ENDIANESS(0xaf); //change_endianless(0xaf);

    //destination broadcast
//    header.address3[0] = 0xff;
//    header.address3[1] = 0xff;
//    header.address3[2] = 0xff;
//    header.address3[3] = 0xff;
//    header.address3[4] = 0xff;
//    header.address3[5] = 0xff;

    //init sequence number to 0
    //seq # first for bits (LSB)
    char seq_lsbs = 0;
    char seq_msbs = 0;
    int i;
    for (i = 0; i < 4; i++) {
        set_bit(&seq_lsbs, i+4, get_bit(seq, i));
    }
    for (i = 0; i < 4; i++) {
        set_bit(&seq_msbs, i, get_bit(seq, i + 4));
    }
    header.sequence[0] = ENDIANESS(seq_lsbs);
    header.sequence[1] = ENDIANESS(seq_msbs);

#ifdef ENDLSS
    change_array_endianless(msdu, msdu_size, msdu);
#endif

    //header size is 24, plus 4 for FCS means 28 bytes
    *psdu_size = 28 + msdu_size;
    *psdu = (char *) calloc(*psdu_size, sizeof(char));

    //copy mac header into psdu
    memcpy(*psdu, &header, 24);
    //copy msdu into psdu
    memcpy(*psdu + 24, msdu, msdu_size);
    //compute and store fcs
    fcs = crc32(*psdu, msdu_size + 24);
    memcpy(*psdu + msdu_size + 24, &fcs, sizeof(unsigned int));

}
