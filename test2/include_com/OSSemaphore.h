#ifndef __OSSEMAPHORE_H__
#define __OSSEMAPHORE_H__
#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include "BaseType.h"
#else
#include <list>
#include <time.h>

#ifdef _AK_FOR_AIX_
    #include <sys/mman.h>
#else
    #ifdef FI_LINUX
        #include <semaphore.h>
    #endif //FI_LINUX
#endif

#include "c_wait_for.h"
#endif

class COSSemaphore
{
public:
    COSSemaphore(int nValue = 0);
    ~COSSemaphore();

public:
    int Wait();
    int  TimedWait(DWORD dwTimeout = INFINITE);
    bool Post();

protected:

#if defined(_WIN32) || defined(_WIN64)
	HANDLE m_pSem;
#else
    c_semaphore* m_pSem;
#endif

};

typedef	COSSemaphore  FiSem;

#endif 
