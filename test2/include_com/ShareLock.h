// ShareLock.h: interface for the CShareLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SHARELOCK_H_)
#define _SHARELOCK_H_

#pragma once

#include "OSThread.h"

#include <queue>
using namespace std;

#define MAXFREEEVENTNUM   36
#define OWNEREVENT        1
#define SHAREEVENT        0

class CShareLock
{
public:
	CShareLock();
	virtual ~CShareLock();

	BOOL OwnerLock();
	BOOL ShareLock(BOOL bForce = FALSE);
	BOOL LeaveLock();

	BOOL TryShareLock();
	BOOL TryLock();
    BOOL WhetherLock()
    {
        BOOL br = FALSE;
        m_ShareLock.Lock();
        br = ((m_ShareNum==0 )&& (m_OwnerNum==0)) ? FALSE:TRUE;
        m_ShareLock.Unlock();

        return br;
    }
	BOOL TestLock();
protected:
	queue<FiEvent*>  m_FreeEventList;
	queue<FiEvent*>  m_UsedEventList;
	queue<int>     m_EventEXList;     

	int            m_ShareNum;		 
	int            m_OwnerNum;       
    int            m_DbgFlag;
    OSTHREAD_ID      m_OwnerTid;
	CCritSec       m_ShareLock;     
};

#endif
