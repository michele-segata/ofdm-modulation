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
 * Description: general utilities for the project
 *
 */

#ifndef _UTILS_H_
#define _UTILS_H_

/**
 * For profiling DSP operations in the project we need a resolution down to
 * nanoseconds, which cannot be reached using gettimeofday(). Linux and Mac
 * systems have different calls for getting nanosecond resolution timers.
 * On Mac, we can use mach_absolute_time(), on linux we can use
 * clock_gettime(CLOCK_REALTIME)
 */
#ifdef __APPLE__
#include <mach/mach_time.h>
#else
#include <time.h>
#endif

typedef unsigned long int nanotimer_t;

/**
 * Starts a timer which can be used afterwards to compute elapsed time
 *
 * \return the started timer
 */
nanotimer_t start_timer();

/**
 * Computes the elapsed time, in nanoseconds, from the moment timer has
 * been started up to now.
 *
 * \param timer the timer started with start_timer()
 * \return the elapsed nanoseconds from the moment time has been started
 */
nanotimer_t elapsed_nanosecond(nanotimer_t timer);

#endif
