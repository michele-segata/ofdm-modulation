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

#endif
