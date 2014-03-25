/*
 * net_work_thread.c
 *
 *  Created on: Mar 19, 2014
 *      Author: shev
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "list.h"
#include "net_work_thread.h"
#include "net_client.h"
#include "net_server.h"

static void* _net_work_thread_fun(void *cxt) {
	msg_t *msg = NULL;
	work_thread_t *work_thread = (work_thread_t *)cxt;
	assert(cxt);

	while (true) {
		sem_wait(&work_thread->sem);
		pthread_mutex_lock(&work_thread->lock);
		work_thread->is_idle = false;
		assert(!list_empty(&work_thread->head));
		msg = (msg_t *)work_thread->head.next;
		assert(msg);
		list_del(work_thread->head.next);
		work_thread->is_idle = true;
		pthread_mutex_unlock(&work_thread->lock);

		if (work_thread->is_server) {
			server_t *svr = (server_t *)work_thread->svr_or_clt;
			svr->svr_cb_recv(svr->svr_cb_recv_cxt, msg->buf, msg->msglen);
		} else {
			client_t *clt = (client_t *)work_thread->svr_or_clt;
			clt->clt_cb_recv(clt->clt_cb_recv_cxt, msg->buf, msg->msglen);
		}

		free(msg->buf);
		msg->buf = NULL;
		free_msg(&msg);
	}

	return NULL;
}

work_thread_t* select_thread(work_thread_pool_t *pool) {
	int i = 0;
	for (i=0; i<pool->num; i++) {
		if (pool->work_threads[i].is_idle) {
			return pool->work_threads + i;
		}
	}

	return pool->work_threads + 0;
}

int push_msg(work_thread_pool_t *pool, msg_t *m) {
	work_thread_t *thread = select_thread(pool);

	pthread_mutex_lock(&thread->lock);
	list_add_tail(&m->acr, &thread->head);
	sem_post(&thread->sem);
	pthread_mutex_unlock(&thread->lock);

	return 0;
}

work_thread_pool_t* create_work_thread_pool(int num, bool is_server, void* svr_or_clt) {
	int i = 0;
	work_thread_pool_t *res = (work_thread_pool_t *)malloc(sizeof(work_thread_pool_t));
	assert(res);

	res->num = num;
	res->is_server = is_server;
	res->svr_or_clt = svr_or_clt;
	res->work_threads = (work_thread_t *)malloc(num * sizeof(work_thread_t));
	assert(res->work_threads);

	/* init threads*/
	for (i=0; i<num; i++) {
		INIT_LIST_HEAD(&res->work_threads[i].head);
		pthread_mutex_init(&res->work_threads[i].lock, NULL);
		sem_init(&res->work_threads[i].sem, 0, 0);
		res->work_threads[i].is_idle = true;
		res->work_threads[i].is_server = is_server;
		res->work_threads[i].svr_or_clt = svr_or_clt;
		res->work_threads[i].thread.t_cxt = res->work_threads + i;
		res->work_threads[i].thread.t_fun = _net_work_thread_fun;
		start_thread(&res->work_threads[i].thread);
	}

	return res;
}
