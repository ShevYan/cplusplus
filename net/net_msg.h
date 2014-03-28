/*
 * net_msg.h
 *
 *  Created on: Mar 18, 2014
 *      Author: shev
 */

#ifndef NET_MSG_H_
#define NET_MSG_H_
#include "list.h"

#define MSG_TYPE_ASYNC_SEND 2

typedef struct _msg {
	struct list_head acr;
	int msg_type;
	void *cbfn;
	void *cbdata;
	int sock;
	char *stream;
	int stream_len;

	int msgid;
	int msglen;
	int crc;
	int magic;
	char *buf;
}msg_t;

/* NOTICE: need to free buf outside. */
void encode(msg_t *m, char **buf, int *len);

/* NOTICE: need to free msg outside. */
void decode(msg_t **m, char *buf, int len);

msg_t* malloc_msg(char *buf, int len);
void free_msg(msg_t **msg);
#endif /* NET_MSG_H_ */
