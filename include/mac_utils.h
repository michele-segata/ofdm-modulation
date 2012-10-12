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

#ifndef MAC_UTILS_H_
#define MAC_UTILS_H_

#include "bit_utils.h"

/**
 * Define a IEEE 802 48 bits MAC address
 */
typedef byte mac_address_t[6];

/**
 * Define type for two bytes elements of the mac
 * array
 */
typedef byte dbyte[2];

/**
 * Define structure for the MAC header of a data frame.
 * Values as stored as MSB, but standard mandates LSB
 * representation. Bit utils functions can help changing
 * endiannes
 */
struct MAC_DATAFRAME_HEADER {
    dbyte frame_control;        //protocol version, type, subtype, to_ds, from_ds, ...
    dbyte duration;             //duration field
    mac_address_t address1;  //address 1 indicating BSSID: 6 bytes MAC address
    mac_address_t address2;  //address 2 indicating receiver
    mac_address_t address3;  //address 3 indicating sender
    dbyte sequence;             //sequence number plus fragment number
};

/**
 * Converts a mac address string into a mac_address_t type
 *
 * \param mac the mac address string in "aa:bb:cc:dd:ee:ff" hex format
 * \param addr pointer to location where to store the converted mac address
 */
void str_to_mac_address(const char *mac, mac_address_t *addr);

/**
 * Given two bytes in the little endian format, constructs a dbyte element.
 * The bytes will be arranged like "byte1byte2"
 *
 * \param byte1 first byte
 * \param byte2 second byte
 * \param value pointer to location where to store the dbyte element
 */
void construct_dbyte(byte byte1, byte byte2, dbyte *value);

/**
 * Returns true if v1 is equal to v2, false otherwise.
 *
 * \param v1 first dbyte value
 * \param byte1 first byte of dbyte to check
 * \param byte2 second byte of dbyte to check
 * \return v1 == "byte1byte2"
 */
int dbyte_equal(dbyte v1, byte byte1, byte byte2);

/**
 * Given a set of parameters, generates a MAC frame header. This function
 * is not able to generate ANY MAC header, like beacons or ACKs, but generates
 * the header for data frames.
 * For setting dbyte parameters, see construct dbyte
 *
 * \param frame_control frame control field. set to 0xFFFF to use the default
 * value used in the 802.11-2012 sample encoding (0x0402)
 * \param duration duration field. set to 0xFFFF to use the default value of
 * 0x002e
 * \param address1 bssid address. set to null to use the default
 * value used in the 802.11-2012 sample encoding (00:60:08:cd:37:a6)
 * \param address2 receiver address. set to null to use the default
 * value used in the 802.11-2012 sample encoding (00:20:d6:01:3c:f1)
 * \param address3 sender address. set to null to use the default
 * value used in the 802.11-2012 sample encoding (00:60:08:ad:3b:af)
 * \param sequence sequence number
 * \return a MAC header with desired informations
 */
struct MAC_DATAFRAME_HEADER generate_mac_header(dbyte frame_control, dbyte duration, const char *address1, const char *address2, const char *address3, byte sequence);

/**
 * Generates a default MAC header. For default, it is intended the MAC header
 * of the sample PSDU in the 802.11-2012 standard, annex L
 *
 * \return a default mac header
 */
struct MAC_DATAFRAME_HEADER generate_default_mac_header();

/**
 * Given a payload and a MAC header, generates a MAC data frame (i.e., a PSDU)
 * to be given to the physical layer for encoding. The function composes
 * the header and the payload and appends the FCS
 *
 * \param msdu the payload for the MAC frame
 * \param msdu_size the size of the msdu in bytes
 * \param header the mac header (see generate_mac_header())
 * \param psdu pointer to a byte array where to store the MAC frame. Memory
 * will be alloced by the function
 * \param psdu_size pointer to an integer where the size of the psdu in bytes
 * will be stored
 */
void generate_mac_data_frame(const char *msdu, int msdu_size, struct MAC_DATAFRAME_HEADER header, char **psdu, int *psdu_size);

#endif /* MAC_UTILS_H_ */
