/*
 * net_global.h
 *
 *  Created on: Mar 18, 2014
 *      Author: shev
 */

#ifndef NET_GLOBAL_H_
#define NET_GLOBAL_H_

#define IP_LEN	16

// connected
typedef void* (*CB_CONNECTED)(void *cxt, const char* host, int port);

// disconnected
typedef void* (*CB_DISCONNECTED)(void *cxt, const char *host, int port);

// retry
typedef void* (*CB_RECONNECT)(void *cxt, char *ip, int port, int retry_times);

// recved
typedef void* (*CB_RECV)(void *cxt, char *buf, int len);


#define RECV_BUF_SIZE 64*1024
#define SPRINT_MSG_ID 101
#define SPRINT_MSG_LEN 102
#define SPRINT_MAGIC 103
#define SPRINT_CRC 104
#define SPRINT_DATA 105
#define MSG_MAGIC 12345678

// epoll related
#define MAX_EVENTS	1024

#endif /* NET_GLOBAL_H_ */
