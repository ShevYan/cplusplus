/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sysfs/libsysfs.h>
#include <linux/kdev_t.h>

#include "ssm_discovery.h"

#define STATE_SIZE		64
#define SYSFS_PATH_SIZE		255
#define DEFAULT_CONFIGFILE	"/etc/multipath.conf"

ssm_node_t	this_node;

ssm_resource_t	ssm_resource;

/*
 * Get lun capacity by using sysfs interface
 */
static int
get_disk_capacity(char *path)
{
	struct sysfs_attribute	*attr;
	char			attr_path[SYSFS_PATH_SIZE];
	unsigned long long	size;
	int			ret;

	sprintf(attr_path, "/sys/block/%s/size", path);

	attr = sysfs_open_attribute(attr_path);
	if (!attr) {
		return -1;
	}

	if (sysfs_read_attribute(attr) < 0) {
		sysfs_close_attribute(attr);
		return -1;
	}

	ret = sscanf(attr->value, "%llu\n", &size);
	sysfs_close_attribute(attr);
	if (ret != 1)
		return -1;

	return (int)(size / 2 / 1024 / 1024);
}

/*
 * Issuing "/sbin/multipath -l mpahtX" command and retrieve following info:
 * Lun level:
 *	1. dm name
 *	2. uuid
 * Path:
 *	1. sg ID
 *	2. Device name
 *	3. major, minor numbers
 *	4. state
 */
static int
get_paths(ssm_lun_t *disk)
{
	int	ret = ENODEV, host, channel, scsi_id, lun_id, major, minor;
	FILE	*fpipe;
	char	multipath_cmd[FILE_NAME_SIZE];
	char	line[FILE_NAME_SIZE], *lp;
	char	mpname[NODE_NAME_SIZE], dmname[NODE_NAME_SIZE];
	char	devname[NODE_NAME_SIZE];
	char	uuid[WWID_SIZE], *uuidp, dontcare[FILE_NAME_SIZE];
	char	state[STATE_SIZE];
	ssm_path_t	*path;

	sprintf(multipath_cmd, "/sbin/multipath -l %s", disk->sl_mpname);

	if (!(fpipe = (FILE*)popen(multipath_cmd, "r"))) {
		perror("Problem with multipath command");
		return 1;
	}

	while (fgets(line, sizeof(line), fpipe)) {
		if (strncmp(line, "mpath", 5) == 0){
			/*
			 * Get the dm name and uuid
			 */
			sscanf(line, "%s %s %s %s", mpname, uuid, dmname,
			       dontcare);
			strcpy(disk->sl_dmname, dmname);

			/*
			 * remove () around uuid
			 */
			uuidp = uuid;
			if (uuidp[0] == '(') {
				uuidp++;
			}
			if (uuidp[strlen(uuidp) - 1] == ')') {
				uuidp[strlen(uuidp) - 1] = '\0';
			}

			strcpy(disk->sl_uuid, uuidp);
		} else if (strstr(line, "running") != NULL) {
			/*
			 * Comming down to a single path
			 */
			lp = line;
			while (!isdigit(lp[0]) && lp[0] != '\0') {
				lp++;
			}

			if (lp[0] == '\0') {
				fprintf(stderr, "Reached the end of the line "
					"unexpectedly: %s", line);
				continue;
			}

			sscanf(lp, "%d:%d:%d:%d %s %d:%d %s %s",
			       &host, &channel, &scsi_id, &lun_id,
			       devname, &major, &minor, state, dontcare);

			path = (ssm_path_t *)malloc(sizeof(ssm_path_t));
			if (!path) {
				fprintf(stderr,	"Failed to allocate memory for "
				"a path structure.\n");
				ret = ENOMEM;
				break;
			}

			strcpy(path->sp_name, devname);
			path->sp_devno = MKDEV(major, minor);
			path->sp_sgid.host_no = host;
			path->sp_sgid.channel = channel;
			path->sp_sgid.scsi_id = scsi_id;
			path->sp_sgid.lun_id = lun_id;

			if (strcmp(state, "active") == 0) {
				path->sp_state = PSTATE_ACTIVE;
			} else if (strcmp(state, "failed") == 0) {
				path->sp_state = PSTATE_FAILED;
			} else {
				path->sp_state = PSTATE_UNDEF;
			}

			/*
			 * Attach the new path to the lun structure
			 */
			if (disk->sl_npaths == 0) {
				disk->sl_paths = (ssm_path_t **)
					malloc(sizeof(ssm_path_t *));
			} else {
				disk->sl_paths = (ssm_path_t **)
					realloc(disk->sl_paths,								(disk->sl_npaths + 1) *
						sizeof(ssm_path_t *));
			}
			if (!disk->sl_paths) {
				fprintf(stderr, "Failed to allocate memory for "
					"disk->sl_paths.\n");
				free(path);
				ret = ENOMEM;
				break;
			}
			disk->sl_paths[disk->sl_npaths++] = path;

			ret = 0;
		}
	}

	pclose(fpipe);

	/*
	 * Get disk size from sysfs
	 */
	if (disk->sl_npaths == 0) {
		disk->sl_size = -1;
	} else {
		disk->sl_size = get_disk_capacity(disk->sl_paths[0]->sp_name);
	}

	return ret;
}

