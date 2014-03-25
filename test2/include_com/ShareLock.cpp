
#include "ShareLock.h"


CShareLock::CShareLock()
{
	m_ShareNum = 0;
	m_OwnerNum = 0;

    m_OwnerTid = 0;
	m_DbgFlag = 1234321;
}

CShareLock::~CShareLock()
{
	m_DbgFlag = 4321234;
	m_ShareLock.Lock();
	while (m_FreeEventList.size())
	{
		_SafeDeletePtr(m_FreeEventList.front());
		m_FreeEventList.pop();
	}
	m_ShareLock.Unlock();
}
BOOL CShareLock::OwnerLock()
{
	FiEvent* tem = NULL;
    m_ShareLock.Lock();

    if (m_ShareNum == 0 && m_OwnerNum == 0)
	{
        m_OwnerTid = GetThreadId();
		ASSERT(m_OwnerNum == 0);
		m_OwnerNum ++;
		m_ShareLock.Unlock();
	}
	else
	{
		ASSERT(m_OwnerNum <= 1);
		if (m_FreeEventList.size())
		{
			tem = m_FreeEventList.front();
			m_FreeEventList.pop();
		}
        else
		{
			tem = new FiEvent;
			ASSERT(tem);
		}

		ASSERT(tem);

		m_UsedEventList.push(tem);
		m_EventEXList.push(OWNEREVENT);                  
	
		m_ShareLock.Unlock();
	
		DWORD dwTem = tem->Wait();

        m_OwnerTid = GetThreadId();
		if (dwTem != WAIT_FAILED)
		{
			m_ShareLock.Lock();

			if (m_FreeEventList.size() > MAXFREEEVENTNUM)
			{
				_SafeDeletePtr(tem);
			}
			else if (tem != NULL)
			{
				m_FreeEventList.push(tem);
			}

			ASSERT(!(m_OwnerNum == 0));
			ASSERT(!(m_ShareNum  > 0));

			m_ShareLock.Unlock();
		}
		else
		{
			ASSERT(0);
		}
	}

	return true;
}
BOOL CShareLock::ShareLock(BOOL bForce)
{
	FiEvent *tem = NULL;

	m_ShareLock.Lock();

   if (bForce || (m_OwnerNum == 0 && m_UsedEventList.size()==0))
	{
		m_ShareNum ++;
		m_ShareLock.Unlock();
	}
	else
	{
		if (m_FreeEventList.size())
		{
			tem = m_FreeEventList.front();
			m_FreeEventList.pop();
		}
        else
		{
			tem = new FiEvent;
			ASSERT(tem);
		}

		ASSERT(tem);

		m_UsedEventList.push(tem);
		m_EventEXList.push(SHAREEVENT);                  
	
		m_ShareLock.Unlock();
			
		DWORD dwTem = tem->Wait();

		if (dwTem != WAIT_FAILED)
		{
			m_ShareLock.Lock();

            if (m_FreeEventList.size() > MAXFREEEVENTNUM)
			{
				_SafeDeletePtr(tem);
			}
			else if (tem != NULL)
			{
				m_FreeEventList.push(tem);
			}

			ASSERT(!(m_ShareNum == 0));
			ASSERT(!(m_OwnerNum  > 0));

			m_ShareLock.Unlock();
		}
		else
		{
			ASSERT(0);
		}
	}
	return true;
}
BOOL CShareLock::LeaveLock()
{
	if((1234321 != m_DbgFlag) && (4321234 != m_DbgFlag))
	{
		ASSERT(0);
	}

	if(4321234 == m_DbgFlag)
	{
		ASSERT(0);
	}

    m_ShareLock.Lock();

	if((1234321 != m_DbgFlag) && (4321234 != m_DbgFlag))
	{
		ASSERT(0);
	}

	if(4321234 == m_DbgFlag)
	{
		ASSERT(0);
	}

	if (m_ShareNum > 0)
	{
		ASSERT(m_OwnerNum == 0);
	}

	if (m_OwnerNum > 0)
	{
		ASSERT(m_OwnerNum == 1);
		ASSERT(m_ShareNum == 0);
	}

	ASSERT(!(m_ShareNum == 0 && m_OwnerNum == 0));

	if (m_ShareNum != 0)
	{
		m_ShareNum --;
			
		if (m_ShareNum == 0)
		{
			if (m_UsedEventList.size() != 0)
			{
				ASSERT(m_EventEXList.front() == OWNEREVENT); 
				m_UsedEventList.front()->Set();
				m_OwnerNum ++;
 				m_UsedEventList.pop();
 				m_EventEXList.pop();
			}
		}			  
	}
	else if(m_OwnerNum != 0)
	{
		ASSERT(m_OwnerNum == 1);
		m_OwnerNum --;

		if (m_UsedEventList.size() != 0)
		{
			if (m_EventEXList.front() == OWNEREVENT)
			{
				m_UsedEventList.front()->Set();
				ASSERT(m_OwnerNum == 0);
				m_OwnerNum ++;
				m_UsedEventList.pop();
				m_EventEXList.pop();	
			}
			else
			{
				while(m_UsedEventList.size() !=0 && m_EventEXList.front() == SHAREEVENT)
				{
					m_UsedEventList.front()->Set();
					ASSERT(m_OwnerNum == 0);
					m_ShareNum ++;
					m_UsedEventList.pop();
					m_EventEXList.pop();
				}
			}
		}
	}
	else
	{
		ASSERT(0);
	}

	if((1234321 != m_DbgFlag) && (4321234 != m_DbgFlag))
	{
		ASSERT(0);
	}

	if(4321234 == m_DbgFlag)
	{
		ASSERT(0);
	}
	m_ShareLock.Unlock();

	return true;
}
BOOL CShareLock::TestLock()
{
	try
	{
		m_ShareLock.Lock();

		if (m_OwnerNum != 0)
		{
			m_ShareLock.Unlock();
			return true;
		}
		else
		{
			m_ShareLock.Unlock();
			return false;
		}
	}
	catch (...)
	{
		m_ShareLock.Unlock();
		ASSERT(0);
		return false;
	}
}


BOOL CShareLock::TryShareLock()
{

	m_ShareLock.Lock();
	if (/*m_ShareNum == 0 && */m_OwnerNum == 0)
	{
		m_ShareNum ++;
		m_ShareLock.Unlock();

		return true;
	}
	else
	{
		m_ShareLock.Unlock();
		return false;
	}
}

BOOL CShareLock::TryLock()
{

	m_ShareLock.Lock();
	if (m_ShareNum == 0 /*&& m_UsedEventList.size()==0*/ && m_OwnerNum == 0)
	{
		ASSERT(m_OwnerNum == 0);
		m_OwnerNum ++;
		m_ShareLock.Unlock();

		return true;
	}
	else
	{
		m_ShareLock.Unlock();
		return false;
	}

}
