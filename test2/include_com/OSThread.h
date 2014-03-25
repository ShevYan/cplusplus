#ifndef __OSTHREAD_H__
#define __OSTHREAD_H__
#pragma once


#include "OSUtility.h"
#include "OSEvent.h"
#include "OSMutex.h"
#include "OSSemaphore.h"
#include "OSWaitObject.h"

#include <list>
#include <time.h>

#ifdef _AK_FOR_AIX_
    #include <sys/mman.h>
#else
    #ifdef __linux__
        #include <semaphore.h>
    #endif //FI_LINUX
#endif



#if defined(_WIN32) || defined(_WIN64)

#define PREFIX_TYPE DWORD WINAPI 


OSTHREAD_H beginthread(__in_opt  LPSECURITY_ATTRIBUTES lpThreadAttributes,
				__in      SIZE_T dwStackSize,
				__in      LPTHREAD_START_ROUTINE lpStartAddress,
				__in_opt  LPVOID lpParameter,
				__in      DWORD dwCreationFlags,
				__out_opt LPDWORD lpThreadId);


#else
#define PREFIX_TYPE void* 
#define OSTHREAD_T pthread_t

int beginthread (pthread_t *__newthread,
                         pthread_attr_t *__attr,
                          void *(*start)(void *),
                         void * __arg);
#endif

// Message class - really just a structure.
//
class CMsg
{
public:
    unsigned int uMsg;
    u64 dwFlags;
    LPVOID lpParam;
    FiEvent *pEvent;
    DWORD dwSize;

    CMsg(UINT u, u64 dw, LPVOID lp,const int &nMsgSize, FiEvent *pEvnt)
    : uMsg(u), dwFlags(dw), lpParam(lp), dwSize(nMsgSize), pEvent(pEvnt) {}

    CMsg()
    : uMsg(0), dwFlags(0L), lpParam(NULL), dwSize(0), pEvent(NULL) {}
};

typedef std::list<CMsg *> MsgPtrList ;

class CMsgThread
{
public:
    CMsgThread() : m_lWaiting(0){}
    ~CMsgThread();

public:
    // override this if you want to block on other things as well
    // as the message loop
    void virtual GetThreadMsg(CMsg *msg);

    // override this if you want to do something on thread startup
    virtual void OnThreadInit() {};

    BOOL CreateThread();

    BOOL WaitForThreadExit(LPDWORD lpdwExitCode) ;
    int GetThreadPriority();
    BOOL SetThreadPriority(int nPriority);

#if defined(_WIN32) || defined(_WIN64)
	HANDLE GetThreadHandle();
#else
    pthread_t GetThreadHandle();
#endif

    void PutThreadMsg(UINT uMsg, u64 dwMsgFlags,LPVOID lpMsgParam, const int &nMsgSize = 0, FiEvent *pEvent = NULL);
    void WaitThreadExit();

    void ClearQueue()
    {
        MsgPtrList::iterator it;
        m_Lock.Lock();
        for (it = m_ThreadQueue.begin(); it != m_ThreadQueue.end(); it++)
        {
            CMsg *pMsg = *it;
            delete pMsg;
        }

        m_ThreadQueue.clear();
		m_Lock.Unlock();
    }
public:
    // This is the function prototype of the function that the client
    // supplies.  It is always called on the created thread, never on
    // the creator thread.
    //
    virtual LRESULT ThreadMessageProc( UINT uMsg, u64 dwFlags, LPVOID lpParam, const int &nMsgSize, FiEvent *pEvent) = 0;

protected:

	static PREFIX_TYPE DefaultThreadProc(LPVOID lpParam);


	OSTHREAD_ID			m_ThreadId;


	OSTHREAD_H			m_hThread;

protected:

    // if you want to override GetThreadMsg to block on other things
    // as well as this queue, you need access to this
    MsgPtrList                m_ThreadQueue;
    CCritSec                  m_Lock;
    COSSemaphore *                m_pSemaphore ;
    int                      m_lWaiting;
};


#define _SafeDeleteWaitHandle(hWait) CloseEventHandle(hWait)
#define EVENTHANDLE WAITHANDLE
#define THREADHANDLE pthread_t


typedef CMsgThread  FiMsgThread;

#endif 
