#include "sv_ipc.h"
int shm_get(char *path,int size)
{
		int fd;
		fd = open(path,O_CREAT|O_EXCL,0666);
		if(fd < 0 && errno != EEXIST)
				oops("sem_get:open error",1);

		lseek(fd,sizeof(struct memmsg)+100,SEEK_SET);
		key_t key = ftok(path,0);
		if(key < 0)
				oops("shm_get:ftok error",1);
		int shmflag = IPC_CREAT | IPC_EXCL | 0666;
		int shmid = shmget(key,size,shmflag);
		if(shmid <0)
		{
				if(errno == EEXIST)
				{
					shmid = shmget(key,size,IPC_CREAT | 0666);
					if(shmid < 0)
							oops("shmget error:IPC_CREAT",1);
					return shmid;
				}
				else
						oops("shmget error:IPC_CREAT | IPC_EXCL",1);
		}

		return shmid;
}
void shm_init(int shmid,int semid)
{
	union semun arg1;
	union semun arg2;
	union semun arg3;
	arg1.val = 1;//MUTEX 
	arg2.val = MAXMSG;//EMPTY
	arg3.val = 0;//FULL

	if(semctl(semid,MX_MUTEX,SETVAL,arg1)<0)
			oops("shm_init:semctl",1);
	if(semctl(semid,MX_EMPTY,SETVAL,arg2)<0)
			oops("shm_init:semctl",1);
	if(semctl(semid,MX_FULL,SETVAL,arg3)<0)
	 		oops("shm_init:semctl",1);

	memmsg *ptr = (memmsg *)shmat(shmid,NULL,0);//default read and write permission
	
	ptr->semid = semid;
	ptr->input = ptr->output = 0;
	//init the offset of each msg
	int i = 0;
	for(i=0;i<MAXMSG;i++)
	{
			ptr->msgoff[i] = i*MSGSIZE;
			ptr->msglen[i] = 0;
			memset(&ptr->msgdata[i*MSGSIZE],0,MSGSIZE);
	}
	return;
}
void shm_remove(int shmid)
{
    //printf("\n[semop] delete [%d]\n",shmid);
		if(shmctl(shmid,0,IPC_RMID)<0)
				oops("shmctl error:remove",1);
		return;
}
void shm_send(int shmid,char *buf,int len)
{
		memmsg *ptr = (memmsg *)shmat(shmid,NULL,0);//default read and write permission

		/*
		int val1,val2,val3;
		val1 = semctl(ptr->semid,MX_FULL,GETVAL);
		val2 = semctl(ptr->semid,MX_MUTEX,GETVAL);
		val3 = semctl(ptr->semid,MX_EMPTY,GETVAL);
		printf("val1 = %d val2 = %d val3 = %d\n",val1,val2,val3);
		*/

		sem_wait(ptr->semid,MX_EMPTY);
		sem_wait(ptr->semid,MX_MUTEX);
		
		int next_input = ptr->input; 
		long offset = ptr->msgoff[next_input];
		memset(&ptr->msgdata[offset],0,MSGSIZE);
		strcpy(&ptr->msgdata[offset],buf);
		ptr->msglen[ptr->input] = len;
		ptr->input = (ptr->input+1) % MAXMSG;

		sem_notify(ptr->semid,MX_MUTEX);
		sem_notify(ptr->semid,MX_FULL);

		/*
		val1 = semctl(ptr->semid,MX_FULL,GETVAL);
		val2 = semctl(ptr->semid,MX_MUTEX,GETVAL);
		val3 = semctl(ptr->semid,MX_EMPTY,GETVAL);
		printf("val1 = %d val2 = %d val3 = %d\n",val1,val2,val3);
		*/

		return;
}
void shm_recv(int shmid,char *buf,int *len)
{
		memmsg *ptr = (memmsg *)shmat(shmid,NULL,0);//default read and write permission

		/*
		int val1 = semctl(ptr->semid,MX_FULL,GETVAL);
		int val2 = semctl(ptr->semid,MX_MUTEX,GETVAL);
		int val3 = semctl(ptr->semid,MX_EMPTY,GETVAL);
		printf("val1 = %d val2 = %d val3 = %d\n",val1,val2,val3);
		*/

		sem_wait(ptr->semid,MX_FULL);
		sem_wait(ptr->semid,MX_MUTEX);

		int next_output = ptr->output;
		long offset = ptr->msgoff[next_output];
		strcpy(buf,&ptr->msgdata[offset]);
		*len = ptr->msglen[ptr->output];
		ptr->output = (ptr->output + 1)%MAXMSG;

		sem_notify(ptr->semid,MX_MUTEX);
		sem_notify(ptr->semid,MX_EMPTY);

		return;
}
