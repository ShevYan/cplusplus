/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#ifndef SSM_IO_H
#define SSM_IO_H

#include "ssm_inc.h"

typedef	struct ssm_handle {
	ssm_guid_t	ctn_guid;
	off_t		offset;
	size_t		length;
	uint32_t	crc32;
} ssm_handle_t;

extern	int ssm_submit_io(ssm_container_t *container, ssm_io_t *io);
extern	int ssm_dump_io(ssm_io_t *io, uint64_t seqno);

#endif /* SSM_IO_H */
