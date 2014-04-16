/*
 * AutoLock.cpp
 *
 *  Created on: Apr 14, 2014
 *      Author: shev
 */

#include "comm.h"
#include "AutoLock.h"

AutoLock::AutoLock() {
	// TODO Auto-generated constructor stub
	m_Lock.ExclusiveLock();
}

AutoLock::~AutoLock() {
	// TODO Auto-generated destructor stub
	m_Lock.Unlock();
}


// test
int AutoLock_main(int argc, char *argv[]) {
	{
		AutoLock al1;
		printf("test 1\n");
	}

	{
		AutoLock al2;
		printf("test 2\n");
	}

	{
		AutoLock al3;
		printf("test 3\n");
	}
}
