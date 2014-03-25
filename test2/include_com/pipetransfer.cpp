
#include "pipetransfer.h"
#include "OSThread.h"
#include "FiLog.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <grp.h>


CPipeTransfer::CPipeTransfer(PIPERECVCALL callback)
{
	m_nMode = PIPE_CLT;
	m_RecvCall = NULL;
	signal(SIGPIPE, SIG_IGN);

	Init(callback);
}

CPipeTransfer::~CPipeTransfer()
{
	vector<PDPipeCxt>::iterator it;

	pthread_mutex_lock(&m_lock);

	for(it = m_vecDPipe.begin(); it != m_vecDPipe.end(); it++)
	{
		if((*it)->thd != (pthread_t)-1)
		{
			pthread_cancel((*it)->thd);
		}
	}

	pthread_mutex_unlock(&m_lock);
}

bool CPipeTransfer::Init(PIPERECVCALL callback)
{
	int nMode = PIPE_CLT;

	if(NULL != callback)
	{
		nMode = PIPE_SVR;
		m_RecvCall = callback;
	}
	
	SetMode(nMode);

	pthread_mutex_init(&m_lock,NULL);
	pthread_mutex_init(&m_lockQueue,NULL);
}

bool CPipeTransfer::AddPipe(string sVolId,string sMountPoint)
{
	// make svr pipe
	// create a select thread wait pipe 
	bool bret = false;
	string pipeSvr;
	string pipeClt;
	vector<PDPipeCxt>::iterator it;
	PDPipeCxt pCxt = NULL;

	replace(sMountPoint.begin(),sMountPoint.end(),SLASHLINCH,('_'));

	pipeSvr ="/tmp/" + sVolId + "_"  + sMountPoint + "_SVR";
	pipeClt ="/tmp/" + sVolId + "_"  + sMountPoint + "_CLT";
	
	pthread_mutex_lock(&m_lock);

	for(it = m_vecDPipe.begin(); it != m_vecDPipe.end(); it++)
	{
		
		if((*it)->sPipeSvr == pipeSvr)
		{
			//printf("AddPipe exist [%s]",pipeSvr.c_str());
			break;
		}

		if((*it)->sPipeClt == pipeClt)
		{
			break;
		}
	}

	if(it == m_vecDPipe.end())
	{
		pCxt = new DPipeCxt;

		pCxt->sPipeSvr = pipeSvr;
		pCxt->sPipeClt = pipeClt;
		pCxt->evt      = NULL;
		pCxt->obj 	   = this;
		pCxt->fdClt    = -1;
		pCxt->fdSvr    = -1;
        int ret = -1;
		if ((ret= mkfifo(pipeSvr.c_str(), S_IFIFO|0666)) != 0 && errno != EEXIST)
		{
            //printf("mkfifo ret %d error[%d] strerr[%s]",ret,errno,strerror(errno));
            assert(0);
			bret = false;

			goto LABLE;
		}

		if (mkfifo(pipeClt.c_str(), S_IFIFO|0666) != 0 && errno != EEXIST)
		{
            assert(0);
			bret = false;
			goto LABLE;
		}

		m_vecDPipe.push_back(pCxt);

		pthread_create(&pCxt->thd,NULL, _CompleteRead,(void *)pCxt);
	}
	
	bret = true;

LABLE:
	pthread_mutex_unlock(&m_lock);

	return 	bret;
}


