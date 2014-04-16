/*
 * Thread.cpp
 *
 *  Created on: Apr 14, 2014
 *      Author: shev
 */

#include "Thread.h"
#include "comm.h"

Thread::Thread(_THREAD_FUN lpThreadFun, void *cxt)
	: m_Tid(0), m_lpThreadFun(lpThreadFun), m_lpThreadCxt(cxt) {

}

Thread::~Thread() {
	// TODO Auto-generated destructor stub
	Stop();
}

void Thread::Start() {
	int res = pthread_create(&m_Tid, NULL, m_lpThreadFun, m_lpThreadCxt);
	ASSERT_SUCCESS(res);
}

void Thread::Stop() {
	int res = pthread_cancel(m_Tid);
	ASSERT_SUCCESS(res);
}

pthread_t Thread::GetThreadID() {
	return m_Tid;
}

void Thread::Join() {
	int res = pthread_join(m_Tid, NULL);
	ASSERT_SUCCESS(res);
}

void Thread::Detach() {
	int res = pthread_detach(m_Tid);
	ASSERT_SUCCESS(res);
}

bool Thread::IsStarted() {
	return (0 != m_Tid);
}

bool Thread::IsSelf() {
	return (pthread_self() == m_Tid);
}

// test
#include <stdio.h>
#include <unistd.h>
void* _ThreadFun(void *cxt) {
	Thread *thread = *(Thread **)cxt;

	for (int i=0; ; i++) {
		printf("%ld-%d\n", thread->GetThreadID(), i);
		sleep(1);
	}

	printf("Thread-[%ld] stopped\n", thread->GetThreadID());
}

// static
void Thread::ThreadTest() {
	Thread* threads[2];
	for (int i=0; i<2; i++) {
		threads[i] = new Thread(_ThreadFun, threads + i);
		threads[i]->Start();
	}

	sleep(10);
	for (int j=0; j<2; j++) {
		threads[j]->Stop();
	}

	sleep(1000);
}

int Thread_main(int argc, char *argv[]) {
	Thread::ThreadTest();
	return 0;
}
