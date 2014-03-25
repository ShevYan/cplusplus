
#include "OSThread.h"

#if defined(_WIN32) || defined(_WIN64)
#include "process.h"
#endif

/******************************Public*Routine******************************\
* Debug CCritSec helpers
*
* We provide debug versions of the Constructor, destructor, Lock and Unlock
* routines.  The debug code tracks who owns each critical section by
* maintaining a depth count.
*
* History:
*
\**************************************************************************/

#if defined(_WIN32) || defined(_WIN64)
HANDLE beginthread(__in_opt  LPSECURITY_ATTRIBUTES lpThreadAttributes,
				__in      SIZE_T dwStackSize,
				__in      LPTHREAD_START_ROUTINE lpStartAddress,
				__in_opt  LPVOID lpParameter,
				__in      DWORD dwCreationFlags,
				__out_opt LPDWORD lpThreadId)
{
	return ::CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress,lpParameter,dwCreationFlags,lpThreadId);
}
#else
extern int errno ;

int beginthread (pthread_t *__newthread,
				 pthread_attr_t *__attr,
				 void *(*start)(void *),
				 void * __arg)
{
	int ret = 0;

	if (NULL == __attr)
	{
		pthread_attr_t attr;
		pthread_attr_init (&attr);
		pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
		ret = pthread_create (__newthread, &attr, start, __arg);
		pthread_attr_destroy(&attr);
	}
	else
	{
		pthread_attr_setdetachstate (__attr, PTHREAD_CREATE_DETACHED);
		ret = pthread_create (__newthread,__attr,start, __arg);
	}

	return ret;
}
#endif


// destructor for CMsgThread  - cleans up any messages left in the
// queue when the thread exited
CMsgThread::~CMsgThread()
{
    int nret = 0;
    bool br = true;

    //nret = pthread_join(m_hThread, &status);

    if (0 != nret)
    {
        br = false;
    }

    while (false == m_ThreadQueue.empty())
    {
        CMsg *pMsg = m_ThreadQueue.front();

        _SafeDeletePtr(pMsg);

        m_ThreadQueue.pop_front();
    }

    //_SafeDeletePtr(m_pSemaphore);
}



int CMsgThread::GetThreadPriority()
{
    return 0;
}

BOOL CMsgThread::SetThreadPriority(int nPriority)
{
    return 0;
}

#if defined(_WIN32) || defined(_WIN64)
HANDLE CMsgThread::GetThreadHandle()
{
	return m_hThread;
}
#else
pthread_t CMsgThread::GetThreadHandle()
{
	return m_hThread;
}
#endif

void CMsgThread::PutThreadMsg(UINT uMsg, u64 dwMsgFlags,LPVOID lpMsgParam,const int &nMsgSize, FiEvent *pEvent )
{
    m_Lock.Lock();
    CMsg* pMsg = new CMsg(uMsg, dwMsgFlags, lpMsgParam, nMsgSize, pEvent);
    m_ThreadQueue.push_back(pMsg);

    m_pSemaphore->Post();

	m_Lock.Unlock();
}
void CMsgThread::WaitThreadExit()
{
    int nret = 0;
    bool br = false ;

#if defined(_WIN32) || defined(_WIN64)
	if(m_hThread != NULL) {
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL ;
	}
#else
    void *status;
    nret = pthread_join(m_hThread, &status);
    if (0 != nret)
    {
        printf(("pthread_join  m_hThread failed nret = %d"),nret);
        br = false;
    }
#endif
}

BOOL CMsgThread::CreateThread( )
{
    int nret = 0 ;
    bool br = true;

    m_pSemaphore = new COSSemaphore(0) ;
    ASSERT(m_pSemaphore);

#if defined(_WIN32) || defined(_WIN64)
	m_hThread = beginthread(NULL, 0, DefaultThreadProc, (LPVOID)this, 0, &m_ThreadId);
	if(NULL == m_hThread)
	{
		br = false;
	}
#else
    nret = beginthread(&m_hThread, NULL, DefaultThreadProc, (void*)this);

    if (0 != nret)
    {
        printf(("create CMsgThread failed err code = %d %s\n"),nret,(GetErrMsg(nret)).c_str());
        br = false;
    }
#endif
    return br;
}


/// This is the threads message pump.  Here we get and dispatch messages to
/// clients thread proc until the client refuses to process a message.
/// The client returns a non-zero value to stop the message pump, this
/// value becomes the threads exit code.

PREFIX_TYPE CMsgThread::DefaultThreadProc(  LPVOID lpParam  )
{
    CMsgThread *lpThis = (CMsgThread *)lpParam;
    CMsg msg;
    LRESULT lResult;
    int nret = 0;

    // allow a derived class to handle thread startup
    lpThis->OnThreadInit();

    do
    {
        lpThis->GetThreadMsg(&msg);

        lResult = lpThis->ThreadMessageProc(msg.uMsg,msg.dwFlags,msg.lpParam, msg.dwSize, msg.pEvent);

    } while (lResult != 0L);
#if defined(_WIN32) || defined(_WIN64)
	return	lResult;
#else
    return(void *)lResult;
#endif
}


/// Block until the next message is placed on the list m_ThreadQueue.
/// copies the message to the message pointed to by *pmsg
void CMsgThread::GetThreadMsg(CMsg *msg)
{
    CMsg * pmsg = NULL;
    // keep trying until a message appears
    while (TRUE)
    {
        {
            m_Lock.Lock();

            if (false == m_ThreadQueue.empty())
            {
                pmsg = m_ThreadQueue.front();
                m_ThreadQueue.pop_front();
                break;
            }
            else
            {
                m_lWaiting++;
            }

			m_Lock.Unlock();
        }

        m_pSemaphore->TimedWait(INFINITE);
    }
    // copy fields to caller's CMsg
    *msg = *pmsg;
    _SafeDeletePtr(pmsg);
}
