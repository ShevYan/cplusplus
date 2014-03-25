//#include "stdafx.h"
#include "OSSemaphore.h"
#include "OSUtility.h"
COSSemaphore::COSSemaphore(int nValue )
{
#if defined(_WIN32) || defined(_WIN64)
	m_pSem = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
#else
	m_pSem = new c_semaphore(nValue);
#endif
};

COSSemaphore::~COSSemaphore()
{
#if defined(_WIN32) || defined(_WIN64)
	CloseHandle(m_pSem);
#else
	_SafeDeletePtr(m_pSem);
#endif
    
};

int COSSemaphore::Wait()
{
#if defined(_WIN32) || defined(_WIN64)
	return WaitForSingleObject(m_pSem, INFINITE);
#else
	return m_pSem->wait();
#endif
};

int COSSemaphore::TimedWait(DWORD dwTimeout)
{
#if defined(_WIN32) || defined(_WIN64)
	return WaitForSingleObject(m_pSem, dwTimeout);
#else 
    return m_pSem->wait(dwTimeout);
#endif
}

bool COSSemaphore::Post()
{
#if defined(_WIN32) || defined(_WIN64)
	return !!ReleaseSemaphore(m_pSem, 1, NULL);
#else
    return m_pSem->set()==0 ? true : false;
#endif
}