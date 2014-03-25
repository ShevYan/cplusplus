#ifndef _UDP_SOCKET_THREAD_H_
#define _UDP_SOCKET_THREAD_H_

#include "udpsocket.h"
#include "FiMemory.h"

class CMemItem;
typedef void (__stdcall *RECVUDPMSGHANDLE_FUNC)(CMemItem *pSendMsgBuf, DWORD dwUserData);

class CUDPSocketThread
{
public:
	CUDPSocketThread(RECVUDPMSGHANDLE_FUNC lpfnOnReceive, DWORD dwUserData = 0);
	~CUDPSocketThread();

	BOOL StartService(DWORD &dwPort );
	BOOL StopService();
	
	DWORD SendUDPMsg(LPCSOCKADDR lpSockAddr, CMemItem *pSendMsgBuf);
	
	pthread_t GetWorkThreadHandle() { return m_hWorkThread; }
	pthread_t GetListenThreadHandle() { return m_hListenThread; }
	
	DWORD GetPort(){ return m_dwListenPort;}

	CMemItem *m_pUdpBuf;
	CCritSec m_BufLock;
	
private:

	LPUDPPER_IO_OPERATION_DATA AssociateSocketWithCompletionPort();
	
	static void* __stdcall _ServerListenThread(LPVOID lpParam);
	static void* __stdcall _ServerWorkThread(LPVOID lpParam);
	
	DWORD ProcessMsg(LPUDPPER_IO_OPERATION_DATA pContext,DWORD dwNumBytes);
	void CloseClient(LPUDPPER_HANDLE_DATA lpPerHandleData);
	
	BOOL StartWorkThread();
	BOOL StartListenThread(DWORD &dwPort);
	
	LPUDPPER_IO_OPERATION_DATA CreateInfo(CUDPSocket &tcpSocket);
	
protected:
	virtual void OnAccept(HANDLE hSocket);
	virtual void OnReceive(HANDLE hSocket, CMemItem *pMsgBuf);

	virtual void OnSendMsg();
	virtual void OnClose(HANDLE hSocket);
	
private:

    CUDPSocket	m_cListenSocket;	// 
	

	CUDPSocket m_cUDPSocket;

	LPUDPPER_IO_OPERATION_DATA m_lpUDPPerIOData;

	
	HANDLE	m_hCompletionPort;
	
	pthread_t m_hListenThread;
	pthread_t m_hWorkThread;
	
	DWORD m_dwListenPort;
	DWORD m_dwNumCurrentThreads;

	
	DWORD m_dwUserData;
	RECVUDPMSGHANDLE_FUNC m_lpfnOnReceive;
};

#endif // _UDP_SOCKET_THREAD_H_

