/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#include <sysfs/libsysfs.h>

#include "ssm_discovery.h"

char		sysfs_path[FILE_NAME_SIZE];

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

	ret = ssm_discover_devices();
	if (ret != 0) {
		fprintf(stderr, "Failed to discover devices");
		exit(1);
	}

	ssm_dump_devices(ssm_disks, ssm_ndisks);

	ret = ssm_organize_devices();
	if (ret != 0) {
		fprintf(stderr, "Failed to organize devices");
		exit(1);
	}

	ssm_dump_node(&this_node);

	return ret;
}
