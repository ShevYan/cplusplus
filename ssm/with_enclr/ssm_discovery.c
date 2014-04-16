/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#include <unistd.h>
#include <sysfs/libsysfs.h>
#include <linux/kdev_t.h>

#include "ssm_discovery.h"

#define STATE_SIZE		64
#define SYSFS_PATH_SIZE		255
#define DEFAULT_CONFIGFILE	"/etc/multipath.conf"

ssm_lun_t	**ssm_disks;
int		ssm_ndisks = 0;

ssm_node_t	this_node;

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
				disk->sl_npaths++;
				disk->sl_paths = (ssm_path_t **)
					malloc(sizeof(ssm_path_t *));
			} else {
				disk->sl_npaths++;
				disk->sl_paths = (ssm_path_t **)
					realloc(disk->sl_paths,								disk->sl_npaths * sizeof(ssm_path_t *));
			}
			if (!disk->sl_paths) {
				fprintf(stderr, "Failed to allocate memory for "
					"disk->sl_paths.\n");
				free(path);
				ret = ENOMEM;
				break;
			}
			disk->sl_paths[disk->sl_npaths - 1] = path;

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
	int		ret;
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
		if (ssm_ndisks == 0) {
			ssm_ndisks++;
			ssm_disks = (ssm_lun_t **)malloc(sizeof(ssm_lun_t *));
		} else {
			ssm_ndisks++;
			ssm_disks = (ssm_lun_t **)realloc(ssm_disks,
				(ssm_ndisks * sizeof(ssm_lun_t *)));
		}
		if (!ssm_disks) {
			fprintf(stderr, "Failed to allocate memory for "
				"ssm_disks.\n");
			free(disk);
			ret = ENOMEM;
			break;
		}
		ssm_disks[ssm_ndisks - 1] = disk;

		ret = 0;
	}

	pclose(fpipe);

	return ret;
}

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

		fprintf(stdout, "%d GB (%ld:%ld) %s\n",
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

/*
 * Organize devices into following layout:
 * Node
 *   \- Enclosure
 *          \- Lun
 *              \-Path
 */
int
ssm_organize_devices()
{
	int		i, j, k, count, hostno, channel;
	ssm_path_t	*path;
	ssm_enclr_t	*enclr;

	/*
	 * Go through the device tree and find out how many enclosures are
	 * there.
	 *
	 * A enclosure should have unique host id and channel numbers.
	 */
	for (i = 0; i < ssm_ndisks; i++) {
		if (ssm_disks[i]->sl_enclr) {
			/*
			 * Already been claimed, skipping...
			 */
			continue;
		}

		enclr = (ssm_enclr_t *)malloc(sizeof(ssm_enclr_t));
		if (!enclr) {
			fprintf(stderr,	"Failed to allocate memory for "
				"an enclosure structure.\n");
			return ENOMEM;
		}

		/*
		 * Get the first/next available disk and use its host no and
		 * channel number as identifiers of this enclosure.
		 */
		hostno = ssm_disks[i]->sl_paths[0]->sp_sgid.host_no;
		channel = ssm_disks[i]->sl_paths[0]->sp_sgid.channel;

		ssm_disks[i]->sl_enclr = (void *)enclr;

		/*
		 * Go through the rest of disks and mark each disk that has the
		 * same host no and channel no.
		 */
		count = 1;
		for (j = (i + 1); j < ssm_ndisks; j++) {
			if (ssm_disks[j]->sl_enclr) {
				/*
				 * Already been claimed, skipping...
				 */
				continue;
			}

			for (k = 0; k < ssm_disks[j]->sl_npaths; k++) {
				path = ssm_disks[j]->sl_paths[k];
				if (hostno == path->sp_sgid.host_no &&
				    channel == path->sp_sgid.channel) {
					count++;
					ssm_disks[j]->sl_enclr = (void *)enclr;
				}
			}
		}

		enclr->se_hostno = hostno;
		enclr->se_channel = channel;
		enclr->se_nluns = count;

		enclr->se_luns = (ssm_lun_t **)malloc(count *
						      sizeof(ssm_lun_t *));
		if (!enclr->se_luns) {
			fprintf(stderr, "Failed to allocate memory for "
				"ssm_disks.\n");
			free(enclr);
			return ENOMEM;
		}

		/*
		 * The first disk belong to this enclosure
		 */
		enclr->se_luns[0] = ssm_disks[i];

		/*
		 * Go through the disk list again and collect all the disks
		 * that had been marked belonging to this enclosure
		 */
		count = 1;
		for (j = (i + 1); j < ssm_ndisks; j++) {
			if (ssm_disks[j]->sl_enclr == (void *)enclr) {
				enclr->se_luns[count++] = ssm_disks[j];
			}
		}

		/*
		 * Put this enclosure on the global node (this node) structure
		 */
		if (this_node.sn_nenclrs == 0) {
			this_node.sn_nenclrs++;
			this_node.sn_enclrs = (ssm_enclr_t **)
				malloc(sizeof(ssm_enclr_t *));
		} else {
			this_node.sn_nenclrs++;
			this_node.sn_enclrs = (ssm_enclr_t **)
				realloc(this_node.sn_enclrs,
					(this_node.sn_nenclrs *
					 sizeof(ssm_enclr_t *)));
		}
		if (!this_node.sn_enclrs) {
			fprintf(stderr, "Failed to allocate memory for "
				"enclosures.\n");
			free(enclr->se_luns);
			free(enclr);
			return ENOMEM;
		}
		this_node.sn_enclrs[this_node.sn_nenclrs - 1] = enclr;
	}

	/*
	 * TODO: node related initialization needs to go to other routines
	 */
	this_node.sn_hostid = gethostid();

	return 0;
}

void
ssm_dump_node(ssm_node_t *node)
{
	int	i;

	fprintf(stdout, "\n\n");
	fprintf(stdout, "---------------------------------------------\n");
	fprintf(stdout, "Node: %lx\n", node->sn_hostid);
	fprintf(stdout, "---------------------------------------------\n\n");

	for (i = 0; i < node->sn_nenclrs; i++) {
		fprintf(stdout, "Enclosure %d:\n\n", i);
		ssm_dump_devices(node->sn_enclrs[i]->se_luns,
				 node->sn_enclrs[i]->se_nluns);
		fprintf(stdout, "\n");
	}
}
