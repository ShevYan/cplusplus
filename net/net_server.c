/*
 * net_server.c
 *
 *  Created on: Mar 19, 2014
 *      Author: shev
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <linux/tcp.h>
#include <arpa/inet.h>
#include "net_server.h"



int server_start(server_t *svr) {
	int res = 0;
	int epollfd = 0;
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];
	int listen_sock = 0;
	int conn_sock = 0;
	struct sockaddr_in s_add;
	int addrlen = sizeof(struct sockaddr_in);
	int sock_flag = 0;
	assert(svr);

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == listen_sock) {
		perror("socket");
		return -errno;
	}

	memset(&s_add, 0x00, sizeof(struct sockaddr_in));
	s_add.sin_family = AF_INET;
	s_add.sin_addr.s_addr = htonl(INADDR_ANY );
	s_add.sin_port = htons(svr->svr_port);
	if (-1
			== bind(listen_sock, (struct sockaddr *) (&s_add),
					sizeof(struct sockaddr))) {
		perror("bind");
		return -errno;
	}

	if (-1 == listen(listen_sock, MAX_EVENTS)) {
		perror("listen");
		return -errno;
	}

	memset(&ev, 0x00, sizeof(struct epoll_event));
	memset(events, 0x00, sizeof(struct epoll_event) * MAX_EVENTS);

	epollfd = epoll_create(MAX_EVENTS);
	if (epollfd == -1) {
		perror("epoll_create");
		return -errno;
	}

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		return -errno;
	}

	svr->svr_listen_sock = listen_sock;
	svr->svr_epollfd = epollfd;

	res = start_thread(&svr->thread);
	printf("server started, listening on port [%d]\n", svr->svr_port);

	return res;
}

int server_stop(server_t *svr) {
	assert(0);
	return 0;
}

int server_join(server_t *svr) {
	return join_thread(&svr->thread);
}

int server_send(server_t *svr, const char *ip, char *buf, int len) {
	int res = -1;
	struct list_head *p = NULL;
	int sock = 0;
	msg_t *msg = NULL;
	char *stream = NULL;
	int stream_len = 0;
	assert(buf);

	pthread_mutex_lock(&svr->svr_lock);

	/* find a socket*/
	for (p=svr->svr_sessions.next; p!=&svr->svr_sessions; p=p->next) {
		session_t *ses = (session_t *)p;
		assert(ses);

		if (0 == strcmp(ses->remote_ip, ip)) {
			sock = ses->socket;
			break;
		}
	}
	pthread_mutex_unlock(&svr->svr_lock);

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

int server_async_send(server_t *svr, const char *ip, char *buf, int len,
		send_completion *cbfn, void *cbdata) {
	int res = -1;
	struct list_head *p = NULL;
	int sock = 0;
	msg_t *msg = NULL;
	char *stream = NULL;
	int stream_len = 0;
	assert(buf);

	pthread_mutex_lock(&svr->svr_lock);

	/* find a socket*/
	for (p=svr->svr_sessions.next; p!=&svr->svr_sessions; p=p->next) {
		session_t *ses = (session_t *)p;
		assert(ses);

		if (0 == strcmp(ses->remote_ip, ip)) {
			sock = ses->socket;
			break;
		}
	}
	pthread_mutex_unlock(&svr->svr_lock);

	if (0 == sock) {
		return -1;
	}

	msg = malloc_msg(buf, len);
	encode(msg, &stream, &stream_len);
	msg->msg_type = MSG_TYPE_ASYNC_SEND;
	msg->cbfn = cbfn;
	msg->cbdata = cbdata;
	msg->sock = sock;
	msg->stream = stream;
	msg->stream_len = stream_len;

	push_msg(svr->work_threads, msg);

	return 0;

}

int add_session(server_t *svr, int epollfd, int conn_sock,
		const char *client_ip, int client_port) {
	struct epoll_event ev;

	session_t *session = malloc_session(true, svr, conn_sock,
			epollfd, client_ip, client_port);

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = conn_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
		perror("epoll_ctl: conn_sock");
		return -errno;
	}

	pthread_mutex_lock(&svr->svr_lock);
	list_add(&session->acr,& svr->svr_sessions);
	pthread_mutex_unlock(&svr->svr_lock);

	return 0;
}

