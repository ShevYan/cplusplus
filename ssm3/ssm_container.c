/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#include "ssm_inc.h"

#define	RANDOM_CONTAINER

int	ssm_ctn_col_sz;		/* Container colomn size */
int	ssm_ctn_ncols;		/* Number of columns within a container */
int	ssm_ctn_unit_sz;	/* Container unit size in MB */
int	ssm_seqno;		/* SSM sequence number */

ssm_ctn_tbl_t	ssm_ctn_tbl;	/* Container table */


/*
 * Adjust available disk list within a node
 * If all the segments on the disk had been USED, remove it from the available
 * list
 */
static void
adjust_avail_disks(ssm_node_t *node, int diskno)
{
	int		i;
	ssm_lun_t	*disk;

	disk = node->sn_avail_disks[diskno];
	for (i = 0; i < disk->sl_nblocks; i++) {
		if (disk->sl_blockmap[i] == SSM_BLOCK_FREE) {
			/*
			 * There is still free segment on this disk, so
			 * we don't need to take this disk out of the available
			 * list.
			 */
			return;
		}
	}

	/*
	 * Take this disk out of the available list by moving the last
	 * element to the current slot
	 */
	node->sn_avail_ndisks--;
	node->sn_avail_disks[diskno] =
		node->sn_avail_disks[node->sn_avail_ndisks];
	node->sn_avail_disks[node->sn_avail_ndisks] = NULL;
}

/*
 * Adjust available node list in the global resource structure
 * If there is no available disks left, remove the node from the available
 * list
 */
static void
adjust_avail_node(int nodeno)
{
	ssm_node_t	*node;

	node = ssm_resource.sr_avail_node[nodeno];
	if (node->sn_avail_ndisks) {
		/*
		 * There is still free disks under this node. Do nothing
		 */
		return;
	}

	/*
	 * Take this node out of the available list by moving the last
	 * element to the current slot
	 */
	ssm_resource.sr_avail_nnodes--;
	ssm_resource.sr_avail_node[nodeno] =
		ssm_resource.sr_avail_node[ssm_resource.sr_avail_nnodes];
	ssm_resource.sr_avail_node[ssm_resource.sr_avail_nnodes] = NULL;
}

/*
 * Allocate one colume from a node's resource tree
 * Returns 0 if successfull
 * Otherwise return 1 and set segment number to SSM_CTN_COL_SEG_UNDEF
 */
static int
alloc_column_from_node(ssm_node_t *node, ssm_column_t *col)
{
	int		i, random_diskno;
	ssm_lun_t	*disk;

	while (node && node->sn_avail_ndisks) {
		random_diskno = ssm_random_within(node->sn_avail_ndisks);
		disk = node->sn_avail_disks[random_diskno];

		for (i = 0; i < disk->sl_nblocks; i++) {
			if (disk->sl_blockmap[i] == SSM_BLOCK_FREE) {
				/*
				 * Found a free slot. Done.
				 */
				disk->sl_blockmap[i] = SSM_BLOCK_USED;

				adjust_avail_disks(node, random_diskno);

				col->sc_node = node;
				col->sc_disk = disk;
				col->sc_seg = i;
				return 0;
			}
		}
	}

	/*
	 * There is no free block left at this point
	 */
	col->sc_seg = SSM_CTN_COL_SEG_UNDEF;
	return 1;
}

/*
 * Allocat one column from any node
 * Returns 0 if successfull, otherwise returns 1
 */
static int
alloc_column_from_any(ssm_column_t *col)
{
	int		random_nodeno, ret = 1;
	ssm_node_t	*node;

	if (ssm_resource.sr_avail_nnodes) {
		random_nodeno = ssm_random_within(ssm_resource.sr_avail_nnodes);
		node = ssm_resource.sr_avail_node[random_nodeno];

		ret = alloc_column_from_node(node, col);
		if (ret == 0) {
			adjust_avail_node(random_nodeno);
		}
	} else {
		/*
		 * Mark this columns was not allocated
		 */
		col->sc_seg = SSM_CTN_COL_SEG_UNDEF;
	}

	return ret;
}

