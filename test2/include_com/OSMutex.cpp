
#include "OSMutex.h"


CCritSec::CCritSec()
{
    m_bDelete = 0;

#if defined(_WIN32) || defined(_WIN64)
	InitializeCriticalSection(&m_CritSec);
    m_currentOwner = 0;
#else
	m_currentOwner = (pthread_t)0;
#endif

}

CCritSec::~CCritSec()
{
#if defined(_WIN32) || defined(_WIN64)
	DeleteCriticalSection(&m_CritSec);
#endif
}

void CCritSec::Lock()
{
#if defined(_WIN32) || defined(_WIN64)
	DWORD us = GetCurrentThreadId();
	EnterCriticalSection(&m_CritSec);
	m_currentOwner = us;

#else
    m_CritSec.lock();
    m_currentOwner = m_CritSec.get_current_owner();
#endif
}

void CCritSec::Unlock()
{
#if defined(_WIN32) || defined(_WIN64)
	 LeaveCriticalSection(&m_CritSec);
#else
    m_CritSec.unlock();
    m_currentOwner = m_CritSec.get_current_owner();
#endif
}
