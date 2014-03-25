#include "sv_ipc.h"

//int sem_get(char * path,int nsems,int semflag)
int sem_get(char * path)
{

		int fd;
		fd = open(path,O_CREAT|O_EXCL,0666);
		if(fd < 0 && errno != EEXIST)
				oops("sem_get:open error",1);
		int semid;
		union semun arg;	
		int nsems = NSEMS;

		key_t key = ftok(path,0);
		if(key < 0)
				oops("sem_get:ftok error",1);
		
		int semflag = IPC_CREAT | IPC_EXCL | 0666;	
		if((semid = semget(key,nsems,semflag))<0)
		{
				if(errno == EEXIST)
				{
						semid = semget(key,NSEMS,IPC_CREAT|0666);
						if(semid < 0)
								oops("semget error:IPC_CREAT",1);
						return semid;
				}
				else
					oops("semget error:IPC_CREAT|IPC_EXCL",1);
		}
	
		/*
		struct semid_ds seminfo;

		arg.buf = &seminfo;
		semctl(semid,0,IPC_STAT,arg);			
		nsems = arg.buf->sem_nsems;
		*/
		unsigned short * ptr = (unsigned short *)calloc(nsems,sizeof(unsigned short));
		arg.array = ptr;
		int i = 0;
		for(i=0;i<nsems;i++)
				arg.array[i] = 0;
		if(semctl(semid,0,SETALL,arg)<0)
				oops("semctl error:setval",1);
		return semid;	
}
void sem_remove(int semid)
{
   // printf("\n[semop] delete [%d]\n",semid);
		if(semctl(semid,0,IPC_RMID)<0)
				oops("sem_ctl error:remove",1);
		return;
}
void sem_wait(int semid,int semx)
{
		/*
		int val1 = semctl(semid,MX_FULL,GETVAL);
		int val2 = semctl(semid,MX_MUTEX,GETVAL);
		int val3 = semctl(semid,MX_EMPTY,GETVAL);
		printf("wait:val1 = %d val2 = %d val3 = %d\n",val1,val2,val3);
		*/

		struct sembuf sbuf;
		sbuf.sem_num = semx;
		sbuf.sem_op = -1;
		sbuf.sem_flg = 0;

		int nsems;
		struct semid_ds seminfo;
/*
		union semun arg;	
		arg.buf = &seminfo;
		semctl(semid,0,IPC_STAT,arg);			
		nsems = arg.buf->sem_nsems;
*/		
		if(semop(semid,&sbuf,1)<0)
				oops("semop error:wait",1);
		return;
}
void sem_notify(int semid,int semx)
{
		struct sembuf sbuf;
		sbuf.sem_num = semx;
		sbuf.sem_op = 1;
		//sbuf.sem_flg = SEM_UNDO;
		sbuf.sem_flg = 0; 

		int nsems;
		struct semid_ds seminfo;

		/*
		union semun arg;	
		arg.buf = &seminfo;
		semctl(semid,0,IPC_STAT,arg);			
		nsems = arg.buf->sem_nsems;
		*/

		if(semop(semid,&sbuf,1)<0)
				oops("semop error:wait",1);
		return;
}

