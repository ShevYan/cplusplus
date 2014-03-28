/*
 * net_client.h
 *
 *  Created on: Mar 18, 2014
 *      Author: shev
 */

#ifndef NET_CLIENT_H_
#define NET_CLIENT_H_
#include "net_global.h"
#include "net_work_thread.h"
#include "net_common.h"

typedef struct _client {
	thread_t epoll_thread;
	thread_t reconn_thread;

	char clt_svr_ip[IP_LEN];
	int clt_svr_port;

	CB_CONNECTED clt_cb_conn;
	void* clt_cb_conn_cxt;

	CB_DISCONNECTED clt_cb_disconn;
	void* clt_cb_disconn_cxt;

	CB_RECV clt_cb_recv;
	void* clt_cb_recv_cxt;

	CB_RECONNECT clt_cb_reconn;
	void* clt_cb_reconn_cxt;

	session_t *session;
	int clt_retry_interval;
	int clt_max_retry;

	int sock;
	work_thread_pool_t *work_threads;
	pthread_mutex_t clt_lock;
}client_t;

/*
 * Threads:
 * 	1) connecting thread;
 * 	2) session->msg_thread;
 * 	3) session->work_thread;
 * */
int client_init(client_t *clt, char *clt_svr_ip, int clt_svr_port,
		CB_CONNECTED cb_conn, void *cb_conn_cxt,
		CB_DISCONNECTED cb_disconn, void *cb_disconn_cxt,
		CB_RECV cb_recv, void *cb_recv_cxt,
		CB_RECONNECT cb_reconn, void *cb_reconn_cxt);
int client_start(client_t *clt);
int client_stop(client_t *clt);
int client_join(client_t *clt);
int client_send(client_t *clt, char *buf, int len);
int client_async_send(client_t *clt, char *buf, int len, send_completion *cb, void *cbdata);


#endif /* NET_CLIENT_H_ */
