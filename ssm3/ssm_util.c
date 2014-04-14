/* $Id$ */
/* #ident "$Source$ $Revision$" */

/*
 * $Copyright$
 */

#include "ssm_inc.h"

#define	SSM_USE_RANDOM

/*
 * data type for UUID generator persistent state
 */
typedef struct {
	uuid_time_t ts;		/* saved timestamp */
	uuid_node_t node;	/* saved node ID */
	uint16_t cs;		/* saved clock sequence */
} uuid_state;

static uuid_state st;

/*
 * read_state -- read UUID generator state from non-volatile store
 */
int
read_state(clockseq, timestamp, node)
	uint16_t	 *clockseq;
	uuid_time_t	 *timestamp;
	uuid_node_t	 *node;
{
	FILE		*fd;
	static int	inited = 0;

	/*
	 * only need to read state once per boot
	 */
	if (!inited) {
		fd = fopen(STATE_FILE, "rb");
		if (!fd) {
			return (0);
		}

		if (fread(&st, sizeof(uuid_state), 1, fd) == 0) {
			/*
			 * If you can't read the state successfully,
			 * return error
			 */
			fclose(fd);
			return (0);
		}
		fclose(fd);
		inited = 1;
	};
	*clockseq = st.cs;
	*timestamp = st.ts;
	*node = st.node;
	return(1);
}

/*
 * write_state -- save UUID generator state back to non-volatile
 * storage
 */
int
write_state(clockseq, timestamp, node)
	uint16_t clockseq;
	uuid_time_t timestamp;
	uuid_node_t node;
{
	FILE		*fd;
	static int	inited = 0;
	static uuid_time_t next_save;

	if (!inited) {
		next_save = timestamp;
		inited = 1;
	};
	/*
	 * always save state to volatile shared state
	 */
	st.cs = clockseq;
	st.ts = timestamp;
	st.node = node;
	if (timestamp >= next_save) {

		/*
		 * This code will create the state file (with owner +rw
		 * permissions and user/group +r permissions if it
		 * doesn't exist already)
		 */
		int filedesc = open(STATE_FILE, O_RDWR);
		if (filedesc == -1) {
			filedesc = open(STATE_FILE, O_RDWR | O_CREAT, 0644);
			/*
			 * Return error if you can't create the state file
			 */
			if (filedesc == -1)
				return 0;

			close(filedesc);
			chmod(STATE_FILE, 0644);
		}
		else
			close(filedesc);

		/*
		 * Proceed with the writing of state file now
		 */

		fd = fopen(STATE_FILE, "wb");
		if (!fd) {
			/*
			 * Return error if file still can't be opened
			 */
			return 0;
		}

		if (fwrite(&st, sizeof(uuid_state), 1, fd) == 0) {
			/*
			 * Return error if you can't write to the file
			 */
			fclose(fd);
			return 0;
		}

		fclose(fd);

		/*
		 * schedule next save for 10 seconds from now
		 */
		next_save = timestamp + (10 * 10 * 1000 * 1000);
	};
	return 1;
}

void
ssm_seed_random()
{
#ifdef	SSM_USE_RANDOM
	struct timeval tm;

	gettimeofday(&tm, NULL);
	srandom(tm.tv_sec + tm.tv_usec * 1000000ul);
#else
	srandom(1);
#endif
}

int
ssm_config_dir()
{
	int	ret;
	struct stat statbuf;

	ret = lstat(SSM_CONFIG_DIR, &statbuf);
	if (ret < 0) {
		ret = mkdir(SSM_CONFIG_DIR, DIRECTORY_MODES);
		if (ret < 0) {
			fprintf(stderr, "mkdir: Failed to make directory %s\n",
				SSM_CONFIG_DIR);
		}
	}

	return ret;
}

