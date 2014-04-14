/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#ifndef SSM_ERASURE_CODE_H
#define SSM_ERASURE_CODE_H

#include "ssm_inc.h"

#define	SSM_DATA_NVECS_DEFAULT		8
#define	SSM_PARITY_NVECS_DEFAULT	2

extern	int ssm_data_nvecs;
extern	int ssm_parity_nvecs;

extern	int ssm_ec_init();
extern	int ssm_ec_encode(ssm_io_t *io, char *net_io_cxt);

#endif /* SSM_ERASURE_CODE_H */
