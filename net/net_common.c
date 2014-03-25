/*
 * net_common.c

 *
 *  Created on: Mar 19, 2014
 *      Author: shev
 */
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include "net_common.h"

void set_sock_opts(int sock) {
	int keepAlive = 1;    // set keep alive flag
	int keepIdle = 5;    // 30s, begin to send first keep alive packet after 30s
	int keepInterval = 5;    // 5s, between two packet send time
	int keepCount = 3;    // confirm disconnect after 3 times try

	int recvSize = SOCKET_RECV_BUFFER_SIZE;
	int sendSize = SOCKET_SEND_BUFFER_SIZE;
	int bNoDelay = 1;
	int nREUSEADDR = 1;

	//  int flag = ::fcntl(m_hSocket.get_socket(), F_GETFL, 0);
	//  int err = ::fcntl(m_hSocket.get_socket(), F_SETFL, flag | O_NONBLOCK);

	if (-1
			== setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
					(const char *) &recvSize, sizeof(int))) {
		assert(0);
	}

	if (-1
			== setsockopt(sock, SOL_SOCKET, SO_SNDBUF,
					(const char *) &sendSize, sizeof(int))) {
		assert(0);
	}

	if (-1
			== setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
					(const char*) &nREUSEADDR, sizeof(int))) {
		assert(0);
	}

	if (-1
			== setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*) &bNoDelay,
					sizeof(bNoDelay))) {
		assert(0);
	}

	if (-1
			== setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*) &keepAlive,
					sizeof(keepAlive))) {
		assert(0);
	}

	if (-1
			== setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, (char*) &keepIdle,
					sizeof(keepIdle))) {
		assert(0);
	}

	if (-1
			== setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, (char *) &keepInterval,
					sizeof(keepInterval))) {
		assert(0);
	}

	if (-1
			== setsockopt(sock, SOL_TCP, TCP_KEEPCNT, (char *) &keepCount,
					sizeof(keepCount))) {
		assert(0);
	}
}

void setnonblocking(int Sock) {
	int opts = fcntl(Sock, F_GETFD);
	opts = opts | O_NONBLOCK | O_ASYNC;
	fcntl(Sock, F_SETFD, opts);
}

int del_epoll_fd(int epollfd, int fd) {
	struct epoll_event ev;
	memset((void*) &ev, 0, sizeof(struct epoll_event));
	ev.data.fd = fd;
	ev.events = 0;
	if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) < 0) {
		return -1;
	}
	return 0;
}
