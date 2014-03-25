
#include "OSEvent.h"
#include "OSUtility.h"
COSEvent::COSEvent(BOOL fManualReset)
{
#if defined(_WIN32) || defined(_WIN64)
	m_pEvent = CreateEvent(NULL, fManualReset, FALSE, NULL);
#else
    m_pEvent = new c_event(fManualReset);
#endif
}

COSEvent::~COSEvent()
{
#if defined(_WIN32) || defined(_WIN64)
	CloseHandle(m_pEvent);
#else
	_SafeDeletePtr(m_pEvent);
#endif
}

bool COSEvent::Set()
{
#if defined(_WIN32) || defined(_WIN64)
	return SetEvent(m_pEvent);
#else
	return m_pEvent->set();
#endif
    
}

//wait in second
int COSEvent::Wait(DWORD dwTimeout )
{
#if defined(_WIN32) || defined(_WIN64)
    return (WaitForSingleObject(m_pEvent, dwTimeout));
#else
	return m_pEvent->wait(dwTimeout);
#endif
}

bool COSEvent::Reset()
{
#if defined(_WIN32) || defined(_WIN64)
	ResetEvent(m_pEvent);
	return TRUE;
#else
    return m_pEvent->reset();
#endif
}

int COSEvent::Check()
{
#if defined(_WIN32) || defined(_WIN64)
	return Wait(0);
#else
	return m_pEvent->check();
#endif
}

#if !(defined(_WIN32) || defined(_WIN64))

bool COSEvent::SetMode(BOOL fManualReset)
{
    return m_pEvent->set_mode(fManualReset);
}

#endif