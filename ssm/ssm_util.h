/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#ifndef SSM_UTIL_H
#define SSM_UTIL_H

#include "ssm_inc.h"

#define MAXGUIDSTR			39

#define	ERR_UUID_CREATE_SUCCESS		0
#define	ERR_UUID_CANNOT_ACQUIRE_LOCK	1
#define	ERR_UUID_CANNOT_READ_STATE	2
#define	ERR_UUID_CANNOT_WRITE_STATE	3

#if !defined(DIRECTORY_MODES)
#define DIRECTORY_MODES		(S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#endif	/* !DIRECTORY_MODES */

/*
 * set the following to the number of 100ns ticks of the actual
 * resolution of
 * your system's clock
 */
#define UUIDS_PER_TICK 1024

#define SSM_CONFIG_DIR			"/etc/ssm"
#define SIG_UUID_SEM_KEY_FILE		"/etc/ssm/.uuid_sem_lock"
#define STATE_FILE			"/etc/ssm/guid.state"

/*
 * Generate a random number between 0 to (x-1)
 */
#define	ssm_random_within(x)		((int)random() % (x))
/*
 * Generate a random number between x and y, including x and y
 */
#define ssm_random_between(x, y)	\
	((int)(x) + ssm_random_within((y) - (x) + 1))

/*
 * union required by semctl() call
 */
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
};

typedef uint64_t	uuid_time_t;
typedef struct {
	char nodeID[6];
} uuid_node_t;

typedef struct ssm_uuid {
	uint32_t	time_low;
	uint16_t	time_mid;
	uint16_t	time_hi_and_version;
	uint8_t		clock_seq_hi_and_reserved;
	uint8_t		clock_seq_low;
	uint8_t		node[6];
} ssm_uuid_t;

typedef struct ssm_guid {
	uint32_t  Data1;
	uint16_t  Data2;
	uint16_t  Data3;
	uint8_t   Data4[8];
} ssm_guid_t;

extern	int ssm_config_dir();
extern	void ssm_seed_random();
extern	int ssm_guid_create(ssm_guid_t *guid);
extern	char *ssm_guidtostr(ssm_guid_t guid, char *bufp);
extern	void ssm_print_bitmap(FILE *fd, ssm_bitmap_t *bmap, off_t offset,
			      size_t len);
extern	void ssm_print_buf(uint8_t *buf, int len);
extern	void ssm_print_vec_buf(uint8_t **s, int k, int m);
extern	void ssm_print_u8xu8(uint8_t *s, int k, int m);

#endif /* SSM_UTIL_H */
