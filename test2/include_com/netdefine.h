// netdefine.h: interface for the netdefine.h class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __NETWORKDEFINE_H__
#define __NETWORKDEFINE_H__

#include "ShareLock.h"
#include "FiMemory.h"

#include <list>
#include <vector>
#include <map>
#include <queue>
using namespace std;

#if !(defined(_WIN32) || defined(_WIN64))
typedef struct _WSABUF
{
	ULONG len;
	char *buf;
}WSABUF, * LPWSABUF;

#define ZeroMemory(x,y) memset(x,0,y)
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#endif

#define GetCallParaKey(sessionId, pktId) \
	(u64)(((u64)(sessionId) << 32) | ( (u64)(pktId) ))


#define RECV_MSG	0x01
#define SEND_MSG	0x02
#define MAX_CLIENT      3000
#define NET_TIMEOUT     (5000) 
#define OVERLAPPED  int


#define CPU_NUMBER 8
const DWORD SOCKET_RECV_BUFFER_SIZE	= 10*1024*1024;
const DWORD SOCKET_SEND_BUFFER_SIZE = 10*1024*1024;

template <typename T>
struct stPER_HANDLE_DATA {
	T Socket;
};

template <typename T>
struct stPER_IO_OPERATION_DATA
{
	stPER_IO_OPERATION_DATA();
	~stPER_IO_OPERATION_DATA();
	
	BOOL Initialize(const T &Socket);
	void Uninitialize();
	
	OVERLAPPED Overlapped;
	DWORD dwSendBytes;
	DWORD dwRecvBytes;
	WSABUF DataBuf ;
	
	DWORD dwOpType ;
	CMemItem *pMsgBuf;
	
	stPER_HANDLE_DATA<T> IoData ;
};

template <typename T>
BOOL stPER_IO_OPERATION_DATA<T> ::Initialize(const T &Socket)
{
	pMsgBuf = new CMemItem ;
	pMsgBuf->SetBufferSize(1024*512);
	DataBuf.len = pMsgBuf->m_dwBufSize;
	DataBuf.buf = (char *)(pMsgBuf->m_ptrData);
	IoData.Socket = Socket;
	dwRecvBytes = 0;
	dwSendBytes = 0;
	dwOpType = 0 ;// recv
	ZeroMemory(&Overlapped, sizeof(Overlapped));
	
	return TRUE;
}

template <typename T>
void stPER_IO_OPERATION_DATA<T> ::Uninitialize()
{
	if (NULL != pMsgBuf)
	{
		delete pMsgBuf;
		pMsgBuf = NULL;
	}
	
	DataBuf.len = 0;
	DataBuf.buf = NULL;
	
	try
	{
		IoData.Socket.Close();
	}
	catch (...)
	{
	}
	
}

template <typename T>
stPER_IO_OPERATION_DATA<T> ::stPER_IO_OPERATION_DATA()
{
	DataBuf.len = 0;
	DataBuf.buf = NULL;
	//IoData.Socket = NULL;
	dwRecvBytes = 0;
	dwSendBytes = 0;
	dwOpType = 0 ;
	
	pMsgBuf = NULL;
	
	ZeroMemory(&DataBuf, sizeof(WSABUF));
}

template <typename T>
stPER_IO_OPERATION_DATA<T> ::~stPER_IO_OPERATION_DATA()
{
    ;
}


#endif
