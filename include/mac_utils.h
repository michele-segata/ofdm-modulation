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

#include <stddef.h>

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
	mac_address_t address1;     //address 1
	mac_address_t address2;     //address 2
	mac_address_t address3;     //address 3
	dbyte sequence;             //sequence number plus fragment number
};

/**
 * Frame control field type. Notice that all field values defined for the MAC
 * frame control field, are meant as LSB -> MSB, so the opposite of what has
 * been defined in table 8-1 in 802.11-2012 standard. This is because all the
 * set_bit and get_bit functions work in LSB -> MSB
 */
enum FC_TYPE {
    TYPE_MANAGEMENT = 0,
    TYPE_CONTROL = 1,
    TYPE_DATA = 2,
    TYPE_RESERVED = 3
};
static const char* STR_FC_TYPE[] = {
	"MANAGEMENT",
	"CONTROL",
	"DATA",
	"RESERVED"
};

/**
 * Frame control field subtype: data
 */
enum FC_DATA_SUBTYPE {
    DATA_DATA = 0,
    DATA_CF_ACK = 1,
    DATA_CF_POLL = 2,
    DATA_CF_ACK_CF_POLL = 3,
    NULL_DATA = 4,
    CF_ACK = 5,
    CF_POLL = 6,
    CF_ACK_CF_POLL = 7,
    QOS_DATA = 8,
    QOS_DATA_CF_ACK = 9,
    QOS_DATA_CF_POLL = 10,
    QOS_DATA_CF_ACK_CF_POLL = 11,
    QOS_NULL = 12,
    DATA_RESERVED = 13,
    QOS_CF_POLL = 14,
    QOS_CF_ACK_CF_POLL = 15
};
static const char* STR_FC_DATA_SUBTYPE[] = {
	"DATA",
	"DATA_CF_ACK",
	"DATA_CF_POLL",
	"DATA_CF_ACK_CF_POLL",
	"NULL_DATA",
	"CF_ACK",
	"CF_POLL",
	"CF_ACK_CF_POLL",
	"QOS_DATA",
	"QOS_DATA_CF_ACK",
	"QOS_DATA_CF_POLL",
	"QOS_DATA_CF_ACK_CF_POLL",
	"QOS_NULL",
	"RESERVED",
	"QOS_CF_POLL",
	"QOS_CF_ACK_CF_POLL"
};

/**
 * Frame control field subtype: management
 */
enum FC_MANAGEMENT_SUBTYPE {
    ASSOCIATION_REQUEST = 0,
    ASSOCIATION_RESPONSE = 1,
    REASSOCIATION_REQUEST = 2,
    REASSOCIATION_RESPONSE = 3,
    PROBE_REQUEST = 4,
    PROBE_RESPONSE = 5,
    TIMING_ADVERTISEMENT = 6,
    RESERVED = 7,
    BEACON = 8,
    ATIM = 9,
    DISASSOCIATION = 10,
    AUTHENTICATION = 11,
    DEAUTHENTICATION = 12,
    ACTION = 13,
    ACTION_NO_ACK = 14,
    MANAGEMENT_RESERVED = 15
};
static const char* STR_FC_MANAGEMENT_SUBTYPE[] = {
	"ASSOCIATION_REQUEST",
	"ASSOCIATION_RESPONSE",
	"REASSOCIATION_REQUEST",
	"REASSOCIATION_RESPONSE",
	"PROBE_REQUEST",
	"PROBE_RESPONSE",
	"TIMING_ADVERTISEMENT",
	"RESERVED",
	"BEACON",
	"ATIM",
	"DISASSOCIATION",
	"AUTHENTICATION",
	"DEAUTHENTICATION",
	"ACTION",
	"ACTION_NO_ACK",
	"RESERVED"
};

/**
 * Frame control field subtype: control
 */
enum FC_CONTROL_SUBTYPE {
    CONTROL_RESERVED_FIRST = 0,
    CONTROL_RESERVED_LAST = 6,
    CONTROL_WRAPPER = 7,
    BLOCK_ACK_REQUEST = 8,
    BLOCK_ACK = 9,
    PS_POLL = 10,
    RTS = 11,
    CTS = 12,
    ACK = 13,
    CF_END = 14,
    CF_END_CF_ACK = 15
};
static const char* STR_FC_CONTROL_SUBTYPE[] = {
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"CONTROL_WRAPPER",
	"BLOCK_ACK_REQUEST",
	"BLOCK_ACK",
	"PS_POLL",
	"RTS",
	"CTS",
	"ACK",
	"CF_END"
};

/**
 * Frame control field subtype: reserved
 */
