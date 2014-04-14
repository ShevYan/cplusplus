/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#include "ssm_inc.h"

#define	MMAX	20
#define	KMAX	20

int	ssm_data_nvecs;
int	ssm_parity_nvecs;

uint8_t	*gf_tbls_enc;

/*
 * Initialize erasure code related global variables and prepare GF(2^8)
 * matrix for encoding
 */
int
ssm_ec_init()
{
	uint8_t	*gen_matrix;
	int	m, k;

	ssm_data_nvecs = SSM_DATA_NVECS_DEFAULT;
	ssm_parity_nvecs = SSM_PARITY_NVECS_DEFAULT;

	m = ssm_data_nvecs + ssm_parity_nvecs;
	k = ssm_data_nvecs;

	gen_matrix = (uint8_t *)malloc(m * k);
	gf_tbls_enc = (uint8_t *)malloc(m * k * 32);
	if (!gen_matrix || !gf_tbls_enc) {
		fprintf(stderr, "Failed to allocate memory for generator "
			"matrix or gf tables.\n");

		return 1;
	}

	/*
	 * Generator Matrix
	 */
	gf_gen_rs_matrix(gen_matrix, m, k);

	/*
	 * Initialize GF(2^8) transformation matrix
	 */
	ec_init_tables(k, m-k, &gen_matrix[k*k], gf_tbls_enc);

	printf("\nGenerator Matrix:\n\n");
	ssm_print_u8xu8(gen_matrix, m, k);

	return 0;
}

/*
 * Erasure coding the io passed in
 */
int
ssm_ec_encode(ssm_io_t *io, char *net_io_cxt)
{
	/*
	 * io passed in should have parity vectors pre-allocated
	 */
	if (io->sb_iovec[io->sb_dvecs] == NULL) {
		return 1;
	}

	printf("\ndata buffer BEFORE Encoding: buffs %d x %ld:\n\n",
	       io->sb_dvecs, io->sb_vec_len);
	sprintf(net_io_cxt+strlen(net_io_cxt), "\ndata buffer BEFORE Encoding: buffs %d x %ld:\n\n",
		       io->sb_dvecs, io->sb_vec_len);
	ssm_print_vec_buf(io->sb_iovec, io->sb_dvecs + io->sb_pvecs, 25, net_io_cxt);

	/*
	 * Using SSE version
	 */
	ec_encode_data_sse(io->sb_vec_len, io->sb_dvecs, io->sb_pvecs,
			   gf_tbls_enc,
			   io->sb_iovec, &io->sb_iovec[io->sb_dvecs]);

	printf("\ndata buffer AFTER Encoding: buffs %d x %ld:\n\n",
	       io->sb_dvecs, io->sb_vec_len);
	sprintf(net_io_cxt+strlen(net_io_cxt), "\ndata buffer AFTER Encoding: buffs %d x %ld:\n\n",
		       io->sb_dvecs, io->sb_vec_len);
	ssm_print_vec_buf(io->sb_iovec, io->sb_dvecs + io->sb_pvecs, 25, net_io_cxt);

	return 0;
}
