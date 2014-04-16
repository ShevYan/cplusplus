/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#include "ssm_inc.h"

#define	CONTROLLED_TEST
#define GRACE_DEMO1_03_14
#define WAIT_FOR_SIGNAL

char		sysfs_path[FILE_NAME_SIZE];

#ifdef	GRACE_DEMO1_03_14

int usr_interrupt;

void
sig_handler(int signo)
{
	if (signo == SIGUSR1) {
		usr_interrupt = 1;
	}
}

void
test_wait_for_signal()
{
	fprintf(stdout, "\nwait for SIGUSR1 signal ...\n");

#ifdef	WAIT_FOR_SIGNAL
	while (!usr_interrupt) {
		sleep(1);
	}
#endif
	usr_interrupt = 0;
}

#endif

int
ssm_init()
{
	int	ret;

	ret = ssm_discover_devices();
	if (ret != 0) {
		fprintf(stderr, "Failed to discover devices");
		goto out;
	}

	/* ssm_dump_devices(ssm_disks, ssm_ndisks); */

	ssm_dump_node(&this_node);

	ret = ssm_global_resource();
	if (ret != 0) {
		fprintf(stderr, "Failed to build global resource tree");
		goto out;
	}

	ret = ssm_build_container_table();
	if (ret != 0) {
		fprintf(stderr, "Failed to build container table");
		goto out;
	}

out:
	return ret;
}

ssm_io_t *
test_generate_io(int start)
{
	int		i, j, nMB, nbytes, m;
	ssm_io_t	*io;
	void		*buf;

	/*
	 * Generate a buffer between 8M to 256 MB
	 */
#ifdef CONTROLLED_TEST
	nMB = 32;
#else
	nMB = ssm_random_between(ssm_ctn_unit_sz * 8, 256);
#endif

	nbytes = nMB * 1024 * 1024;

	io = (ssm_io_t *)malloc(sizeof(ssm_io_t));
	if (!io) {
		fprintf(stdout, "Failed to allocate io.\n");
		return NULL;
	}

	io->sb_flags = 0;
	io->sb_rw = SSM_IO_WRITE;
	io->sb_size = nbytes;
	io->sb_dvecs = ssm_data_nvecs;
	io->sb_pvecs = ssm_parity_nvecs;
	io->sb_vec_len = (nMB + ssm_data_nvecs - 1) / ssm_data_nvecs;
	io->sb_vec_len = io->sb_vec_len * 1024 * 1024;

	m = ssm_data_nvecs + ssm_parity_nvecs;
	io->sb_iovec = (uint8_t **)malloc(m * sizeof(uint8_t *));
	if (!io->sb_iovec) {
		fprintf(stdout, "Failed to allocate io_vec.\n");
		free(io);
		return NULL;
	}
	memset(io->sb_iovec, 0, m);

	for (i = 0; i < ssm_data_nvecs; i++) {
		if (posix_memalign(&buf, 64, io->sb_vec_len)) {
			fprintf(stdout, "Failed to allocate io_vec.\n");
			goto err_out;
		}

		io->sb_iovec[i] = buf;

		for (j = 0; j < io->sb_vec_len; j++) {
			io->sb_iovec[i][j] = (uint8_t)((start) & 0x00ff);
		}
	}

	for (i = 0; i < ssm_parity_nvecs; i++) {
		if (posix_memalign(&buf, 64, io->sb_vec_len)) {
			fprintf(stdout, "Failed to allocate io_vec.\n");
			goto err_out;
		}

		io->sb_iovec[ssm_data_nvecs + i] = buf;
	}

	return io;

err_out:
	for (i = 0; i < io->sb_dvecs; i++) {
		if (io->sb_iovec[i]) {
			free(io->sb_iovec[i]);
		}
	}

	free(io->sb_iovec);
	free(io);
	return NULL;
}

