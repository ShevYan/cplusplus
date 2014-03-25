#ifndef __OSMUTEX_H__
#define __OSMUTEX_H__
#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include "BaseType.h"
#else
#include "c_wait_for.h"
#endif

class CCritSec
{
public:
    CCritSec();
    ~CCritSec();

public:
    void Lock();
    void Unlock();

    BOOL Islock()
    {
#if defined(_WIN32) || defined(_WIN64)
		DWORD us = GetCurrentThreadId();
		return (m_currentOwner && m_currentOwner != us);
#else
        return m_CritSec.islock();
#endif
    }

public:

#if defined(_WIN32) || defined(_WIN64)
	DWORD m_currentOwner;
#else
    pthread_t m_currentOwner;
#endif

    bool m_bDelete;

    void SetDeleteFlag()
    {
        m_bDelete = TRUE;
    }
    bool IsNeedDelete()
    {
        return(m_bDelete );
    }
private:

#if defined(_WIN32) || defined(_WIN64)
	CRITICAL_SECTION m_CritSec;
#else
	c_lock m_CritSec;
#endif

};

typedef CCritSec    FiCritSec;

#endif 
