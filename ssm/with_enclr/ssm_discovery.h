/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define WWID_SIZE		64

#define BLK_DEV_SIZE		33
#define NODE_NAME_SIZE		19
#define FILE_NAME_SIZE		256

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
} ssm_lun_t;

typedef struct ssm_enclr {
	int		se_hostno;
	int		se_channel;
	int		se_nluns;
	ssm_lun_t	**se_luns;
} ssm_enclr_t;

typedef struct ssm_node {
	long		sn_hostid;
	int		sn_nenclrs;
	ssm_enclr_t	**sn_enclrs;
} ssm_node_t;

extern	ssm_lun_t	**ssm_disks;
extern	int		ssm_ndisks;

extern	ssm_node_t	this_node;

extern	int ssm_discover_devices();
extern	void ssm_dump_devices(ssm_lun_t **disks, int ndisks);
extern	int ssm_organize_devices();
extern	void ssm_dump_node(ssm_node_t *node);

