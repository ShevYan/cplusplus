/*
 * net_work_thread.h
 *
 *  Created on: Mar 18, 2014
 *      Author: shev
 */

#ifndef NET_WORK_THREAD_H_
#define NET_WORK_THREAD_H_
#include <stdbool.h>
#include "list.h"
#include "semaphore.h"
#include "net_thread.h"
#include "net_session.h"
#include "net_msg.h"

typedef struct _work_thread {
	thread_t thread;
	struct list_head head;
	pthread_mutex_t lock;
	sem_t sem;
	bool is_idle;
	bool is_server;
	void* svr_or_clt;
}work_thread_t;

typedef struct _work_thread_pool {
	work_thread_t *work_threads;
	int num;
	bool is_server;
	void* svr_or_clt;
}work_thread_pool_t;

/*
 * Thread:
 * 	1) work thread
 * */
work_thread_pool_t* create_work_thread_pool(int num, bool is_server, void* svr_or_clt);
int push_msg(work_thread_pool_t *pool, msg_t *m);

#endif /* NET_WORK_THREAD_H_ */
