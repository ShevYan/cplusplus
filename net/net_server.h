/*
 * net_server.h
 *
 *  Created on: Mar 18, 2014
 *      Author: shev
 */

#ifndef NET_SERVER_H_
#define NET_SERVER_H_
#include "net_global.h"
#include "net_work_thread.h"
#include "net_common.h"

typedef struct _server {
	thread_t thread;

	//char svr_ip[IP_LEN];
	int svr_port;

	CB_CONNECTED svr_cb_conn;
	void* svr_cb_conn_cxt;

	CB_DISCONNECTED svr_cb_disconn;
	void* svr_cb_disconn_cxt;

	CB_RECV svr_cb_recv;
	void* svr_cb_recv_cxt;

	int svr_listen_sock;
	int svr_epollfd;
	struct list_head svr_sessions;

	work_thread_pool_t *work_threads;

	pthread_mutex_t svr_lock;
}server_t;

/*
 * Threads:
 * 	1) epoll thread;
 * 	2) session->msg_thread;
 * 	3) session->work_thread;
 * */

int server_init(server_t *svr, int sv_port,
		CB_CONNECTED cb_conn, void *cb_conn_cxt,
		CB_DISCONNECTED cb_disconn, void *cb_disconn_cxt,
		CB_RECV cb_recv, void *cb_recv_cxt);
int server_start(server_t *svr);
int server_stop(server_t *svr);
int server_join(server_t *svr);
int server_send(server_t *svr, const char *ip, char *buf, int len);

int server_async_send(server_t *svr, const char *ip, char *buf, int len,
		send_completion *cbfn, void *cbdata);

#endif /* NET_SERVER_H_ */
