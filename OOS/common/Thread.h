/*
 * Thread.h
 *
 *  Created on: Apr 14, 2014
 *      Author: shev
 */

#ifndef THREAD_H_
#define THREAD_H_
#include <pthread.h>

typedef void* (* _THREAD_FUN)(void *cxt);

class Thread {
private:
	pthread_t 	m_Tid;
	_THREAD_FUN m_lpThreadFun;
	void*		m_lpThreadCxt;

public:
	Thread(_THREAD_FUN lpThreadFun, void *cxt);
	virtual ~Thread();

	void Start();
	void Stop();
	void Join();
	void Detach();
	pthread_t GetThreadID();
	bool IsStarted();
	bool IsSelf();

public:
	static void ThreadTest();
};

#endif /* THREAD_H_ */
