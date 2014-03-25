/*
 * net_client.c
 *
 *  Created on: Mar 19, 2014
 *      Author: shev
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <linux/tcp.h>
#include <arpa/inet.h>
#include "net_client.h"

static void* _epoll_thread(void *cxt) {
	client_t *clt = (client_t *)cxt;
	struct epoll_event ev;
	struct epoll_event events[10];
	int nfds = 0;
	int epollfd = 0;

	assert(clt);
	epollfd = epoll_create(10);
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = clt->sock;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clt->sock, &ev) == -1) {
		perror("epoll_ctl: sock");
		exit(EXIT_FAILURE);
	}

	for (;;) {

		nfds = epoll_wait(epollfd, events, 10, -1);
		if (nfds == -1) {
			perror("epoll_pwait");
			continue;
		}

		assert(nfds == 1);
		assert(events[0].data.fd == clt->sock);

		if (events[0].events & EPOLLIN) {
			int len = 0;
			bool first_read = true;

			do {
				len = read(events[0].data.fd, clt->session->recv_buf,
						clt->session->recv_buf_size);
				if (-1 == len) {
					printf("read err[%d]\n", -errno);
				}

				if (-1 == len || first_read && 0 == len) {
					/* disconnected: call user defined call back */
					if (clt->clt_cb_disconn) {
						clt->clt_cb_disconn(clt->clt_cb_disconn_cxt,
								clt->session->remote_ip,
								clt->session->remote_port);
					}

					/* remove from list */
					free_session(&clt->session);
					clt->sock = -1;
					del_epoll_fd(epollfd, events[0].data.fd);
					close(events[0].data.fd);
					break;

				}

				/* serialize message */
				serialize(clt->session, clt->session->recv_buf, len);
				first_read = false;
			} while (len > 0);

		} else if (events[0].events & EPOLLOUT) {
			/* reserved*/
		} else {
			printf("unused epoll events[0x%08x]\n", events[0].events);
		}
	}

	return NULL;
}


static void* _reconn_thread(void *cxt) {
	client_t *clt = (client_t *)cxt;
	struct sockaddr_in s_add;
	int retry_times = 0;
	int sock_flag = 0;
	int sock = 0;
	char *ip = NULL;
	int port = 0;
	assert(clt);

	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr= inet_addr(clt->clt_svr_ip);
	s_add.sin_port=htons(clt->clt_svr_port);

	for (; ;) {
		retry_times = 0;
		while (NULL == clt->session) {
			sock = socket(AF_INET, SOCK_STREAM, 0);
			if (-1 == sock) {
				perror("socket");
				break;;
			}

			set_sock_opts(sock);
			if (-1 == connect(sock, (struct sockaddr *)(&s_add), sizeof(struct sockaddr)) ) {
				perror("connect");
				break;
			}

			setnonblocking(sock);
			set_sock_opts(sock);

			ip = (char *) inet_ntoa(s_add.sin_addr);
			port = ntohs(s_add.sin_port);
			clt->sock = sock;
			clt->session = malloc_session(false, clt, clt->sock, 0, ip, port);

			/* call user defined call back */
			if (clt->clt_cb_conn) {
				clt->clt_cb_conn(clt->clt_cb_conn_cxt, ip, port);
			}

			start_thread(&clt->epoll_thread);

			retry_times++;
			if (retry_times > clt->clt_max_retry) {
				//break;
			} else {
				printf("try connecting to [%s:%d] retry[%d]\n", clt->clt_svr_ip,
						clt->clt_svr_port, retry_times);
			}
		}

		sleep(clt->clt_retry_interval);

	}

	return NULL;
}

int client_init(client_t *clt, char *clt_svr_ip, int clt_svr_port,
		CB_CONNECTED cb_conn, void *cb_conn_cxt,
		CB_DISCONNECTED cb_disconn, void *cb_disconn_cxt,
		CB_RECV cb_recv, void *cb_recv_cxt,
		CB_RECONNECT cb_reconn, void *cb_reconn_cxt) {

	memset(clt, 0x00, sizeof(client_t));

	pthread_mutex_init(&clt->clt_lock, NULL);

	strcpy(clt->clt_svr_ip, clt_svr_ip);
	clt->clt_svr_port = clt_svr_port;

	clt->clt_cb_conn = cb_conn;
	clt->clt_cb_conn_cxt = cb_conn_cxt;

	clt->clt_cb_disconn = cb_disconn;
	clt->clt_cb_disconn_cxt = cb_disconn_cxt;

	clt->clt_cb_recv = cb_recv;
	clt->clt_cb_recv_cxt = cb_recv_cxt;

	clt->clt_cb_reconn = cb_reconn;
	clt->clt_cb_reconn_cxt = cb_reconn_cxt;

	clt->clt_retry_interval = 1;
	clt->clt_max_retry = 100;

	clt->epoll_thread.t_cxt = clt;
	clt->epoll_thread.t_fun = _epoll_thread;
	clt->reconn_thread.t_cxt = clt;
	clt->reconn_thread.t_fun = _reconn_thread;

	clt->work_threads = create_work_thread_pool(1, false, clt);

	return 0;
}

int client_start(client_t *clt) {
	assert(clt);
	start_thread(&clt->reconn_thread);

	return 0;
}

int client_stop(client_t *clt) {
	assert(0);

	return -1;
}

int client_join(client_t *clt) {
	return join_thread(&clt->reconn_thread);
}

int client_send(client_t *clt, char *buf, int len) {
	int res = 0;
	int sock = 0;
	msg_t *msg = NULL;
	char *stream = NULL;
	int stream_len = 0;
	assert(buf);

	pthread_mutex_lock(&clt->clt_lock);
	if (clt->session) {
		sock = clt->session->socket;
	}
	pthread_mutex_unlock(&clt->clt_lock);

	if (0 == sock) {
		return -1;
	}

	msg = malloc_msg(buf, len);
	encode(msg, &stream, &stream_len);
	res = write(sock, stream, stream_len);
	msg->buf = NULL;
	free_msg(&msg);
	free(stream);
	stream = NULL;

	return res;
}