int
getMacAddress(pNode)
	uuid_node_t *pNode;
{
	int s, ret;
	int rc = 0;
	struct arpreq ar;
	/*
	 * Determine how many network interfaces we have
	 */
	int cif = 0;

	memset(&ar, 0, sizeof(ar));

	if (!pNode) {
		return rc;
	}
	memset(pNode, 0, 6); /* zero out 6 bytes */

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		return rc;
	}

	/*
	 * SIOCGIFNUM is not supported
	 * Assume arbitrary number for the maximum number of interfaces
	 * instead of asking the OS.
	 *
	 * ret = ioctl(s, SIOCGIFNUM, &cif);
	 */
	ret = 0;    /* always succeeds */
	cif = 8;    /* Assume maximum 8 interfaces */

	if (ret != -1) {
		struct ifconf ifc = {0};
		char *ifc_buffer = 0;

		if (cif) {
			ifc.ifc_len = cif * sizeof(struct ifreq);
			ifc_buffer = (char *)
				      malloc( ifc.ifc_len * sizeof(char));
			if (ifc_buffer) {
				memset(ifc_buffer, 0, ifc.ifc_len);
				ifc.ifc_buf = ifc_buffer;
				ret = ioctl(s, SIOCGIFCONF, &ifc);

				if (ret != -1) {
					int i;
					struct ifreq ifr_temp;
					for ( i = 0; i < cif; ++i) {
					   memset(&ar, 0, sizeof(ar));
					   ifr_temp = ifc.ifc_req[i];

					   ar.arp_pa.sa_family = AF_INET;
					   ar.arp_pa =
					      ifc.ifc_req[i].ifr_ifru.ifru_addr;

					   ar.arp_ha.sa_family = AF_UNSPEC;
					   ret = ioctl(s, SIOCGARP, &ar);
					   if (ret != -1) {
						/*
						 * make sure it is not
						 * the loopback
						 */
						memcpy(pNode,
						       ar.arp_ha.sa_data, 6);
						rc = 1;
						break;
					   }
					}
				}
			}
		}
		free(ifc_buffer);
	}
	close(s);
	return rc;
}


/*
 * system dependent call to get IEEE node ID.
 * This sample implementation generates a random node ID
 */
int
get_ieee_node_identifier(node)
	uuid_node_t *node;
{
	static int inited = 0;
	static uuid_node_t saved_node;

	if (!inited) {
		if (getMacAddress(&saved_node)) {
			inited = 1;
		}
		else {
			return 0;
		}
	};

	*node = saved_node;
	return (1);
}

void
get_system_time(uuid_time)
	uuid_time_t *uuid_time;
{
	struct timeval tp;

	gettimeofday(&tp, (struct timezone *)0);

	/*
         * Offset between UUID formatted times and Unix formatted times.
         * UUID UTC base time is October 15, 1582.
         * Unix base time is January 1, 1970.
         */
	*uuid_time = (uint64_t)tp.tv_sec * 10000000 +
		(uint64_t)tp.tv_usec * 10 + 0x01B21DD213814000ULL;
}

/*
 * get-current_time -- get time as 60 bit 100ns ticks since whenever.
 * Compensate for the fact that real clock resolution is
 * less than 100ns.
 */
void
get_current_time(timestamp)
	uuid_time_t * timestamp;
{
	uuid_time_t	    time_now;
	static uuid_time_t  time_last;
	static uint16_t   uuids_this_tick;
	static int	    inited = 0;

	if (!inited) {
		get_system_time(&time_now);
		uuids_this_tick = UUIDS_PER_TICK;
		inited = 1;
	};

	while (1) {
		get_system_time(&time_now);

		/*
		 * if clock reading changed since last UUID generated...
		 */
		if (time_last != time_now) {
			/*
			 * reset count of uuids gen'd with this clock reading
			 */
			uuids_this_tick = 0;
			break;
		};
		if (uuids_this_tick < UUIDS_PER_TICK) {
			uuids_this_tick++;
			break;
		};
		/*
		 * going too fast for our clock; spin
		 */
	};
	/*
	 * add the count of uuids to low order bits of the clock reading
	 */
	*timestamp = time_now + uuids_this_tick;
}

/*
 * true_random -- generate a crypto-quality random number.
 * This sample doesn't do that.
 */
static uint16_t
true_random(void)
{
	static int inited = 0;
	uuid_time_t time_now;

	if (!inited) {
		get_system_time(&time_now);
		time_now = time_now/UUIDS_PER_TICK;
		srand((unsigned int)(((time_now >> 32) ^ time_now)&0xffffffff));
		inited = 1;
	};
	return (rand());
}

/*
 * true_random_advanced -- generate a crypto-quality random number.
 * This is based on lrand48() provided on UNIX platforms and isn't really
 * a crypto-quality random number.
 */
static long
true_random_advanced(void)
{
	static int inited = 0;
	uuid_time_t time_now;
	if (!inited)
	{
		get_system_time(&time_now);
		time_now = time_now/UUIDS_PER_TICK;
		srand48((long) (((time_now >> 32) ^ time_now) & 0xffffffff));
		inited = 1;
	};
	return (lrand48());
}

