/*
 * AutoLock.h
 *
 *  Created on: Apr 14, 2014
 *      Author: shev
 */

#ifndef AUTOLOCK_H_
#define AUTOLOCK_H_
#include "Lock.h"

class AutoLock {
private:
	Lock m_Lock;

public:
	AutoLock();
	virtual ~AutoLock();
};

#endif /* AUTOLOCK_H_ */