bool CPipeTransfer::DoJob(string sVolId, string sMountPoint,
							  MSG_TYPE type, PPipeMsg pMsg)
{
	string pipeSvr;
	string pipeClt;

	PDPipeCxt pCxt = NULL;
	vector<PDPipeCxt>::iterator it;
	FiEvent evt;
	PPipeMsg pRecvMsg = NULL;

	assert(pMsg);

	AddPipe(sVolId,sMountPoint);
	std::replace(sMountPoint.begin(),sMountPoint.end(),SLASHLINCH,('_'));


	pipeSvr = "/tmp/" + sVolId + "_"  + sMountPoint + "_SVR";
	pipeClt = "/tmp/" + sVolId + "_"  + sMountPoint + "_CLT";
	
	// query svr
	pCxt = GetPipeCxt(pipeSvr);
	
	if(pCxt == NULL)
	{

		return false;
	}

	assert(pCxt);

	pCxt->evt = &evt;

	if(!CheckOpenPipe(pCxt->fdSvr,pCxt->sPipeSvr,O_WRONLY | O_NONBLOCK,0666))
	{
		pMsg->way = PIPE_MSG_RECV;	
		//printf("[PIPE_CLT] open failed.\n");
		//fixme set error msg	
		return false;
	}
	
	pMsg->way  = PIPE_MSG_SEND;

	//printf("[pipeclt] begin write msg to pipe svr\n");
	int nByte = write(pCxt->fdSvr,pMsg,sizeof(PipeMsg));
	//printf("[pipeclt] end write msg to pope svr byte[%d]\n",nByte);
	if(nByte == -1)
	{
		return false;
	}

	if(pMsg->type == QUERY_ALL ||
	   pMsg->type == QUERY_SCM ||
	   pMsg->type == QUERY_MAS || 
	   pMsg->type == QUERY_DISK ||
	   pMsg->type == QUERY_SCMD ||
	   pMsg->type == QUERY_VOLBANDWIDTH ||
	   pMsg->type == QUERY_LOGOPT)
	{
		if (-1 != evt.Wait(10000))
		{
			// pop msg
			//printf("[pipeclt] wait success msg return \n");
			pthread_mutex_lock(&m_lockQueue);
	
			assert( 0 != m_MsgQueue.size());
			pRecvMsg = m_MsgQueue.front();
	
			m_MsgQueue.pop();
	
			//printf("[pipeclt] queue size[%d]\n",m_MsgQueue.size());
			pthread_mutex_unlock(&m_lockQueue);
			
			
			memcpy(pMsg,pRecvMsg,sizeof(PipeMsg));

			delete[] pRecvMsg;
			pRecvMsg = NULL;
		}
		else
		{
			//printf("[pipeclt] wait vol[%s] time out \n",sVolId.c_str());
			return false;
		}
	}

	//close(pCxt->fdSvr);
	
	return true;
}

bool CPipeTransfer::ReplyInfo(PPipeMsg pMsg)
{
	assert(pMsg);

	PDPipeCxt pCxt = NULL;
	int nWrite = 0;
	bool bret = false;

	pCxt = *(m_vecDPipe.begin());

	assert(pCxt);

	// open pipe svrnode,maybe clt thread start slowly 

	for(int i = 0; i < 10; i++)
	{
		if(!(bret = CheckOpenPipe(pCxt->fdClt,pCxt->sPipeClt,O_WRONLY | O_NONBLOCK,0666)))
		{
			//printf("[pipesvr] pipename[%s] can not open error[%d]\n",pCxt->sPipeClt.c_str(),errno);
			usleep(50);
		}
		else
		{
			break;
		}
	}
	
	if(!bret)
	{
		return false;
	}
	
	pMsg->way = PIPE_MSG_RECV;

	//printf("[pipesvr] begin reply msg to clt \n");
	nWrite = write(pCxt->fdClt,pMsg,sizeof(PipeMsg));
	//printf("[pipesvr] end reply msg to clt byte[%d] \n", nWrite);
	
	if(nWrite == -1)
	{
		//printf("[pipesvr] mgr pipe exit \n");
	}

	delete  pMsg;
	pMsg = NULL;

	return true;
}

void CPipeTransfer::SetMode(int nMode)
{
	m_nMode = nMode;
}

