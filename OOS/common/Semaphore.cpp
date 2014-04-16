/*
 * Semaphore.cpp
 *
 *  Created on: Apr 14, 2014
 *      Author: shev
 */

#include "comm.h"
#include "Semaphore.h"

Semaphore::Semaphore() {
	// TODO Auto-generated constructor stub
	int res = sem_init(&m_Sem, 0, 0);
	ASSERT_SUCCESS(res);
}

Semaphore::~Semaphore() {
	// TODO Auto-generated destructor stub
	int res = -1;

	res = sem_destroy(&m_Sem);
	ASSERT_SUCCESS(res);
}

void Semaphore::Post() {
	int res = sem_post(&m_Sem);
	ASSERT_SUCCESS(res);
}

void Semaphore::Wait() {
	int res = sem_wait(&m_Sem);
	ASSERT_SUCCESS(res);
}

// test
#include "Thread.h"
void* _ThreadFun1(void *cxt) {
	Semaphore *pSem = (Semaphore *)cxt;
	assert(pSem);

	int i = 0;
	while (true) {
		pSem->Wait();
		printf("waited - %d\n", i++);
	}

	return NULL;
}

int main(int argc, char *argv[]) {
	Semaphore sem;
	Thread thread(_ThreadFun1, &sem);
	thread.Start();

	for (int i=0; i<2; i++) {
		sem.Post();
		sleep(1);
	}
}