static void
fill_identifier_with_random_value(node)
	uuid_node_t *node;
{
	/*
	 * If for some reason you can't get MAC address,
	 * generate random number for the node ID field
	 */

	union randUnion_u {
		long randNumber;
		char bytes [4];
	} randUnion;
	uint16_t tmpRandomValue = true_random();

	randUnion.randNumber = true_random_advanced();

	/*
	 * Fill first 4 bytes of the node ID
	 */
	memcpy(node, randUnion.bytes, 4);

	tmpRandomValue = true_random();

	/*
	 * Fill rest 2 bytes of the node ID
	 */
	memcpy((char *) ((char *)node + 4), &tmpRandomValue, 2);
}

/*
 * format_uuid_v1 -- make a UUID from the timestamp, clockseq,
 * and node ID
 */

void
format_uuid_v1(uuid, clock_seq, timestamp, node)
	ssm_uuid_t *uuid;
	uint16_t clock_seq;
	uuid_time_t timestamp;
	uuid_node_t node;
{

	/*
	 * Construct a version 1 uuid with the information we've gathered
	 * plus a few constants.
	 */
	uuid->time_low = (unsigned long)(timestamp & 0xFFFFFFFF);
	uuid->time_mid = (unsigned short)((timestamp >> 32) & 0xFFFF);
	uuid->time_hi_and_version = (unsigned short)((timestamp >> 48) &
         0x0FFF);
	uuid->time_hi_and_version |= (1 << 12);
	uuid->clock_seq_low = clock_seq & 0xFF;
	uuid->clock_seq_hi_and_reserved = (clock_seq & 0x3F00) >> 8;
	uuid->clock_seq_hi_and_reserved |= 0x80;
	memcpy(&uuid->node, &node, sizeof uuid->node);
}

/*
 * uuid_create -- generator a UUID
 */

int
ssm_uuid_create(ssm_uuid_t *uuid)
{
	uuid_time_t timestamp, last_time;
	uint16_t clockseq = 0;
	uuid_node_t node;
	uuid_node_t last_node;
	int f;
	int retval = ERR_UUID_CREATE_SUCCESS;
	int semid; /* Id of the semaphore. */
	struct sembuf sem_lock;
	struct sembuf sem_unlock;
	union semun arg = {0};
	int fd;		/* file discriptor for the lock file */
	key_t key;

	/*
	 * locking argument.
	 */
	sem_lock.sem_num = 0; /* semaphore number.*/
	sem_lock.sem_op = -1; /* semaphore operation.*/
	sem_lock.sem_flg = SEM_UNDO; /* undo the operation if process exists.*/

	/*
	 * unlocking argument.
	 */
	sem_unlock.sem_num = 0; /* semaphore number. */
	sem_unlock.sem_op = 1; /* semaphore operation. */
	sem_unlock.sem_flg = SEM_UNDO;

	/*
	 * Set to nil UUID
	 */
	memset(uuid, 0x00, sizeof(ssm_uuid_t));

	/*
	 * find the key which is used for the semaphore lock
	 */
	if ((key = ftok(SIG_UUID_SEM_KEY_FILE, 'U')) == -1) {
		fd = open(SIG_UUID_SEM_KEY_FILE, O_CREAT | O_EXCL, 0644);
		if ( fd == -1 ) {
			return ERR_UUID_CANNOT_ACQUIRE_LOCK;
		}
		close (fd);
		if ((key = ftok(SIG_UUID_SEM_KEY_FILE, 'U')) == -1) {
			return ERR_UUID_CANNOT_ACQUIRE_LOCK;
		}
	}
	/*
	 * Create a semaphore if it is not there
	 */
	semid = semget((key_t)key, 1, IPC_EXCL | IPC_CREAT | 0666);

	if (semid == -1) {
		/*
		 * semaphore already exists. So get its id.
		 */
		semid = semget((key_t)key, 1, 0666);
	}
	else {
		/*
		 * Semaphore created. Initialise it to the unlocked state
		 */
		arg.val = 1;
		semctl(semid,0,SETVAL,arg);
	}

	/*
	 * Acquire the semaphore lock.
	 */
	semop(semid,&sem_lock,1);

	/*
	 * get current time
	 */
	get_current_time(&timestamp);

	/*
	 * get node ID
	 */
	if (!get_ieee_node_identifier(&node)) {
		fill_identifier_with_random_value(&node);
	}

	/*
	 * If the caller supplies a clock sequence, don't read/write to a file
	 * Assume that the user supplied clock sequence takes these conditions
	 * into account.
         */

	/*
	 * get saved state from NV storage
	 */
	f = read_state(&clockseq, &last_time, &last_node);

	if (!f) {
		retval = ERR_UUID_CANNOT_READ_STATE;
	}

	/*
	 * if no NV state, or if clock went backwards, or node ID changed
	 * (e.g., net card swap) change clockseq
	 */
	if (!f || memcmp(&node, &last_node, sizeof(uuid_node_t))) {
		clockseq = true_random();
	} else if (timestamp <= last_time) {
		clockseq++;
	}

	/*
	 * stuff fields into the UUID
	 */
	format_uuid_v1(uuid, clockseq, timestamp, node);

	/*
	 * If the caller supplies a clock sequence, don't save to a file
	 */

	/*
	 * save the state for next time
	 */
	if (write_state(clockseq, timestamp, node) == 0)
		retval = ERR_UUID_CANNOT_WRITE_STATE;

	/*
	 * Release the lock.
	 */
	semop(semid,&sem_unlock,1);
	return retval;
}

