/*
 * net_thread.h
 *
 *  Created on: Mar 19, 2014
 *      Author: shev
 */

#ifndef NET_THREAD_H_
#define NET_THREAD_H_
#include <pthread.h>

typedef void* (*thread_fun)(void *cxt);

typedef struct _thread {
	pthread_t t_id;
	thread_fun t_fun;
	void	*t_cxt;
}thread_t;

int start_thread(thread_t *thread);
int join_thread(thread_t *thread);
int stop_thread(thread_t *thread);

#endif /* NET_THREAD_H_ */