void* CPipeTransfer::_CompleteRead(void* lparam)
{
	assert(lparam);

	string sname;
	PPipeMsg pMsg = NULL;
	fd_set readfds;
	int n_index = 0;
	int n_hold_pipe_size = 1;
	int n_ret = 0;
	int n_max_sock = 0;
	int pipe_return = 0;
	int fd_pipe = 0;
	int begin = 0;
	PDPipeCxt pCxt = (PDPipeCxt)lparam;
	CPipeTransfer *pThis = (CPipeTransfer*)(pCxt->obj);

	if(pThis->m_nMode == PIPE_SVR)
	{
		fd_pipe = pCxt->fdSvr;
		sname = pCxt->sPipeSvr;
	}
	else
	{
		fd_pipe = pCxt->fdClt;
		sname = pCxt->sPipeClt;
	}

LABLE:

	pThis->CheckOpenPipe(fd_pipe,sname,O_RDWR | O_NONBLOCK,0666);
	
	if(fd_pipe == -1)
	{
		int rr = errno;

		//printf("pipe _CompleteRead start failed \n");
		return 0;
		//assert(0);
	}

	while (TRUE) 
	{

		FD_ZERO(&readfds);
		FD_SET(fd_pipe, &readfds);

		//printf(("[Mgr ] pre select \n"));
		n_ret = select(fd_pipe+1, &readfds, NULL, NULL, 0);
		//printf(("[Mgr ]  select n_ret[%d] errno[%d]\n"),n_ret,errno);

		//if(errno == EAGAIN)
		//{
		//	goto LABLE;
		//}

		if (n_ret < 0)
		{
			if (EINTR == errno)
			{
				STRACE(("Select is Interrupted by system call\n"));
				continue;
			}
			else
			{
				printf("select error<%d>.\n", errno);
				int nErr = errno;
				goto LABLE;
			}
		}

		if (n_ret == 0)
		{
			//printf(("n_ret == 0 n_ret == 0n_ret == 0n_ret == 0 \n"));
			continue;
		}

		//printf(("[Mgr] ___ select has msg_____ begin \n"));

		if (FD_ISSET(fd_pipe, &readfds))
		{
			pMsg = (PipeMsg*)new char[sizeof(PipeMsg)];
			memset(pMsg,0x00,sizeof(PipeMsg));

			n_ret = read(fd_pipe,pMsg,sizeof(PipeMsg));

			//printf(("[Mgr]____read end____ \n"));
//  		if(errno == ENXIO )
//  		{
//  			goto LABLE;
//  		}
			if (n_ret == 0)
			{
				//close(fd_pipe);
				//FD_CLR(fd_pipe,&readfds);

				//printf(("[Mgr] ___ select pipe exit goto reopen pipe \n"));
				goto LABLE;;
			}

			if(pThis->m_RecvCall != NULL)
			{   // svr
				//printf("callback \n");
				pThis->m_RecvCall((PPipeMsg)pMsg);
			}
			else
			{	// clt
				pthread_mutex_lock(&pThis->m_lockQueue);
				pThis->m_MsgQueue.push((PPipeMsg)pMsg);
				pthread_mutex_unlock(&pThis->m_lockQueue);

				assert(pCxt->evt);
				((FiEvent*)pCxt->evt)->Set();

				//printf("set clt query event \n");
			}
		}

		//printf(("[Mgr] ___ select has msg_____ end \n"));
	} 

    close(fd_pipe);

    if (0 != remove(sname.c_str()))
    {
        //printf("[Mgr] Delete File<%s> Fail, ERRNO[%d].\n", sname.c_str(), errno);
    }
    else
    {
        //printf("[Mgr]  Delete File<%s> OK.\n", sname.c_str());
    }
}

PDPipeCxt CPipeTransfer::GetPipeCxt(string sPipeName )
{
	assert(!sPipeName.empty());

	vector<PDPipeCxt>::iterator it;
	PDPipeCxt pCxt = NULL;

	pthread_mutex_lock(&m_lock);

	for(it = m_vecDPipe.begin(); it != m_vecDPipe.end(); it++)
	{
		pCxt = *it;

		if(pCxt->sPipeSvr == sPipeName)
		{
			break;
		}
	}

	pthread_mutex_unlock(&m_lock);

	return pCxt;
}

bool CPipeTransfer::CheckOpenPipe(int &fd,string sName,int nOFlag,int nAFlag)
{
	assert(!sName.empty());
	
	bool bret = false;

	if(fd != -1)
	{
		close(fd);
	}

	for(int i = 0; i < 3; i++)
	{
		//fd = open(sName.c_str(), nOFlag, nAFlag);
		fd = open(sName.c_str(), nOFlag,0);

		//printf("open pipe %s fd[%d] errno[%d]\n",sName.c_str(),fd,errno);
		if (fd != -1)
		{
			bret = true;
			break;
		}

		if(fd == -1 && errno == EINTR)
		{
			//printf("open pipe EINTR____");
			continue;
		}

		if (fd ==-1 || errno == ENXIO)
		{
			//printf("pipe not open Oflag[%d] Aflag[%d] on pipe[%s] err[%d]",nOFlag,nAFlag,sName.c_str(),errno);
			if (errno == EACCES)
			{
				//printf("open pipe [%s] Permission denied \n",sName.c_str());
			}

			errno  = 0;
			bret = false;
			break;
		}

		//printf("open pipe %s\n",sName.c_str());
	}   
	
	return bret;
}

void CPipeTransfer::testWait()
{
	FiEvent evt;
	evt.Wait(INFINITE);
}
