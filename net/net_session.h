/*
 * net_session.h
 *
 *  Created on: Mar 18, 2014
 *      Author: shev
 */

#ifndef NET_SESSION_H_
#define NET_SESSION_H_
#include <stdbool.h>
#include <string.h>
#include "list.h"
#include "net_msg.h"


typedef struct _session {
	struct list_head acr;
	bool is_server;
	void *server_or_client;
	int socket;
	int epollfd;
	char remote_ip[64];
	int remote_port;

	/* for serialization*/
	char *recv_buf;
	int recv_buf_size;
	int msgid;
	msg_t *cur_msg;
	int cur_sprint;
	int cur_size;
	int cur_pos;
}session_t;

/*
 * Serialize msg in epoll thread!
 * Threads:
 * 	1) work_thread_pool
 * */

session_t* malloc_session(bool is_svr, void *ptr, int socket,
		int epollfd, char *remote_ip, int remote_port);
void free_session(session_t **session);
void serialize(session_t *s, char *buf, int len);
//int session_send(session_t *s, char *buf, int len);

#endif /* NET_SESSION_H_ */
