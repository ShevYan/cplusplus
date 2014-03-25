/*
 * net_session.c
 *
 *  Created on: Mar 19, 2014
 *      Author: shev
 */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "net_global.h"
#include "net_msg.h"
#include "net_session.h"
#include "net_client.h"
#include "net_server.h"
#include "net_work_thread.h"

session_t* malloc_session(bool is_svr, void *ptr, int socket,
		int epollfd, char *remote_ip, int remote_port) {
	session_t *res = (session_t *)malloc(sizeof(session_t));

	assert(res);
	assert(ptr);
	memset(res, 0x00, sizeof(session_t));
	INIT_LIST_HEAD(&res->acr);
	res->is_server = is_svr;
	res->server_or_client = ptr;
	res->socket = socket;
	res->epollfd = epollfd;
	strcpy(res->remote_ip, remote_ip);
	res->remote_port = remote_port;

	res->recv_buf_size = RECV_BUF_SIZE;
	res->recv_buf = (char *)malloc(res->recv_buf_size);
	assert(res->recv_buf);
	memset(res->recv_buf, 0x00, RECV_BUF_SIZE);
	res->msgid = -1;
	res->cur_sprint = SPRINT_MSG_ID;
	res->cur_size = sizeof(int);
	res->cur_pos = 0;

	return res;
}

void free_session(session_t **session) {
	assert(session);
	assert(*session);
	assert((*session)->server_or_client);

	memset(*session, 0x00, sizeof(session_t));
	free(*session);
	*session = NULL;
}

void serialize(session_t *s, char *buf, int len) {
	assert(buf);
	char *p = buf;

	do {
		switch (s->cur_sprint) {
		case SPRINT_MSG_ID:
			// msgid,sizeof(int)
			if (s->cur_pos + buf + len - p >= s->cur_size) {
				memcpy((char *) &s->msgid + s->cur_pos, p,
						s->cur_size - s->cur_pos);
				p += s->cur_size - s->cur_pos;
				s->cur_msg = (msg_t *)malloc(sizeof(msg_t));
				s->cur_sprint = SPRINT_MSG_LEN;
				s->cur_size = sizeof(int);
				s->cur_pos = 0;
			} else {
				memcpy((char *) &s->msgid + s->cur_pos, p,
						buf + len - p);
				s->cur_pos += buf + len - p;
				p += buf + len - p;
			}
			break;

		case SPRINT_MSG_LEN:
			// msglen, sizeof(int)
			if (s->cur_pos + buf + len - p >= s->cur_size) {
				memcpy((char *) &s->cur_msg->msglen + s->cur_pos, p,
						s->cur_size - s->cur_pos);
				p += s->cur_size - s->cur_pos;
				s->cur_sprint = SPRINT_CRC;
				s->cur_size = sizeof(int);
				s->cur_pos = 0;

			} else {
				memcpy((char *) &s->cur_msg->msglen + s->cur_pos, p,
						buf + len - p);
				s->cur_pos += buf + len - p;
				p += buf + len - p;

			}
			break;

		case SPRINT_CRC:
			if (s->cur_pos + buf + len - p >= s->cur_size) {
				memcpy((char *) &s->cur_msg->crc + s->cur_pos, p, s->cur_size - s->cur_pos);

				p += s->cur_size - s->cur_pos;
				s->cur_sprint = SPRINT_MAGIC;
				s->cur_size = sizeof(int);
				s->cur_pos = 0;

			} else {
				memcpy((char *) &s->cur_msg->crc + s->cur_pos, p, buf + len - p);
				s->cur_pos += buf + len - p;
				p += buf + len - p;

			}
			break;

		case SPRINT_MAGIC:
			if (s->cur_pos + buf + len - p >= s->cur_size) {
				memcpy((char *) &s->cur_msg->magic + s->cur_pos, p,
						s->cur_size - s->cur_pos);
				p += s->cur_size - s->cur_pos;
				assert(s->cur_msg->magic == MSG_MAGIC);
				s->cur_sprint = SPRINT_DATA;
				s->cur_size = s->cur_msg->msglen;
				s->cur_msg->buf = (char *)malloc(s->cur_msg->msglen);
				s->cur_pos = 0;

			} else {
				memcpy((char *) &s->cur_msg->magic + s->cur_pos, p, buf + len - p);
				s->cur_pos += buf + len - p;
				p += buf + len - p;

			}
			break;

		case SPRINT_DATA:
			assert(s->cur_msg->buf);
			if (s->cur_pos + buf + len - p >= s->cur_size) {
				memcpy(s->cur_msg->buf + s->cur_pos, p, s->cur_size - s->cur_pos);
				p += s->cur_size - s->cur_pos;

				int crc = 0;
				crypto_crc((unsigned char *) s->cur_msg->buf,
						s->cur_msg->msglen, &crc);
				assert(crc == s->cur_msg->crc);

				// push_msg
				if (s->is_server) {
					server_t *svr = (server_t *)s->server_or_client;
					push_msg(svr->work_threads, s->cur_msg);
				} else {
					client_t *clt = (client_t *)s->server_or_client;
					push_msg(clt->work_threads, s->cur_msg);
				}

				s->cur_msg = NULL;

				// next round
				s->msgid = -1;
				s->cur_sprint = SPRINT_MSG_ID;
				s->cur_size = sizeof(int);
				s->cur_pos = 0;
			} else {
				memcpy(s->cur_msg->buf + s->cur_pos, p,
						buf + len - p);
				s->cur_pos += buf + len - p;
				p += buf + len - p;

			}
			break;

		default:
			assert(0);
			break;
		}
	} while (buf + len > p);
}

int session_send(session_t *s, char *buf, int len) {
	int res = 0;
	msg_t *msg = NULL;
	char *stream = NULL;
	int stream_len = 0;
	assert(s);
	assert(buf);

	msg = malloc_msg(buf, len);
	encode(msg, &stream, &stream_len);

	res = write(s->socket, stream, stream_len);

	msg->buf = NULL;
	free_msg(&msg);
	free(stream);
	stream = NULL;

	return res;
}
