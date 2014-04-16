/*
 * Lock.cpp

 *
 *  Created on: Apr 14, 2014
 *      Author: shev
 */

#include "comm.h"
#include "Lock.h"
#include <assert.h>

Lock::Lock() {
	// TODO Auto-generated constructor stub
	pthread_mutex_init(&m_Mutex, NULL);
}

Lock::~Lock() {
	// TODO Auto-generated destructor stub
	pthread_mutex_destroy(&m_Mutex);
}

int Lock::TryLock() {
	return pthread_mutex_trylock(&m_Mutex);
}

void Lock::ExclusiveLock() {
	int res = pthread_mutex_lock(&m_Mutex);
	ASSERT_SUCCESS(res);
}

void Lock::ShareLock() {
	assert(0);
}

void Lock::Unlock() {
	int res = pthread_mutex_unlock(&m_Mutex);
	ASSERT_SUCCESS(res);
}

// test
#include <stdio.h>
#include <unistd.h>

int Lock_main(int argc, char *argv[]) {
	Lock lock;
	lock.ExclusiveLock();
	lock.Unlock();

	printf("2nd test\n");
	lock.ExclusiveLock();
	sleep(10);
	lock.Unlock();

	return 0;
}
