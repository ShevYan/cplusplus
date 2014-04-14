/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#ifndef SSM_BMAP_H
#define SSM_BMAP_H

#include "ssm_inc.h"

#define SSM_MAGIC_1_0	('S' << 24 | 'S' << 16 | 'M' << 8 | '1')

typedef unsigned long bitmap_t;

/*
 * MAP_BIT: number of bits for each bitmap_t
 * BIT_NMAP: number of bitmap_t for a given nbits
 */
#define MAP_BIT (CHAR_BIT * sizeof(bitmap_t))
#define BIT_NMAP(nbits) ((size_t)(((nbits) + MAP_BIT - 1) / MAP_BIT))

#define BMAP_SET(map, bit) \
	((void)((map)[(bit) / MAP_BIT] |= 1UL << ((bit) % MAP_BIT)))
#define BMAP_CLEAR(map, bit) \
	((void)((map)[(bit) / MAP_BIT] &= ~(1UL << ((bit) % MAP_BIT))))
#define BMAP_TEST(map, bit) \
	(((map)[(bit) / MAP_BIT] & (1UL << ((bit) % MAP_BIT)))? 1: 0)

typedef struct ssm_bitmap {
	int		sb_magic;
	int		sb_seqno;

	bitmap_t	*sb_bitmap;
	size_t		sb_nbit;		/* bitmap size in bits */
} ssm_bitmap_t;

extern bitmap_t *bmap_alloc(int);
extern bitmap_t *bmap_realloc(bitmap_t *, int, int);
extern int bmap_find_set(bitmap_t *, int);
extern int bmap_find_set_from(bitmap_t *, int, int);
extern int bmap_find_set_reverse(bitmap_t *, int);
extern int bmap_find_clear(bitmap_t *, int);
extern int bmap_find_clear_from(bitmap_t *, int, int);
extern int bmap_find_clear_reverse(bitmap_t *, int);
extern void bmap_set(bitmap_t *, int);
extern void bmap_clear(bitmap_t *, int);
extern int bmap_test(bitmap_t *, int);
extern void bmap_add(bitmap_t *, bitmap_t *, int);
extern void bmap_sub(bitmap_t *, bitmap_t *, int);
extern int bmap_find_clear_range(bitmap_t *, int, int, int);
extern int bmap_find_clear_reverse_range(bitmap_t *, int, int);
extern int bmap_count_clear(bitmap_t *, int, int);
extern int bmap_count_clear_reverse(bitmap_t *, int, int);
extern void bmap_set_range(bitmap_t *, int, int);
extern void bmap_clear_range(bitmap_t *, int, int);

#endif /* SSM_BMAP_H */
