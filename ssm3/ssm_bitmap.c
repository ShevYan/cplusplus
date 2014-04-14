/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

/*
 * bmap_alloc - allocate an array of bits for the bmap_ functions
 *
 * Return NULL if allocation failed.
 */

#include "ssm_inc.h"

bitmap_t *
bmap_alloc(nbits)
	int nbits;
{
	bitmap_t *ptr = (bitmap_t *)malloc(sizeof(bitmap_t) * BIT_NMAP(nbits));
	if (ptr) {
		memset(ptr, 0, sizeof(bitmap_t) * BIT_NMAP(nbits));
	}

	return ptr;
}

/*
 * bmap_realloc - reallocate bit array
 *
 * Use old size to determine if bits need to be cleared after the realloc.
 */

bitmap_t *
bmap_realloc(map, nbits, old_nbits)
	bitmap_t *map;
	int nbits;
	int old_nbits;
{
	bitmap_t *new;

	new = (bitmap_t *)malloc (sizeof(bitmap_t) * BIT_NMAP(nbits));
	if (new) {
		memset(new, 0, sizeof(bitmap_t) * BIT_NMAP(nbits));
		if (map) {
			memcpy((void *)new, (void *)map,
			       sizeof(bitmap_t) * BIT_NMAP(old_nbits));
			free(map);
			map = NULL;
		}

		if (nbits > old_nbits) {
			/*
			 * Clear the freshly allocated bits from old_nbits to
			 * till the end of newly allocated bits.
			 */
			bmap_clear_range(new, old_nbits, nbits - old_nbits);
		}
	}

	return new;
}

/*
 * bmap_find_set - find the index of the first set bit in a bit array
 * bmap_find_set_reverse_range - search for a contiguous bit range
 *
 * Return -1 if not set bit could be found
 */

int
bmap_find_set(map, nbits)
	bitmap_t *map;
	int nbits;
{

	return bmap_find_set_from(map, nbits, 0);
}
int
bmap_find_set_from(map, nbits, startbit)
	bitmap_t *map;
	int nbits;
	int startbit;
{
	int nmap = BIT_NMAP(nbits);
	int startmap = startbit / MAP_BIT;
	int ibit, masklim;
	bitmap_t mask;

	ibit = startmap * MAP_BIT;
	map += startmap;
	nmap -= startmap;

	for (;;) {
		while (nmap && *map == 0) {
			map++;
			--nmap;
			ibit += MAP_BIT;
		}
		if (nmap == 0)
			return -1;
		mask = 1;
		masklim = nbits < (int)MAP_BIT? nbits: (ibit + (int)MAP_BIT);
		while (ibit < masklim) {
			if (mask & *map && ibit >= startbit)
				return ibit;
			ibit++;
			mask <<= 1;
		}
		map++;
		--nmap;
	}
}

/*
 * bmap_find_set_reverse - search backward for a set bit in a bit array
 *
 * Return -1 if all bits were set.
 */

int
bmap_find_set_reverse(map, nbits)
	bitmap_t *map;
	int nbits;
{
	int nmap;
	int ibit, i;
	bitmap_t mask;

	nmap = BIT_NMAP(nbits);
	ibit = nmap * MAP_BIT;
	map += nmap - 1;

	for (;;) {
		while (nmap && *map == 0) {
			--map;
			--nmap;
			ibit -= MAP_BIT;
		}
		if (nmap == 0)
			return -1;
		mask = (bitmap_t)1 << (MAP_BIT - 1);
		for (i = MAP_BIT; i > 0; --i) {
			if (mask & *map && ibit < nbits)
				return ibit;
			--ibit;
			mask >>= 1;
		}
		--map;
		--nmap;
	}
}

/*
 * bmap_find_clear - find the index of the first zero bit in a bit array
 * bmap_find_clear_from - find a zero bit starting at a specific bit offset
 *
 * Return -1 if no zero bit could be found
 */

int
bmap_find_clear(map, nbits)
	bitmap_t *map;
	int nbits;
{

	return bmap_find_clear_from(map, nbits, 0);
}

int
bmap_find_clear_from(map, nbits, startbit)
	bitmap_t *map;
	int nbits;
	int startbit;
{
	int nmap = BIT_NMAP(nbits);
	int startmap = startbit / MAP_BIT;
	int ibit, masklim;
	bitmap_t mask;

	ibit = startmap * MAP_BIT;
	map += startmap;
	nmap -= startmap;

	for (;;) {
		while (nmap && *map == (bitmap_t)(~0)) {
			map++;
			--nmap;
			ibit += MAP_BIT;
		}
		if (nmap == 0)
			return -1;
		mask = 1;
		masklim = nbits < (int)MAP_BIT? nbits: (ibit + (int)MAP_BIT);
		while (ibit < masklim) {
			if (! (mask & *map) && ibit >= startbit)
				return ibit;
			ibit++;
			mask <<= 1;
		}
		map++;
		--nmap;
	}
}

