/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#ifndef SSM_BUF_H
#define SSM_BUF_H

#include "ssm_inc.h"

#define	SSM_IO_READ	0
#define	SSM_IO_WRITE	1

struct ssm_buf {
	uint32_t		sb_flags;
	uint32_t		sb_rw;		/* READ/WRITE */
	size_t			sb_size;	/* total size in bytes */

	int			sb_dvecs;	/* number of data vecs */
	int			sb_pvecs;	/* number of parity vecs */
	size_t			sb_vec_len;	/* vector length in bytes */
	uint8_t			**sb_iovec;	/* actual vec list */
};

typedef struct ssm_buf		ssm_io_t;


#endif /* SSM_BUF_H */
