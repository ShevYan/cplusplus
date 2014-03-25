#ifndef __OSEVENT_H__
#define __OSEVENT_H__

#pragma once

#if !(defined(_WIN32) || defined(_WIN64))

    #include "c_wait_for.h"
    typedef c_wait_object* WAITHANDLE;
#else
	#include "BaseType.h"
#endif

class COSEvent
{
public:
    COSEvent(BOOL fManualReset = FALSE);
    ~COSEvent();

public:
    bool Set();
    int Wait(DWORD dwTimeout = INFINITE);
    bool Reset();
    int Check();

#if defined(_WIN32) || defined(_WIN64)
	operator HANDLE () const { return m_pEvent; };
	HANDLE GetHandle() const {return m_pEvent; };
#else
	bool SetMode(BOOL fManualReset);
    operator WAITHANDLE() const{ return m_pEvent; };
    c_event* GetHandle() const {  return m_pEvent; };
#endif

private:
#if defined(_WIN32) || defined(_WIN64)
	HANDLE m_pEvent;
#else
    c_event *m_pEvent;
#endif
};

typedef COSEvent    FiEvent;
  

#endif 