/*
 * Free allocated container
 */
static void
free_container(ssm_container_t *container)
{
	int		i, segid;
	ssm_lun_t	*disk;

	for (i = 0; i < container->sc_ncols; i++) {
		if (container->sc_col[i].sc_seg == SSM_CTN_COL_SEG_UNDEF) {
			/*
			 * reached the end of the column list, or an error
			 * condition
			 */
			break;
		}

		/*
		 * Restore block used bitmap
		 */
		disk = container->sc_col[i].sc_disk;
		segid = container->sc_col[i].sc_seg;
		disk->sl_blockmap[segid] = SSM_BLOCK_FREE;
	}

	free(container->sc_col);
	free(container);
}

static ssm_bitmap_t *
ctn_space_bmap_init(int col_size, int unit_size)
{
	int		nbits;
	bitmap_t	*bitmap;
	ssm_bitmap_t	*bmap;

	nbits = (col_size + unit_size - 1) / unit_size;
	bitmap = bmap_alloc(nbits);
	if (!bitmap) {
		fprintf(stderr,	"Failed to allocate bitmap.\n");
		return NULL;
	}

	bmap = (ssm_bitmap_t *)malloc(sizeof(ssm_bitmap_t));
	if (!bmap) {
		fprintf(stderr,	"Failed to allocate bitmap structure.\n");
		free(bitmap);
		return NULL;
	}

	bmap->sb_magic = SSM_MAGIC_1_0;
	bmap->sb_seqno = ssm_seqno;
	bmap->sb_bitmap = bitmap;
	bmap->sb_nbit = nbits;

	return bmap;
}

/*
 * Create a single container
 * Returns a pointer to container if successfull
 * Returns NULL if failed
 */
static ssm_container_t *
create_container()
{
	int		i, ret;
	ssm_container_t	*container = NULL;

	/*
	 * Allocate one container
	 */
	container = (ssm_container_t *)malloc(sizeof(ssm_container_t));
	if (!container) {
		fprintf(stderr,	"Failed to allocate memory for a container.\n");
		return NULL;
	}

	ret = ssm_guid_create(&container->sc_guid);
	if (ret != 0 ) {
		fprintf(stderr,	"Failed to generate guid for a container.\n");
		free(container);
		return NULL;
	}

	container->sc_genid = 0;
	container->sc_owner = this_node.sn_hostid;
	container->sc_colsz = ssm_ctn_col_sz * 1024;
	container->sc_unitsz = ssm_ctn_unit_sz;
	container->sc_ncols = ssm_ctn_ncols;

	container->sc_col = (ssm_column_t *)malloc(container->sc_ncols *
						  sizeof(ssm_column_t));
	if (!container->sc_col) {
		fprintf(stderr,	"Failed to allocate memory for columns.\n");
		free(container);
		return NULL;
	}

	bzero(container->sc_col, container->sc_ncols * sizeof(ssm_column_t));

	/*
	 * Try to allocate the first column from this node
	 */
	ret = alloc_column_from_node(&this_node, &container->sc_col[0]);
	if (ret) {
		free_container(container);
		return NULL;
	}

	/*
	 * Continue with the rest of the columns
	 */
	for (i = 1; i < container->sc_ncols; i++){
		ret = alloc_column_from_any(&container->sc_col[i]);
		if (ret) {
			free_container(container);
			return NULL;
		}
	}

	/*
	 * Allocate and initialize the space bitmap
	 */
	container->sc_space_bmap = ctn_space_bmap_init(container->sc_colsz,
						       container->sc_unitsz);
	container->sc_space_avail = container->sc_space_bmap->sb_nbit;

	return container;
}

