/*
 * Semaphore.h
 *
 *  Created on: Apr 14, 2014
 *      Author: shev
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_
#include <semaphore.h>

class Semaphore {
private:
	sem_t m_Sem;
public:
	Semaphore();
	virtual ~Semaphore();

	void Post();
	void Wait();
};

#endif /* SEMAPHORE_H_ */
