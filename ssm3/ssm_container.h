/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#ifndef SSM_CONTAINER_H
#define SSM_CONTAINER_H

#include "ssm_inc.h"

#define SSM_CTN_UNIT_SIZE_DEFAULT	1	/* default unit size is 1MB */
#define SSM_CTN_COL_SEG_UNDEF		-1

typedef struct ssm_column {
	ssm_node_t	*sc_node;		/* ptr to the node */
	ssm_lun_t	*sc_disk;		/* ptr to the disk */
	int		sc_seg;			/* segment no within disk */
} ssm_column_t;

typedef struct ssm_container {
	ssm_guid_t	sc_guid;
	int		sc_genid;		/* generation number */
	uint64_t	sc_owner;

	size_t		sc_colsz;		/* the size of a column in MB */
	size_t		sc_unitsz;		/* alloc unit size in MB */

	int		sc_ncols;		/* number of columns */
	ssm_column_t	*sc_col;		/* list of columns */
	ssm_bitmap_t	*sc_space_bmap;
	size_t		sc_space_avail;	/* available space/bits */
} ssm_container_t;

typedef struct ssm_ctn_tbl {
	int		scb_genno;		/* generation number */
	ssm_container_t	**scb_ctn;		/* Container table */
	int		scb_entries;		/* number of container table
						   entries */
} ssm_ctn_tbl_t;

extern	ssm_ctn_tbl_t	ssm_ctn_tbl;
extern	int ssm_ctn_col_sz;
extern	int ssm_ctn_unit_sz;

extern	int ssm_build_container_table();
extern	ssm_container_t	*ssm_select_container(size_t len);
extern	ssm_container_t	*ssm_select_container_by_index(size_t len, int index);
extern	int ssm_dump_container(char *filename, ssm_ctn_tbl_t *ctn_tbl);
extern	int ssm_dump_container_bitmap(char *filename, ssm_container_t *ctn);

#endif /* SSM_CONTAINER_H */