void
ssm_print_disk_usage()
{
	int		i, j, k;
	ssm_node_t	*node;
	ssm_lun_t	*disk;

	fprintf(stdout, "\nDisk Usage Map: \n\n");
	for (i = 0; i < ssm_resource.sr_nnodes; i++) {
		node = ssm_resource.sr_node[i];
		for (j = 0; j < node->sn_ndisks; j++) {
			disk = node->sn_disks[j];
			fprintf(stdout, "%s:\t", disk->sl_dmname);

			for (k = 0; k < disk->sl_nblocks; k++) {
				fprintf(stdout, "  %d",
					(int)disk->sl_blockmap[k]);
			}

			fprintf(stdout, "\n");
		}
	}
}

void
ssm_print_container(FILE *fd, int id, ssm_container_t *container)
{
	int		i;
	ssm_column_t	*col;
	char		guidbuf[MAXGUIDSTR];

	fprintf(fd, "\n\nContainer %d: %s\n\n",
		id, ssm_guidtostr(container->sc_guid, guidbuf));

	fprintf(fd, "Generation ID: %d\n", container->sc_genid);
	fprintf(fd, "Owner ID: %lX\n", container->sc_owner);
	fprintf(fd, "Column Size: %ld\n", container->sc_colsz);
	fprintf(fd, "\n");
	fprintf(fd, "Number of Columns: %d\n", container->sc_ncols);
	fprintf(fd, "\n");
	for (i = 0; i< container->sc_ncols; i++) {
		col = &container->sc_col[i];
		fprintf(fd, "column %2d:\t%lX\t%8s\t%d\n", i,
			col->sc_node->sn_hostid, col->sc_disk->sl_dmname,
			col->sc_seg);
	}

	/*
	 * Print space bitmap
	 */
	if (container->sc_space_bmap) {
		fprintf(fd, "\nSpace bitmap: (%ld bits: printing truncated "
			"to 500)\n", container->sc_space_bmap->sb_nbit);
		ssm_print_bitmap(fd, container->sc_space_bmap, 0, 500);
	}
}

/*
 * Dump containers to a file
 */
int
ssm_dump_container(char *filename, ssm_ctn_tbl_t *ctn_tbl)
{
	int	i;
	FILE	*fd;

	fd = fopen(filename, "w");
	if (fd == NULL) {
		fprintf(stderr, "Failed to open %s.\n", filename);
		return 1;
	}

	fprintf(stdout, "dumping container map to file %s...\n", filename);

	for (i = 0; i < ctn_tbl->scb_entries; i++) {
		ssm_print_container(fd, i, ctn_tbl->scb_ctn[i]);
	}

	fflush(fd);

	fclose(fd);

	return 0;
}

/*
 * Dump container bitmap to a file
 */
int
ssm_dump_container_bitmap(char *filename, ssm_container_t *ctn)
{
	FILE	*fd;

	fd = fopen(filename, "w");
	if (fd == NULL) {
		fprintf(stderr, "Failed to open %s.\n", filename);
		return 1;
	}

	ssm_print_bitmap(fd, ctn->sc_space_bmap, 0,
			 ctn->sc_space_bmap->sb_nbit);

	fflush(fd);

	fclose(fd);

	return 0;
}