/*
 * Issuing "dmsetup ls --target multipath" command to get mpathX list
 */
int
ssm_discover_devices()
{
	int		i, ret;
	FILE		*fpipe;
	char		*dmsetup_cmd = "dmsetup ls --target multipath";
	char		line[FILE_NAME_SIZE], mpname[NODE_NAME_SIZE];
	int		major, minor;
	ssm_lun_t	*disk;

	if (!(fpipe = (FILE*)popen(dmsetup_cmd, "r"))) {
		perror("Problem with dmsetup command");
		return 1;
	}

	while (fgets(line, sizeof(line), fpipe)) {
		disk = (ssm_lun_t *)malloc(sizeof(ssm_lun_t));
		if (!disk){
			fprintf(stderr,	"Failed to allocate memory for "
				"a disk structure.\n");
			ret = ENOMEM;
			break;
		}

		/*
		 * Get mpath name and major/minor numbers
		 */
		ret = sscanf(line, "%s (%d, %d)", mpname, &major, &minor);
		if (ret != 3) {
			fprintf(stderr, "Failed to read disk name and (major, "
				"minor).\n");
			disk->sl_state = DSTATE_UNDEF;
			continue;
		}

		strcpy(disk->sl_mpname, mpname);
		disk->sl_devno = MKDEV(major, minor);
		disk->sl_npaths = 0;
		disk->sl_enclr = NULL;

		ret = get_paths(disk);
		if (ret) {
			fprintf(stderr, "Failed to read disk name and (major, "
				"minor).\n");
			disk->sl_state = DSTATE_FAILED;
		} else {
			disk->sl_state = DSTATE_ACTIVE;
		}

		/*
		 * Put it on the global dun list
		 */
		if (this_node.sn_ndisks == 0) {
			this_node.sn_disks = (ssm_lun_t **)
				malloc(sizeof(ssm_lun_t *));
		} else {
			this_node.sn_disks = (ssm_lun_t **)
				realloc(this_node.sn_disks,
					(this_node.sn_ndisks + 1) *
					 sizeof(ssm_lun_t *));
		}
		if (!this_node.sn_disks) {
			fprintf(stderr, "Failed to allocate memory for "
				"the disk list.\n");
			free(disk);
			ret = ENOMEM;
			break;
		}
		this_node.sn_disks[this_node.sn_ndisks++] = disk;

		disk->sl_node = (void *)&this_node;

		ret = 0;
	}

	pclose(fpipe);

	this_node.sn_hostid = gethostid();

	/*
	 * Initialize avail disks. They are all available before alloc happens
	 */
	this_node.sn_avail_ndisks = this_node.sn_ndisks;
	this_node.sn_avail_disks = (ssm_lun_t **)malloc(this_node.sn_ndisks *
							sizeof(ssm_lun_t *));
	for (i = 0; i < this_node.sn_avail_ndisks; i++) {
		this_node.sn_avail_disks[i] = this_node.sn_disks[i];
	}

	return ret;
}

