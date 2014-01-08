/*
 * Copyright (C) 2013 Andrea Mazzoleni
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __RAID_H
#define __RAID_H

/**
 * RAID mode supporting up to 6 parities.
 *
 * It requires SSSE3 to get good performance with triple or more
 * parities.
 *
 * This is the default mode set after calling raid_init().
 */
#define RAID_MODE_CAUCHY 0

/**
 * RAID mode supporting up to 3 parities,
 *
 * It has a fast triple parity implementation even without SSSE3, but it
 * cannot go beyond triple parity.
 * This is mostly intended for low end CPUs like ARM and AMD Athlon.
 */
#define RAID_MODE_VANDERMONDE 1

/**
 * Maximum number of parity disks supported.
 */
#define RAID_PARITY_MAX 6

/**
 * Maximum number of data disks supported.
 */
#define RAID_DATA_MAX 251

/**
 * Initializes the RAID system.
 */
void raid_init(void);

/**
 * Runs a basic functionality self test.
 *
 * It returns 0 on success.
 */
int raid_selftest(void);

/**
 * Sets the mode to use. One of RAID_MODE_*.
 *
 * You can change mode at any time, and it will affect next calls to raid_gen(),
 * raid_rec() and raid_recpar().
 *
 * The two modes are compatible for the first two levels of parity.
 * The third one is different.
 */
void raid_mode(int mode);

/**
 * Sets the zero buffer to use in recovering.
 *
 * Before calling raid_rec_dataonly() and raid_recpar() you must provide a memory
 * buffer filled with zero with the same size of the blocks to recover.
 */
void raid_zero(void *zero);

/**
 * Sets an additional buffer used by raid_rec_dataonly() to avoid to overwrite
 * unused parities.
 */
void raid_waste(void *zero);

/**
 * Computes the parity blocks.
 *
 * This function computes the specified number of parity blocks of the
 * provided set of data blocks.
 *
 * Each parity block, will allow to recover on data block.
 *
 * @nd Number of data blocks.
 * @np Number of parities blocks to compute.
 * @size Size of the blocks pointed by @v. It must be a multipler of 64.
 * @v Vector of pointers to the blocks of data and parity.
 *   It has (@nd + @np) elements. The starting elements are the blocks for
 *   data, following with the parity blocks.
 *   Data blocks are only read and not modified. Parity blocks are written.
 *   Each block has @size bytes.
 */
void raid_gen(int nd, int np, size_t size, void **v);

/**
 * Recovers failures in data and parity blocks.
 *
 * This function recovers all the data and parity blocks marked as bad
 * in the @ir vector.
 *
 * Ensure to have @nr <= @np, otherwise recovering is not possible.
 *
 * The parities blocks used for recovering are automatically selected from
 * the ones NOT present in the @ir vector.
 *
 * In case there are more parity blocks than needed to recover, the parities
 * at lower indexes are used in the recovering, and the others are ignored.
 *
 * Note that no internal integrity check is done when recovering. If the
 * provided parities are correct the resulting data will be also correct.
 * If parities are wrong, also the resulting recovered data will be wrong.
 * This happens even in the case you have more parities blocks than needed,
 * and some form of integrity verification is possible.
 *
 * @nr Number of failed data and parity blocks to recover.
 * @ir[] Vector of @nr indexes of the data and parity blocks to recover.
 *   The indexes start from 0. They must be in order.
 *   The first parity is represented with value @nd, the second with value
 *   @nd + 1, just like positions in the @v vector.
 * @nd Number of data blocks.
 * @np Number of parity blocks.
 * @size Size of the blocks pointed by @v. It must be a multipler of 64.
 * @v Vector of pointers to the blocks of data and parity.
 *   It has (@nd + @np) elements. The starting elements are the blocks
 *   for data, following with the parity blocks.
 *   Each block has @size bytes.
 */
void raid_rec(int nr, int *ir, int nd, int np, size_t size, void **v);

/**
 * Recovers failures of data blocks using the specified parities.
 *
 * The data blocks marked as bad in the @id vector are recovered.
 *
 * If you have provided an additional buffer with raid_waste(), the
 * parity blocks are not modified. Without this buffer, the content of
 * parities blocks not specified in the @ip vector will be destroyed.
 *
 * @nr Number of failed data blocks to recover.
 * @id[] Vector of @nr indexes of the data blocksto recover.
 *   The indexes start from 0. They must be in order.
 * @ip[] Vector of @nr indexes of the parity blocks to use for recovering.
 *   The indexes start from 0. They must be in order.
 * @nd Number of data blocks.
 * @size Size of the blocks pointed by @v. It must be a multipler of 64.
 * @v Vector of pointers to the blocks of data and parity.
 *   It has (@nd + @ip[@nr - 1] + 1) elements. The starting elements are the
 *   blocks for data, following with the parity blocks.
 *   Each blocks has @size bytes.
 */
void raid_rec_dataonly(int nr, int *id, int *ip, int nd, size_t size, void **v);

/**
 * Sorts a small vector of integers.
 *
 * If you have block indexes not in order, you can use this function to sort
 * them before calling raid_rec().
 *
 * @n Number of integers. No more than RAID_PARITY_MAX.
 * @v Vector of integers.
 */
void raid_sort(int n, int *v);

#endif