int
ssm_build_container_table()
{
	int		i, j, k, ret = 0;
	ssm_node_t	*node;
	ssm_lun_t	*disk;
	ssm_container_t	*container;


	/*
	 * TODO: Use the minimal size of the disk for now.
	 */
	ssm_ctn_col_sz = ssm_resource.sr_min_size;
	ssm_ctn_ncols = ssm_data_nvecs + ssm_parity_nvecs;
	ssm_ctn_unit_sz = SSM_CTN_UNIT_SIZE_DEFAULT;

	/*
	 * Initialize disk block usage map for each disk
	 */
	for (i = 0; i < ssm_resource.sr_nnodes; i++) {
		node = ssm_resource.sr_node[i];
		for (j = 0; j < node->sn_ndisks; j++) {
			disk = node->sn_disks[j];
			disk->sl_nblocks = disk->sl_size / ssm_ctn_col_sz;
			disk->sl_blksz = ssm_ctn_col_sz;
			disk->sl_blockmap = (unsigned char *)
				malloc(disk->sl_nblocks *
				       sizeof(unsigned char));
			if (!disk->sl_blockmap) {
				fprintf(stderr,	"Failed to allocate memory "
					"for disk block usage map.\n");
				return ENOMEM;
			}

			for (k = 0; k < disk->sl_nblocks; k++) {
				disk->sl_blockmap[k] = SSM_BLOCK_FREE;
			}
		}
	}

	ssm_print_disk_usage();

	while (1) {
		container = create_container();
		if (!container) {
			break;
		}

		ssm_print_container(stdout, ssm_ctn_tbl.scb_entries, container);
		ssm_print_disk_usage();

		if (ssm_ctn_tbl.scb_entries == 0) {
			ssm_ctn_tbl.scb_ctn = (ssm_container_t **)
				malloc(sizeof(ssm_container_t *));
		} else {
			ssm_ctn_tbl.scb_ctn = (ssm_container_t **)
				realloc(ssm_ctn_tbl.scb_ctn,
					(ssm_ctn_tbl.scb_entries + 1) *
					sizeof(ssm_container_t *));
		}
		if (!ssm_ctn_tbl.scb_ctn) {
			fprintf(stderr, "Failed to allocate memory for "
				"the container table.\n");
			free_container(container);
			ret = ENOMEM;
			break;
		}
		ssm_ctn_tbl.scb_ctn[ssm_ctn_tbl.scb_entries++] = container;
	}

	return ret;
}

/*
 * This routine tries to find a container that can accommodate 'len' of data
 * Returns a container on success, otherwise NULL.
 * TODO:
 *	locking???
 */
ssm_container_t	*
ssm_select_container(size_t len)
{
	int	i, rand;
	int	oneMB = 1024 * 1024;
	size_t	nbits, lenMB;
	ssm_container_t	*ctnr = NULL, *temp;

	if (len < oneMB) {
		fprintf(stderr, "IO size is less than 1MB.\n");
		return NULL;
	}

	/*
	 * round up to MB
	 */
	lenMB = (len + oneMB - 1) / oneMB;

	for (i = 0; i < ssm_ctn_tbl.scb_entries; i++) {
		ctnr = ssm_ctn_tbl.scb_ctn[i];

		/*
		 * convert it to the number of bits required
		 */
		nbits = (lenMB + ctnr->sc_unitsz - 1) / ctnr->sc_unitsz;
		if (ctnr->sc_space_avail > nbits) {
#ifdef	RANDOM_CONTAINER
			/*
			 * Found a container we could use.
			 * This container will be the most recently used.
			 * To maintain the randomness, swap this container
			 * with a random container.
			 */
			rand = ssm_random_within(ssm_ctn_tbl.scb_entries);
			if (rand != i){
				temp = ssm_ctn_tbl.scb_ctn[i];
				ssm_ctn_tbl.scb_ctn[i] =
					ssm_ctn_tbl.scb_ctn[rand];
				ssm_ctn_tbl.scb_ctn[rand] = temp;
			}
#endif
			return ctnr;
		}
	}

	return NULL;
}

ssm_container_t	*
ssm_select_container_by_index(size_t len, int index)
{
	int	oneMB = 1024 * 1024;
	size_t	nbits, lenMB;
	ssm_container_t	*ctnr = NULL;

	if (len < oneMB) {
		fprintf(stderr, "IO size is less than 1MB.\n");
		return NULL;
	}

	/*
	 * round up to MB
	 */
	lenMB = (len + oneMB - 1) / oneMB;

	if (index >= ssm_ctn_tbl.scb_entries) {
		fprintf(stderr, "Not a valid index.\n");
		return NULL;
	}

	ctnr = ssm_ctn_tbl.scb_ctn[index];

	nbits = (lenMB + ctnr->sc_unitsz - 1) / ctnr->sc_unitsz;
	if (ctnr->sc_space_avail > nbits) {
		return ctnr;
	}

	return NULL;
}