session_t* find_session(server_t *svr, int sock) {
	struct list_head *p = NULL;
	session_t *res = NULL;

	for (p=svr->svr_sessions.next; p!=&svr->svr_sessions; p=p->next) {
		res = (session_t *)p;
		assert(res);

		if (res->socket == sock) {
			break;
		}
	}

	return res;
}

static void* _epoll_thread(void *cxt) {
	server_t *svr = (server_t *)cxt;
	struct epoll_event events[MAX_EVENTS];
	int conn_sock = 0;
	int nfds = 0;
	struct sockaddr_in c_add;
	int i = 0;
	int addrlen = sizeof(struct sockaddr_in);
	char *ip = NULL;
	int port = 0;
	assert(svr);

	for (;;) {
		nfds = epoll_wait(svr->svr_epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			//perror("epoll_pwait");
			continue;
		}

		for (i = 0; i < nfds; ++i) {
			if (events[i].data.fd == svr->svr_listen_sock) {

				conn_sock = accept(svr->svr_listen_sock, (struct sockaddr *) &c_add,
						&addrlen);
				if (conn_sock == -1) {
					perror("accept");
					break;
				}

				setnonblocking(conn_sock);
				set_sock_opts(conn_sock);
				ip = (char *)inet_ntoa(c_add.sin_addr);
				printf("ip: %s\n", ip);
				port = ntohs(c_add.sin_port);

				add_session(svr, svr->svr_epollfd, conn_sock, ip, port);

				/* call user defined call back */
				if (svr->svr_cb_conn) {
					svr->svr_cb_conn(svr->svr_cb_conn_cxt, ip, port);
				}

			} else {
				session_t *session = find_session(svr, events[i].data.fd);
				bool first_read = true;
				if (session == NULL) {
					assert(0);
					del_epoll_fd(svr->svr_epollfd, events[i].data.fd);
					close(events[i].data.fd);
					continue;
				}

				if (events[i].events & EPOLLIN) {
					int len = 0;

					do {
						len = read(events[i].data.fd, session->recv_buf, session->recv_buf_size);
						if (-1 == len) {
							printf("read err[%d]\n", -errno);
						}

						if (-1 == len || first_read && len == 0) {
							/* disconnected: call user defined call back */
							if (svr->svr_cb_disconn) {
								svr->svr_cb_disconn(svr->svr_cb_disconn_cxt,
										session->remote_ip,
										session->remote_port);
							}

							/* remove from list */
							pthread_mutex_lock(&svr->svr_lock);
							list_del(&session->acr);
							free_session(&session);
							pthread_mutex_unlock(&svr->svr_lock);

							del_epoll_fd(svr->svr_epollfd, events[i].data.fd);
							close(events[i].data.fd);
							break;
						}

						/* serialize message */
						if (len > 0) {
							serialize(session, session->recv_buf, len);
						}

						first_read = false;
					} while (len > 0);

				} else if (events[i].events & EPOLLOUT) {
					/* reserved*/
				} else {
					printf("unused epoll events[0x%08x]\n", events[i].events);
				}
			}
		}
	}

	return NULL;
}

int server_init(server_t *svr, int port,
		CB_CONNECTED cb_conn, void *cb_conn_cxt,
		CB_DISCONNECTED cb_disconn, void *cb_disconn_cxt,
		CB_RECV cb_recv, void *cb_recv_cxt) {

	memset(svr, 0x00, sizeof(server_t));
	pthread_mutex_init(&svr->svr_lock, NULL);

	pthread_mutex_lock(&svr->svr_lock);
	svr->svr_port = port;
	svr->svr_cb_conn = cb_conn;
	svr->svr_cb_conn_cxt = cb_conn_cxt;
	svr->svr_cb_disconn = cb_disconn;
	svr->svr_cb_disconn_cxt = cb_disconn_cxt;
	svr->svr_cb_recv = cb_recv;
	svr->svr_cb_recv_cxt = cb_recv_cxt;
	INIT_LIST_HEAD(&svr->svr_sessions);

	svr->thread.t_cxt = svr;
	svr->thread.t_fun = _epoll_thread;

	svr->work_threads = create_work_thread_pool(5, true, svr);
	pthread_mutex_unlock(&svr->svr_lock);
	return 0;
}