enum FC_RESERVED_SUBTYPE {
    RESERVED_RESERVED_FIRST = 0,
    RESERVED_RESERVED_LAST = 15
};
static const char* STR_FC_RESERVED_SUBTYPE[] = {
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED"
};

/**
 * Addresses functionalities as function of the ToDS and FromDS fields,
 * valid only for DATA frames!!!
 */
static const char *STR_DATA_ADDRESSES_FUNCTIONALITY[4][4] = {
	//ToDS = 0, FromDS = 0
	{"Destination", "Source", "BSSID", "Not used"},
	//ToDS = 0, FromDS = 1
	{"Destination", "BSSID", "Source", "Not used"},
	//ToDS = 1, FromDS = 0
	{"BSSID", "Source", "Destination", "Not used"},
	//ToDS = 1, FromDS = 1
	{"Receiver", "Transmitter", "Destination", "Source"},
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
 * Prints a mac address in textual format to a file
 *
 * \param address the mac address
 * \param f output file
 */
void print_mac_address(mac_address_t address, FILE *f);

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
struct MAC_DATAFRAME_HEADER generate_mac_header(dbyte frame_control, dbyte duration, const char *bssid, const char *receiver, const char *sender, byte sequence);

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

/**
 * Given a MAC control field, prints the textual representation of it
 *
 * \param frame_control frame control field
 * \param f output file
 */
void print_frame_control_field(dbyte frame_control, FILE *f);

/**
 * Set a bit in the frame control field
 *
 * \param frame_control frame control field
 * \param i index of the bit
 * \param b bit value
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_bit(dbyte *frame_control, int i, int b, int lsb);
char get_frame_control_bit(dbyte frame_control, int i, int lsb);

/**
 * Set protocol version for MAC control field
 *
 * \param frame_control frame control field
 * \param version protocol version to be set
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_version(dbyte *frame_control, char version, int lsb);

/**
 * Set type for MAC control field
 *
 * \param frame_control frame control field
 * \param type type to be set
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_type(dbyte *frame_control, enum FC_TYPE type, int lsb);

/**
 * Set subtype for MAC control field
 *
 * \param frame_control frame control field
 * \param subtype subtype to be set
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_subtype(dbyte *frame_control, char subtype, int lsb);

/**
 * Set from DS for MAC control field
 *
 * \param frame_control frame control field
 * \param from_ds fromDS to be set
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_from_ds(dbyte *frame_control, char from_ds, int lsb);
char get_frame_control_from_ds(dbyte frame_control, int lsb);

/**
 * Set to DS for MAC control field
 *
 * \param frame_control frame control field
 * \param to_ds toDS to be set
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_to_ds(dbyte *frame_control, char to_ds, int lsb);
char get_frame_control_to_ds(dbyte frame_control, int lsb);

/**
 * Set more fragment for MAC control field
 *
 * \param frame_control frame control field
 * \param more_fragment more fragment to be set
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_more_fragment(dbyte *frame_control, char more_fragment, int lsb);
char get_frame_control_more_fragment(dbyte frame_control, int lsb);

/**
 * Set retry for MAC control field
 *
 * \param frame_control frame control field
 * \param retry retry to be set
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_retry(dbyte *frame_control, char retry, int lsb);
char get_frame_control_retry(dbyte frame_control, int lsb);

/**
 * Set power management for MAC control field
 *
 * \param frame_control frame control field
 * \param power_management power management to be set
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_power_management(dbyte *frame_control, char power_management, int lsb);
char get_frame_control_power_management(dbyte frame_control, int lsb);

/**
 * Set more data for MAC control field
 *
 * \param frame_control frame control field
 * \param more_data more data to be set
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_more_data(dbyte *frame_control, char more_data, int lsb);
char get_frame_control_more_data(dbyte frame_control, int lsb);

/**
 * Set protected frame for MAC control field
 *
 * \param frame_control frame control field
 * \param protected_frame protected frame to be set
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_protected_frame(dbyte *frame_control, char protected_frame, int lsb);
char get_frame_control_protected_frame(dbyte frame_control, int lsb);

/**
 * Set order for MAC control field
 *
 * \param frame_control frame control field
 * \param order order to be set
 * \param lsb first bit of frame control is the LSB
 */
void set_frame_control_order(dbyte *frame_control, char order, int lsb);
char get_frame_control_order(dbyte frame_control, int lsb);

/**
 * Computes the crc32 for a set of bytes
 *
 * \param buf bytes for which crc must be computed
 * \param len size of buf in bytes
 */
unsigned int crc32(const char *buf, size_t len);

#endif /* MAC_UTILS_H_ */
