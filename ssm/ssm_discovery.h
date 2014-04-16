/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#ifndef SSM_DISCOVERY_H
#define SSM_DISCOVERY_H

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define WWID_SIZE		64

#define BLK_DEV_SIZE		33
#define NODE_NAME_SIZE		19
#define FILE_NAME_SIZE		256


#define SSM_BLOCK_FREE		0x00
#define	SSM_BLOCK_USED		0x01

enum pathstates {
	PSTATE_UNDEF,
	PSTATE_FAILED,
	PSTATE_ACTIVE
};

enum diskstates {
	DSTATE_UNDEF,
	DSTATE_FAILED,
	DSTATE_ACTIVE
};

struct ssm_sgid {
	int host_no;
	int channel;
	int scsi_id;
	int lun_id;
};

typedef struct ssm_path {
	char		sp_name[BLK_DEV_SIZE];
	int64_t		sp_devno;
	int		sp_state;
	struct ssm_sgid	sp_sgid;
} ssm_path_t;

typedef struct ssm_lun {
	char		sl_mpname[NODE_NAME_SIZE];	/* lun name */
	char		sl_dmname[NODE_NAME_SIZE];	/* dm name */
	char		sl_uuid[WWID_SIZE];		/* uuid */
	int		sl_size;			/* in GB */
	int64_t		sl_devno;			/* devno */
	int		sl_state;

	ssm_path_t	**sl_paths;
	int		sl_npaths;

	void		*sl_enclr;
	void		*sl_node;

	uint8_t		*sl_blockmap;			/* allocation map
							   for simplicity,
							   just use char */
	int		sl_nblocks;			/* num of blocks */
	size_t		sl_blksz;			/* block size in GB*/
} ssm_lun_t;

typedef struct ssm_enclr {
	int		se_hostno;
	int		se_channel;
	int		se_nluns;
	ssm_lun_t	**se_luns;
} ssm_enclr_t;

typedef struct ssm_node {
	uint64_t	sn_hostid;
	int		sn_ndisks;
	ssm_lun_t	**sn_disks;

	int		sn_avail_ndisks;
	ssm_lun_t	**sn_avail_disks;
} ssm_node_t;

typedef struct ssm_resource {
	ssm_node_t	**sr_node;		/* global node list */
	int		sr_nnodes;

	int		sr_min_size;		/* mininum size of the lun */
	int		sr_max_size;		/* maximux size of the lun */
	int		sr_nluns;		/* number of luns altogether */

	int		sr_avail_nnodes;	/* num of nodes having avail
						   disks */
	ssm_node_t	**sr_avail_node;	/* list of nodes having avail
						   disks */
} ssm_resource_t;

extern	ssm_node_t	this_node;
extern	ssm_resource_t	ssm_resource;

extern	int ssm_discover_devices();
extern	void ssm_dump_devices(ssm_lun_t **disks, int ndisks);
extern	void ssm_dump_node(ssm_node_t *node);
extern	int ssm_global_resource();

#endif /* SSM_DISCOVERY_H */
