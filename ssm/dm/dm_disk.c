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
#include <sysfs/libsysfs.h>
#include <linux/kdev_t.h>
#include <errno.h>

#define WWID_SIZE		64
#define STATE_SIZE		64
#define BLK_DEV_SIZE		33
#define NODE_NAME_SIZE		19
#define FILE_NAME_SIZE		256
#define SYSFS_PATH_SIZE		255
#define DEFAULT_CONFIGFILE	"/etc/multipath.conf"

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

struct s4_sgid {
	int host_no;
	int channel;
	int scsi_id;
	int lun_id;
};

typedef struct s4_path {
	char		sp_name[BLK_DEV_SIZE];
	int64_t		sp_devno;
	int		sp_state;
	struct s4_sgid	sp_sgid;
} s4_path_t;

typedef struct s4_lun {
	char		sl_mpname[NODE_NAME_SIZE];	/* lun name */
	char		sl_dmname[NODE_NAME_SIZE];	/* dm name */
	char		sl_uuid[WWID_SIZE];		/* uuid */
	int		sl_size;			/* in GB */
	int64_t		sl_devno;			/* devno */
	int		sl_state;

	s4_path_t	**sl_paths;
	int		sl_npaths;
} s4_lun_t;

typedef struct s4_enclr {
	int		se_nluns;
	s4_lun_t	**se_luns;
} s4_enclr_t;

typedef struct s4_node {
	int		sn_nenclrs;
	s4_enclr_t	**sn_enclrs;
} s4_node_t;


char		sysfs_path[FILE_NAME_SIZE];
s4_lun_t	**s4_disks;
int		s4_ndisks = 0;

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

static int
get_paths(s4_lun_t *disk)
{
	int	ret = ENODEV, host, channel, scsi_id, lun_id, major, minor;
	FILE	*fpipe;
	char	multipath_cmd[FILE_NAME_SIZE];
	char	line[FILE_NAME_SIZE], *lp;
	char	mpname[NODE_NAME_SIZE], dmname[NODE_NAME_SIZE];
	char	devname[NODE_NAME_SIZE];
	char	uuid[WWID_SIZE], *uuidp, dontcare[FILE_NAME_SIZE];
	char	state[STATE_SIZE];
	s4_path_t	*path;

	sprintf(multipath_cmd, "/sbin/multipath -l %s", disk->sl_mpname);

	if (!(fpipe = (FILE*)popen(multipath_cmd, "r"))) {
		perror("Problem with multipath command");
		return 1;
	}

	while (fgets(line, sizeof(line), fpipe)) {
		if (strncmp(line, "mpath", 5) == 0){
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

			path = (s4_path_t *)malloc(sizeof(s4_path_t));
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

			if (disk->sl_npaths == 0) {
				disk->sl_npaths++;
				disk->sl_paths = (s4_path_t **)
					malloc(sizeof(s4_path_t *));
			} else {
				disk->sl_npaths++;
				disk->sl_paths = (s4_path_t **)
					realloc(disk->sl_paths,								disk->sl_npaths * sizeof(s4_path_t *));
			}
			if (!disk->sl_paths) {
				fprintf(stderr, "Failed to allocate memory for "
					"disk->sl_paths.\n");
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

int
s4_device_discovery()
{
	int		ret;
	FILE		*fpipe;
	char		*dmsetup_cmd = "dmsetup ls --target multipath";
	char		line[FILE_NAME_SIZE], mpname[NODE_NAME_SIZE];
	int		major, minor;
	s4_lun_t	*disk;

	if (!(fpipe = (FILE*)popen(dmsetup_cmd, "r"))) {
		perror("Problem with dmsetup command");
		return 1;
	}

	while (fgets(line, sizeof(line), fpipe)) {
		disk = (s4_lun_t *)malloc(sizeof(s4_lun_t));
		if (!disk){
			fprintf(stderr,	"Failed to allocate memory for "
				"a disk structure.\n");
			ret = ENOMEM;
			break;
		}

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

		ret = get_paths(disk);
		if (ret) {
			fprintf(stderr, "Failed to read disk name and (major, "
				"minor).\n");
			disk->sl_state = DSTATE_FAILED;
		} else {
			disk->sl_state = DSTATE_ACTIVE;
		}

		if (s4_ndisks == 0) {
			s4_ndisks++;
			s4_disks = (s4_lun_t **)malloc(sizeof(s4_lun_t *));
		} else {
			s4_ndisks++;
			s4_disks = (s4_lun_t **)realloc(s4_disks,
				(s4_ndisks * sizeof(s4_lun_t *)));
		}
		if (!s4_disks){
			fprintf(stderr, "Failed to allocate memory for "
				"s4_disks.\n");
			ret = ENOMEM;
			break;
		}
		s4_disks[s4_ndisks - 1] = disk;
	}

	pclose(fpipe);

	return ret;
}

void
s4_device_dump()
{
	int		i, j;
	s4_lun_t	*disk;
	s4_path_t	*path;

	for (i = 0; i < s4_ndisks; i++) {
		disk = s4_disks[i];

		fprintf(stdout, "mpname: %s dmname: %s uuid: %s\n",
			disk->sl_mpname, disk->sl_dmname, disk->sl_uuid);

		fprintf(stdout, "size: %d GB (%ld:%ld) state: %s\n",
			disk->sl_size, MAJOR(disk->sl_devno),
			MINOR(disk->sl_devno),
			(disk->sl_state == DSTATE_ACTIVE) ? "active" :
			((disk->sl_state == DSTATE_FAILED) ? "failed" :
			 "undefined"));

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

int
main(int argc, char *argv[])
{
	int ret = 0;

	if (getuid() != 0) {
		fprintf(stderr, "Only root can run this command.\n");
		exit(1);
	}

	if (sysfs_get_mnt_path(sysfs_path, FILE_NAME_SIZE)) {
		fprintf(stderr, "sysfs must be mounted");
		exit(1);
	}

	ret = s4_device_discovery();

	s4_device_dump();

	return ret;
}
