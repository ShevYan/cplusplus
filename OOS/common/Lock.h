/*
 * Lock.h
 *
 *  Created on: Apr 14, 2014
 *      Author: shev
 */

#ifndef LOCK_H_
#define LOCK_H_
#include <pthread.h>

class Lock {
private:
	pthread_mutex_t m_Mutex;
public:
	Lock();
	virtual ~Lock();

	int TryLock();
	void ExclusiveLock();
	void ShareLock();
	void Unlock();
};

#endif /* LOCK_H_ */
