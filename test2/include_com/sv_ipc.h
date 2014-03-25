#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define NSEMS 10
#define MSGSIZE 4096 
#define MAXMSG 10

#define MX_MUTEX 0
#define MX_EMPTY 1
#define MX_FULL 2

#define oops(s,m) {perror(s);exit(m);}
//union semun
//{
//		int val;
//		struct semid_ds *buf;
//		unsigned short *array;
//};
#ifdef FI_LINUX
typedef union semun {
	int val;			/* value for SETVAL */
	struct semid_ds *buf;	/* buffer for IPC_STAT & IPC_SET */
	unsigned short *array;	/* array for GETALL & SETALL */
	struct seminfo *__buf;	/* buffer for IPC_INFO */
	void *__pad;
}ts;
#endif
struct memmsg
{
	int semid;
	int input;//next input pos
	int output;//next output pos 
	int msglen[MAXMSG];
	long msgoff[MAXMSG];
	char msgdata[MAXMSG*MSGSIZE];
};

extern int shm_get(char*,int );
extern void shm_init(int,int);
extern void shm_send(int,char*,int);
extern void shm_recv(int,char*,int*);
extern void shm_remove(int);

extern int sem_get(char*);
extern void sem_remove(int); 
extern void sem_wait(int,int);
extern void sem_notify(int,int);