/*
 * Print a list of disks
 */
void
ssm_dump_devices(ssm_lun_t **disks, int ndisks)
{
	int		i, j;
	ssm_lun_t	*disk;
	ssm_path_t	*path;

	for (i = 0; i < ndisks; i++) {
		disk = disks[i];

		/*
		 * Lun level
		 */
		fprintf(stdout, "%s: %s -> %s: ",
			disk->sl_uuid, disk->sl_mpname, disk->sl_dmname);

		fprintf(stdout, "%dGB (%ld:%ld) %s\n",
			disk->sl_size, MAJOR(disk->sl_devno),
			MINOR(disk->sl_devno),
			(disk->sl_state == DSTATE_ACTIVE) ? "active" :
			((disk->sl_state == DSTATE_FAILED) ? "failed" :
			 "undefined"));

		/*
		 * Path level
		 */
		for (j = 0; j < disk->sl_npaths; j++) {
			path = disk->sl_paths[j];

			fprintf(stdout, "\t[%d:%d:%d:%d]\t%s\t%s\t(%ld:%ld)\n",
				path->sp_sgid.host_no, path->sp_sgid.channel,
				path->sp_sgid.scsi_id, path->sp_sgid.lun_id,
				path->sp_name,
				(path->sp_state == PSTATE_ACTIVE) ? "active" :
				((path->sp_state == PSTATE_FAILED) ? "failed" :
				 "undefined"),
				MAJOR(path->sp_devno), MINOR(path->sp_devno));
		}
	}
}

void
ssm_dump_node(ssm_node_t *node)
{
	fprintf(stdout, "\n\n");
	fprintf(stdout, "---------------------------------------------\n");
	fprintf(stdout, "Node: %lx\n", node->sn_hostid);
	fprintf(stdout, "---------------------------------------------\n\n");

	ssm_dump_devices(node->sn_disks,  node->sn_ndisks);
	fprintf(stdout, "\n");
}

/*
 * Build global resouce table
 * TODO: collect node info from all the nodes in the cluster
 *       Need re-work for cluster env. For now fake for single node.
 */
int
ssm_global_resource()
{
	int		i, j;
	int		min_size = INT_MAX, max_size = 0, nluns = 0;
	ssm_node_t	*node;
	ssm_lun_t	*disk;

	ssm_resource.sr_nnodes = 1;
	ssm_resource.sr_node = (ssm_node_t **)malloc(sizeof(ssm_node_t *));
	ssm_resource.sr_node[0] = &this_node;

	/*
	 * Go through the resource list and find out what is the smallest
	 * and largest disk size and how many are there.
	 */
	for (i = 0; i < ssm_resource.sr_nnodes; i++) {
		node = ssm_resource.sr_node[i];
		for (j = 0; j < node->sn_ndisks; j++) {
			disk = node->sn_disks[j];

			if (disk->sl_size > max_size) {
				max_size = disk->sl_size;
			}

			if (disk->sl_size < min_size) {
				min_size = disk->sl_size;
			}

			nluns++;
		}
	}

	fprintf(stdout, "min_size = %dGB max_size = %dGB nlun = %d\n\n",
		min_size, max_size, nluns);

	ssm_resource.sr_min_size = min_size;
	ssm_resource.sr_max_size = max_size;
	ssm_resource.sr_nluns = nluns;

	ssm_resource.sr_avail_nnodes = 1;
	ssm_resource.sr_avail_node =
		(ssm_node_t **)malloc(sizeof(ssm_node_t *));
	ssm_resource.sr_avail_node[0] = &this_node;

	return 0;
}
