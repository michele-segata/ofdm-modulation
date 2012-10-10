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
 * Define structure for the MAC header of a data frame.
 * Values as stored as MSB, but standard mandates LSB
 * representation. Bit utils functions can help changing
 * endiannes
 */
struct MAC_DATAFRAME_HEADER {
    char frame_control[2];  //protocol version, type, subtype, to_ds, from_ds, ...
    char duration[2];       //duration field
    char address1[6];       //address 1 indicating BSSID: 6 bytes MAC address
    char address2[6];       //address 2 indicating receiver
    char address3[6];       //address 3 indicating sender
    char sequence[2];       //sequence number plus fragment number
};

/**
 * Given a payload, generates a MAC data frame (i.e., a PSDU) to be given
 * to the physical layer for encoding.
 *
 * \param msdu the payload for the MAC frame
 * \param msdu_size the size of the msdu in bytes
 * \param psdu pointer to a byte array where to store the MAC frame. Memory
 * will be alloced by the function
 * \param psdu_size pointer to an integer where the size of the psdu in bytes
 * will be stored
 * \param seq sequence number of the frame
 */
void generate_mac_data_frame(const char *msdu, int msdu_size, char **psdu, int *psdu_size, char seq);

#endif /* MAC_UTILS_H_ */