void
ConvertUuidToGuid(uuid, guid)
	ssm_uuid_t *uuid;
	ssm_guid_t *guid;
{
	int i;
	guid->Data1 = uuid->time_low;
	guid->Data2 = uuid->time_mid;
	guid->Data3 = uuid->time_hi_and_version;
	guid->Data4[0] = uuid->clock_seq_hi_and_reserved;
	guid->Data4[1] = uuid->clock_seq_low;
	for(i = 0; i < 6; i++) {
		guid->Data4[i+2] =  uuid->node[i];
	}
}

int
ssm_guid_create(guid)
	ssm_guid_t * guid;
{
	int status;
	int result;
	ssm_uuid_t uuid;

	status = ssm_uuid_create(&uuid);
	switch (status) {
        /*
         * VXERROR_CANNOT_READ_GUIDSTATE and VXERROR_CANNOT_WRITE_GUIDSTATE
         * are informational errors. A guid _is_ generated in these cases,
         * but the caller may or may not want to go ahead.
         */

	case ERR_UUID_CREATE_SUCCESS:
		result = 0;
		ConvertUuidToGuid(&uuid, guid);
		break;

	case ERR_UUID_CANNOT_READ_STATE:
		result = 0;
		ConvertUuidToGuid(&uuid, guid);
		break;

	case ERR_UUID_CANNOT_WRITE_STATE:
		result = 0;
		ConvertUuidToGuid(&uuid, guid);
		break;

	default:
		result = 1;
		break;
        /*
         * NOTE : You should also be throwing an exception or handling the
         * error that uuid was not generated.
         */
	}
	return result;
}


char *
ssm_guidtostr(ssm_guid_t guid, char *bufp)
{
	static char sbuf[MAXGUIDSTR];
	if (bufp == NULL)
		bufp = sbuf;
	sprintf(bufp,
		"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	return bufp;
}

/*
 * Print container space bitmap starting from 'offset' to the length of 'len'
 */
void
ssm_print_bitmap(FILE *fd, ssm_bitmap_t *bmap, off_t offset, size_t len)
{
	int	i, nbits;

	/*
	 * print 'len' bits at most, otherwise it would flood the
	 * termnal
	 */
	nbits = (bmap->sb_nbit > len) ? len : bmap->sb_nbit;
	for (i = 0; i < nbits; i++) {
		fprintf(fd, "%d ", bmap_test(bmap->sb_bitmap, i + offset));
	}

	fflush(fd);

	fprintf(fd, "\n\n");
}

/*
 * Print out a single char buffer limited by 'len'
 * Useful for printing a vector/column
 */
void
ssm_print_buf(uint8_t *buf, int len)
{
	int i;

	for (i = 0; i < len; ) {
		printf(" %2x", 0xff & buf[i++]);
		if (i % 32 == 0) {
			printf("\n");
		}
	}

	printf("\n");
}

/*
 * Print out a matrix buffer with limited elements
 * Used for printing data vectors (only printing 'm' elements)
 */
void
ssm_print_vec_buf(uint8_t **s, int k, int m, char *net_io_cxt)
{
	int	i, j;
	for (j = 0; j < m; j++) {
		for (i = 0; i < k; i++){
			printf(" %2x", s[i][j]);
			sprintf(net_io_cxt+strlen(net_io_cxt), " %2x", s[i][j]);
		}
		printf("\n");
		sprintf(net_io_cxt+strlen(net_io_cxt), "\n");
	}
	printf("\n");
	sprintf(net_io_cxt+strlen(net_io_cxt), "\n");
}

/*
 * Print out a character matrix
 * Used for printing coefficients matrix
 */
void
ssm_print_u8xu8(uint8_t *s, int k, int m)
{
	int	i, j;

	for (i = 0; i < k; i++) {
		for (j = 0; j < m; j++){
			printf(" %2x", 0xff & s[j+(i*m)]);
		}
		printf("\n");
	}

	printf("\n");
}