#ifdef	GRACE_DEMO1_03_14
ssm_io_t *
test_read_io_from_file(char *filename)
{
	int		i, m, nMB, oneMB = 1024 * 1024;
	ssm_io_t	*io = NULL;
	size_t		readlen, size;
	FILE		*fd;
	void		*buf;

	fd = fopen(filename, "r");
	if (fd == NULL) {
		fprintf(stderr, "Failed to open %s.\n", filename);
		free(io);
		return NULL;
	}

	fseek(fd, 0L, SEEK_END);
	size = ftell(fd);
	fseek(fd, 0L, SEEK_SET);

	nMB = (size + oneMB - 1) / oneMB;

	io = (ssm_io_t *)malloc(sizeof(ssm_io_t));
	if (!io) {
		fprintf(stdout, "Failed to allocate io.\n");
		return NULL;
	}

	io->sb_flags = 0;
	io->sb_rw = SSM_IO_WRITE;
	io->sb_size = size;
	io->sb_dvecs = ssm_data_nvecs;
	io->sb_pvecs = ssm_parity_nvecs;
	io->sb_vec_len = (nMB + ssm_data_nvecs - 1) / ssm_data_nvecs;
	io->sb_vec_len = io->sb_vec_len * 1024 * 1024;

	m = ssm_data_nvecs + ssm_parity_nvecs;
	io->sb_iovec = (uint8_t **)malloc(m * sizeof(uint8_t *));
	if (!io->sb_iovec) {
		fprintf(stdout, "Failed to allocate io_vec.\n");
		free(io);
		return NULL;
	}
	memset(io->sb_iovec, 0, m);

	for (i = 0; i < ssm_data_nvecs; i++) {
		if (posix_memalign(&buf, 64, io->sb_vec_len)) {
			fprintf(stdout, "Failed to allocate io_vec.\n");
			goto err_out;
		}

		io->sb_iovec[i] = buf;

		readlen = fread(buf, io->sb_vec_len, 1, fd);
		if (readlen != 1) {
			fprintf(stdout, "Failed to read io_vec.\n");
			goto err_out;
		}
	}

	for (i = 0; i < ssm_parity_nvecs; i++) {
		if (posix_memalign(&buf, 64, io->sb_vec_len)) {
			fprintf(stdout, "Failed to allocate io_vec.\n");
			goto err_out;
		}

		io->sb_iovec[ssm_data_nvecs + i] = buf;
	}

	return io;

err_out:
	for (i = 0; i < io->sb_dvecs; i++) {
		if (io->sb_iovec[i]) {
			free(io->sb_iovec[i]);
		}
	}

	free(io->sb_iovec);
	free(io);
	return NULL;
}

#endif

int
test_ssm_io(int ctn_idx, ssm_io_t *io, uint64_t seqno)
{
	int		ret = 0;
	char		filename[FILE_NAME_SIZE];
	ssm_container_t	*ctn;

	ret = ssm_ec_encode(io);
	if (ret) {
		goto out;
	}

	if (ctn_idx == -1) {
		ctn = ssm_select_container(io->sb_vec_len);
	} else {
		ctn = ssm_select_container_by_index(io->sb_vec_len,
							  ctn_idx);
	}
	if (!ctn){
		fprintf(stderr, "No container left to accommadate I/O.\n");
		ret = EIO;
		goto out;
	}

#ifdef	GRACE_DEMO1_03_14
	ssm_dump_io(io, seqno);

	sprintf(filename, "bmap_%lu.before", seqno);
	fprintf(stdout, "\ndumping bitmap(BEFORE I/O: %ld bits) to file "
		"%s...\n\n", ctn->sc_space_bmap->sb_nbit, filename);
	ssm_dump_container_bitmap(filename, ctn);

	ret = ssm_submit_io(ctn, io);
	if (ret) {
		goto out;
	}

	sprintf(filename, "bmap_%lu.after", seqno);
	fprintf(stdout, "\ndumping bitmap(AFTER I/O: %ld bits) to file "
		"%s...\n\n", ctn->sc_space_bmap->sb_nbit, filename);
	ssm_dump_container_bitmap(filename, ctn);

#else
	ret = ssm_submit_io(ctn, io);
	if (ret) {
		goto out;
	}
#endif
	fflush(stdout);

out:
	return ret;
}

int
main(int argc, char *argv[])
{
	int		ret = 0;
	ssm_io_t	*io;

	ret = ssm_config_dir();
	if (ret != 0) {
		fprintf(stderr, "Failed to create ssm config directory.\n");
		exit(1);
	}

	ssm_seed_random();

	if (getuid() != 0) {
		fprintf(stderr, "Only root can run this command.\n");
		exit(1);
	}

	if (sysfs_get_mnt_path(sysfs_path, FILE_NAME_SIZE)) {
		fprintf(stderr, "sysfs must be mounted.\n");
		exit(1);
	}

	ret = ssm_ec_init();
	if (ret != 0) {
		fprintf(stderr, "Failed to initialize EC.\n");
		goto out;
	}

	ret = ssm_init();
	if (ret != 0) {
		fprintf(stderr, "Failed to initialize ssm.\n");
		goto out;
	}

	fprintf(stdout, "\nInitialization Complete.\n\n");

#ifdef	GRACE_DEMO1_03_14

	if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
		fprintf(stderr, "Can't catch SIGUSR1.\n");
		ret = 1;
		goto out;
	}

	ssm_dump_container("./container.map", &ssm_ctn_tbl);

	fflush(stdout);
	test_wait_for_signal();

	io = test_read_io_from_file("./file_1_32M");
	if (!io) {
		ret = EIO;
		goto out;
	}
	ret = test_ssm_io(-1, io, 0);
	fflush(stdout);
	test_wait_for_signal();

	io = test_read_io_from_file("./file_2_48M");
	if (!io) {
		ret = EIO;
		goto out;
	}
	ret = test_ssm_io(5, io, 1);
	fflush(stdout);
	test_wait_for_signal();

	io = test_read_io_from_file("./file_3_64M");
	if (!io) {
		ret = EIO;
		goto out;
	}
	ret = test_ssm_io(5, io, 2);
	fflush(stdout);

#else
	io = test_generate_io(1);
	ret = test_ssm_io(-1, io, 0);

	io = test_generate_io(2);
	ret = test_ssm_io(5, io, 0);

	io = test_generate_io(3);
	ret = test_ssm_io(5, io, 0);
#endif

out:
	return ret;
}