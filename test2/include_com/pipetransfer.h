#ifndef __PIPETRANSFER_H__
#define __PIPETRANSFER_H__

#include "BaseType.h"
#include <string>
#include <vector>
#include <queue>
using namespace std;

#define MAX_PIPEMSG_SIZE 1024
#define LIMIT_NAME_LEN   16

#define PIPE_CLT 0
#define PIPE_SVR 1

enum MSG_TYPE
{
	QUERY_ALL = 0,
	QUERY_SCM,
	QUERY_MAS,
	QUERY_DISK,
	QUERY_SCMD,
	QUERY_LOGOPT,
	QUERY_VOLBANDWIDTH,
	SET_LOGOPT,
	SET_PROCEXIT,
	SET_VOLBANDWIDTH,
	SET_OTHER
};

enum MSG_WAY
{
	PIPE_MSG_SEND = 0,
	PIPE_MSG_RECV
};

typedef struct _QueryDataCxt
{
	char cModuleName[LIMIT_NAME_LEN]; // module limit 16,please check
	int  nModuleStatus;   // 0 ,1
	int  nProcId;
	unsigned long lip;
	int  nConnStatus;     // conn svr ip status
	unsigned long long VolId;
	int nVolStatus;	 			   
}QueryDataCxt,*PQueryDataCxt;

typedef struct _LogLevelCxt
{
	int nLevel;
	int nOptArr[100];
}LogLevelCxt,*PLogLevelCxt;

typedef struct _VolBandWidth
{
	u32 volID;
	u64 allBW;
	u64 readBW;
	u64 writeBW;
}VolBandWidth,*PVolBandWidth;

typedef struct _PipeMsg
{
	MSG_TYPE type;
	MSG_WAY  way;
	int      ntolsize; // header size + body size

	union
	{
		QueryDataCxt query;
		LogLevelCxt  loglevel;
		VolBandWidth vbw;
	}data;

}PipeMsg,*PPipeMsg;

typedef struct _DPipeCxt
{
	string sPipeSvr;
	string sPipeClt;
	int    fdSvr;
	int    fdClt;
	pthread_t thd;
	void*  evt;           // event
	void*  obj;           // 
}DPipeCxt,*PDPipeCxt;

typedef void ( *PIPERECVCALL)(PPipeMsg pMsg);

typedef struct _PIPEFUNCTAB
{
	MSG_TYPE type;
	PIPERECVCALL lpFunc;
}PIPEFUNCTAB;

class CPipeTransfer
{
public:
	CPipeTransfer(PIPERECVCALL callback = NULL);
	~CPipeTransfer();
public:
	
	bool AddPipe(string sVolId,string sMountPoint);

	// client call all info
	// client call one volume ref info
	virtual bool DoJob(string sVolId,string sMountPoint,MSG_TYPE type,PPipeMsg pMsg);
	
	// svr call 
	virtual bool ReplyInfo(PPipeMsg pMsg);
	
	// only test call
	void testWait();
	
private:
	bool Init(PIPERECVCALL callback = NULL);
	void SetMode(int nMode);
	PDPipeCxt GetPipeCxt(string sPipeName );
	bool CheckOpenPipe(int &fd,string sName,int nOFlag,int nAFlag);
	static void* _CompleteRead(void* lparam);

private:
	int m_nMode; // 0->clt ,1->svr.

	pthread_mutex_t   m_lock; 
	vector<PDPipeCxt> m_vecDPipe;
	PIPERECVCALL m_RecvCall;

	pthread_mutex_t  m_lockQueue;
	queue<PPipeMsg> m_MsgQueue;
};

#endif
