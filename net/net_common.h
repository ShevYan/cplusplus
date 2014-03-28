/*
 * net_common.h
 *
 *  Created on: Mar 19, 2014
 *      Author: shev
 */

#ifndef NET_COMMON_H_
#define NET_COMMON_H_

#define SOCKET_RECV_BUFFER_SIZE 10*1024*1024;
#define SOCKET_SEND_BUFFER_SIZE 10*1024*1024;

void set_sock_opts(int sock);
void setnonblocking(int Sock);
int del_epoll_fd(int epollfd, int fd);


typedef void (*send_completion)(int ret, void *cbdata);
void ms_sleep(int milliseconds);
#endif /* NET_COMMON_H_ */