/*
 * bmap_find_clear_reverse - search backward for a zero bit in a bit array
 *
 * Return -1 if all bits were zero.
 */

int
bmap_find_clear_reverse(map, nbits)
	bitmap_t *map;
	int nbits;
{
	int nmap;
	int ibit, i;
	bitmap_t mask;

	nmap = BIT_NMAP(nbits);
	ibit = nmap * MAP_BIT - 1;
	map += nmap - 1;

	for (;;) {
		while (nmap && *map == (bitmap_t)(~0)) {
			--map;
			--nmap;
			ibit -= MAP_BIT;
		}
		if (nmap == 0)
			return -1;
		mask = (bitmap_t)1 << (MAP_BIT - 1);
		for (i = MAP_BIT; i > 0; --i) {
			if (! (mask & *map) && ibit < nbits)
				return ibit;
			--ibit;
			mask >>= 1;
		}
		--map;
		--nmap;
	}
}

/*
 * bmap_set - set bit in bit array
 */

void
bmap_set(map, bit)
	bitmap_t *map;
	int bit;
{
	BMAP_SET(map, bit);
}

/*
 * bmap_clear - clear bit in bit array
 */

void
bmap_clear(map, bit)
	bitmap_t *map;
	int bit;
{
	BMAP_CLEAR(map, bit);
}

/*
 * bmap_test - test state of bit in bit array
 *
 * Return:
 *	0  bit is clear
 *	1 bit is set
 */

int
bmap_test(map, bit)
	bitmap_t *map;
	int bit;
{
	return BMAP_TEST(map, bit);
}

/*
 * bmap_add - set all bits in a bit array that are set in another bit array
 */

void
bmap_add(dstmap, srcmap, nbits)
	bitmap_t *dstmap;
	bitmap_t *srcmap;
	int nbits;
{
	int nmap = BIT_NMAP(nbits);

	while (nmap) {
		*dstmap++ |= *srcmap++;
		--nmap;
	}
}

/*
 * bmap_sub - clear all bits in a bit array that are set in another bit array
 */

void
bmap_sub(dstmap, srcmap, nbits)
	bitmap_t *dstmap;
	bitmap_t *srcmap;
	int nbits;
{
	int nmap = BIT_NMAP(nbits);

	while (nmap) {
		*dstmap++ &= ~(*srcmap++);
		--nmap;
	}
}


/*
 * bmap_find_clear_from - find a clear bit starting at a specific bit offset
 * bmap_find_clear_reverse_range - search backward for a contiguous bit range
 *
 * Return -1 if no such contiguous range was found.
 */

int
bmap_find_clear_range(map, nbits, startbit, count)
	bitmap_t *map;
	int nbits;
	int startbit;
	int count;
{
	int bit = startbit;
	int lim;

next:
	startbit = bmap_find_clear_from(map, nbits, bit);
	if (startbit == -1)
		return startbit;
	lim = startbit + count;
	if (lim > nbits)
		return -1;
	for (bit = startbit; bit < lim; bit++) {
		if (BMAP_TEST(map, bit))
			goto next;
	}
	return startbit;
}
int
bmap_find_clear_reverse_range(map, startbit, count)
	bitmap_t *map;
	int startbit;
	int count;
{
	int bit = startbit;
	int lim;

next:
	startbit = bmap_find_clear_reverse(map, bit + 1);
	if (startbit == -1)
		return startbit;
	lim = startbit - count;
	if (lim < -1)
		return -1;
	for (bit = startbit; bit > lim; --bit) {
		if (BMAP_TEST(map, bit))
			goto next;
	}
	return startbit;
}

/*
 * bmap_count_clear - count the number of contiguous clear bits from a start bit
 * bmap_count_clear_reverse - count clear bits in the reverse direction
 */

int
bmap_count_clear(map, startbit, endbit)
	bitmap_t *map;
	int startbit;
	int endbit;
{
	int bit = startbit;

	for (bit = startbit; bit <= endbit; bit++) {
		if (BMAP_TEST(map, bit))
			break;
	}
	return bit - startbit;
}
int
bmap_count_clear_reverse(map, startbit, endbit)
	bitmap_t *map;
	int startbit;
	int endbit;
{
	int bit = startbit;

	for (bit = startbit; bit >= endbit; --bit) {
		if (BMAP_TEST(map, bit))
			break;
	}
	return startbit - bit;
}

/*
 * bmap_set_range - set a range of bits
 * bmap_clear_range - clear a range of bits
 */

void
bmap_set_range(map, bit, nbit)
	bitmap_t *map;
	int bit;
	int nbit;
{
	int i;

	for (i = nbit; i > 0; --i, bit++) {
		BMAP_SET(map, bit);
	}
}
void
bmap_clear_range(map, bit, nbit)
	bitmap_t *map;
	int bit;
	int nbit;
{
	int i;

	for (i = nbit; i > 0; --i, bit++) {
		BMAP_CLEAR(map, bit);
	}
}

