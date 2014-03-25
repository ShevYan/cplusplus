/*
 * net_thread.c
 *
 *  Created on: Mar 19, 2014
 *      Author: shev
 */

#include <assert.h>
#include "net_thread.h"


int start_thread(thread_t *thread) {
	assert(thread);
	assert(thread->t_fun);
	return pthread_create(&thread->t_id, NULL, thread->t_fun, thread->t_cxt);
}

int stop_thread(thread_t *thread) {
	// not suppot.
	assert(0);
	return -1;
}

int join_thread(thread_t *thread) {
	return pthread_join(thread->t_id, NULL);
}
