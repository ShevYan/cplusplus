/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#include "ssm_inc.h"

static int
ssm_disk_io(ssm_lun_t *disk, off_t offset, size_t len, uint8_t *buf)
{
	char	dev[FILE_NAME_SIZE];
	int	fd, nbytes, ret = 0;

	sprintf(dev, "/dev/%s", disk->sl_dmname);

	fd = open(dev, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		fprintf(stderr, "Failed to open device %s.\n", dev);
		ret = ENODEV;
		goto out;
	}

	lseek(fd, offset, SEEK_SET);

	nbytes = write(fd, buf, len);
	if (nbytes != len) {
		fprintf(stderr, "Failed to open device %s.\n", dev);
		ret = EIO;
	}

	close(fd);

#if 0
	ssm_print_buf(buf, 25);
#endif

out:
	return ret;
}

static int
ssm_vec_io(ssm_column_t *col, off_t ctn_offset, size_t len, uint8_t *iovec, char *net_io_cxt)
{
	ssm_lun_t	*disk;
	int		ret;
	off_t		disk_offset, offset;
	off_t		oneGB = 1024 * 1024 * 1024, oneMB = 1024 * 1024;

	disk = col->sc_disk;

	/*
	 * Disk offset is in GB
	 */
	disk_offset = disk->sl_blksz * col->sc_seg * oneGB;

	/*
	 * Container offset is in MB
	 */
	offset = disk_offset + ctn_offset * oneMB;

	fprintf(stdout, "%lX: %s: %d offset: %ld length: %ld\n",
		col->sc_node->sn_hostid, col->sc_disk->sl_dmname,
		col->sc_seg, offset, len);

	sprintf(net_io_cxt+strlen(net_io_cxt), "%lX: %s: %d offset: %ld length: %ld\n",
			col->sc_node->sn_hostid, col->sc_disk->sl_dmname,
			col->sc_seg, offset, len);
	ret = ssm_disk_io(disk, offset, len, iovec);

	return ret;
}

/*
 * Process I/O against a container
 */
int
ssm_submit_io(ssm_container_t *ctn, ssm_io_t *io, char *net_io_cxt)
{
	int	i, nbits, ret = 0, oneMB = 1024 * 1024;
	off_t	bit_offset, ctn_offset;
	size_t	lenMB;
	char	guidbuf[MAXGUIDSTR];
	ssm_bitmap_t	*bmap = ctn->sc_space_bmap;

	if (ctn->sc_ncols != io->sb_dvecs + io->sb_pvecs) {
		fprintf(stderr, "Container columns don't match io vectors.\n");
		ret = EIO;
		goto out;
	}

	fprintf(stdout, "I/O on container: %s\n\t   Owner: %lX\n\n",
		ssm_guidtostr(ctn->sc_guid, guidbuf),
		ctn->sc_owner);
	sprintf(net_io_cxt+strlen(net_io_cxt), "I/O on container: %s\n\t   Owner: %lX\n\n",
			ssm_guidtostr(ctn->sc_guid, guidbuf),
			ctn->sc_owner);

	lenMB = (io->sb_vec_len + oneMB - 1) / oneMB;
	nbits = (lenMB + ctn->sc_unitsz - 1) / ctn->sc_unitsz;

	bit_offset = bmap_find_clear_range(bmap->sb_bitmap,
					   bmap->sb_nbit, 0, nbits);
	ctn_offset = bit_offset * ctn->sc_unitsz;

	for (i = 0; i < ctn->sc_ncols; i++){
		fprintf(stdout, "col %d: ", i);
		sprintf(net_io_cxt+strlen(net_io_cxt), "col %d: ", i);
		ret = ssm_vec_io(&ctn->sc_col[i], ctn_offset, io->sb_vec_len,
				 io->sb_iovec[i], net_io_cxt);
		if (ret) {
			fprintf(stderr, "Failed to write vector %d to disk.\n",
				i);
			goto out;
		}
	}

	bmap_set_range(bmap->sb_bitmap, bit_offset, nbits);

out:
	return ret;
}

/*
 * Dump I/O to files
 * Files generated will be
 * d_$suffix.col0
 * ...
 * d_$suffix.col7
 * p_$suffix.out
 * q_$suffix.out
 */
int
ssm_dump_io(ssm_io_t *io, uint64_t seqno)
{
	int	i, ret = 0;
	char	filename[FILE_NAME_SIZE];
	FILE	*fd;

	/*
	 * col 0 - col 7
	 */
	for (i = 0; i < io->sb_dvecs; i++){
		sprintf(filename, "d_%ld.col%d", seqno, i);
		fprintf(stdout, "dumping col %d to file %s...\n", i, filename);

		fd = fopen(filename, "w");
		if (fd == NULL) {
			fprintf(stderr, "Failed to open %s.\n", filename);
			ret = errno;
			goto out;
		}

		fwrite(io->sb_iovec[i], io->sb_vec_len, 1, fd);
		fflush(fd);
		fclose(fd);
	}

	/*
	 * p column
	 */
	sprintf(filename, "p_%ld.out", seqno);
	fprintf(stdout, "dumping col p to file %s...\n", filename);

	fd = fopen(filename, "w");
	if (fd == NULL) {
		fprintf(stderr, "Failed to open %s.\n", filename);
		ret = errno;
		goto out;
	}

	fwrite(io->sb_iovec[io->sb_dvecs], io->sb_vec_len, 1, fd);
	fflush(fd);
	fclose(fd);

	/*
	 * q column
	 */
	sprintf(filename, "q_%ld.out", seqno);
	fprintf(stdout, "dumping col q to file %s...\n", filename);

	fd = fopen(filename, "w");
	if (fd == NULL) {
		fprintf(stderr, "Failed to open %s.\n", filename);
		ret = errno;
		goto out;
	}

	fwrite(io->sb_iovec[io->sb_dvecs + 1], io->sb_vec_len, 1, fd);
	fflush(fd);
	fclose(fd);

out:
	return ret;
}
