/*
 * ZDThread.h
 *
 *  Created on: Apr 1, 2014
 *      Author: shev
 */

#ifndef ZDTHREAD_H_
#define ZDTHREAD_H_
#include <pthread.h>
namespace ZDCommon {

class ZDThread {
private:
	pthread_t tid;

public:
	ZDThread();
	virtual ~ZDThread();
};

} /* namespace ZDCommon */
#endif /* ZDTHREAD_H_ */
