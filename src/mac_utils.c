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

void str_to_mac_address(const char *mac, mac_address_t *addr) {

    unsigned int int_addr[6], i;
    sscanf(mac, "%x:%x:%x:%x:%x:%x", &int_addr[0], &int_addr[1], &int_addr[2], &int_addr[3], &int_addr[4], &int_addr[5]);
    for (i = 0; i < 6; i++)
        (*addr)[i] = (byte) int_addr[i];

}

void construct_dbyte(byte byte1, byte byte2, dbyte *value) {
    (*value)[0] = byte1;
    (*value)[1] = byte2;
}

int dbyte_equal(dbyte v1, byte byte1, byte byte2) {
    return ((v1[0] == byte1) && (v1[1] == byte2));
}

void dbyte_set(dbyte *a, dbyte b) {
    (*a)[0] = b[0];
    (*a)[1] = b[1];
}

struct MAC_DATAFRAME_HEADER generate_mac_header(dbyte frame_control, dbyte duration, const char *address1, const char *address2, const char *address3, byte sequence) {

    struct MAC_DATAFRAME_HEADER hdr;

    if (!dbyte_equal(frame_control, 0xFF, 0xFF)) {
        dbyte_set(&hdr.frame_control, frame_control);
    } else {
        construct_dbyte(0x04, 0x02, &hdr.frame_control);
    }

    if (!dbyte_equal(duration, 0xFF, 0xFF)) {
        dbyte_set(&hdr.duration, duration);
    } else {
        construct_dbyte(0x00, 0x2E, &hdr.duration);
    }

    if (address1) {
        str_to_mac_address(address1, &hdr.address1);
    } else {
        str_to_mac_address("00:60:08:cd:37:a6", &hdr.address1);
    }

    if (address2) {
        str_to_mac_address(address2, &hdr.address2);
    } else {
        str_to_mac_address("00:20:d6:01:3c:f1", &hdr.address2);
    }

    if (address3) {
        str_to_mac_address(address3, &hdr.address3);
    } else {
        str_to_mac_address("00:60:08:ad:3b:af", &hdr.address3);
    }

    char seq_lsbs = 0;
    char seq_msbs = 0;
    int i;
    for (i = 0; i < 4; i++) {
        set_bit(&seq_lsbs, i + 4, get_bit(sequence, i));
    }
    for (i = 0; i < 4; i++) {
        set_bit(&seq_msbs, i, get_bit(sequence, i + 4));
    }
    construct_dbyte(seq_lsbs, seq_msbs, &hdr.sequence);

    return hdr;

}

struct MAC_DATAFRAME_HEADER generate_default_mac_header() {

    struct MAC_DATAFRAME_HEADER hdr;

    //frame check sequence
    unsigned int fcs;

    dbyte frame_control;
    dbyte duration;

    //use values in the sample psdu of 802.11-2012
    construct_dbyte(0xFF, 0xFF, &frame_control);
    construct_dbyte(0xFF, 0xFF, &duration);

    hdr = generate_mac_header(frame_control, duration, 0, 0, 0, 0);

    return hdr;

}

void generate_mac_data_frame(const char *msdu, int msdu_size, struct MAC_DATAFRAME_HEADER header, char **psdu, int *psdu_size) {

    //frame check sequence
    unsigned int fcs;

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
