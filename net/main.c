/*
 * main.c
 *
 *  Created on: Mar 19, 2014
 *      Author: shev
 */
#include <stdio.h>
#include <stdlib.h>
#include "stdio.h"
#include "net_client.h"
#include "net_server.h"
#include <sys/epoll.h>

void* _cb_svr_connect(void *cxt, const char* host, int port) {
	printf("server: %s:%d connected.\n", host, port);

	return NULL;
}

void* _cb_clt_connect(void *cxt, const char* host, int port) {
	printf("client: %s:%d connected.\n", host, port);

	return NULL;
}

void* _cb_svr_disconnect(void *cxt, const char* host, int port) {
	printf("server: %s:%d disconnected.\n", host, port);

	return NULL;
}

void* _cb_clt_disconnect(void *cxt, const char* host, int port) {
	printf("client: %s:%d disconnected.\n", host, port);

	return NULL;
}

void* _cb_svr_recv(void *cxt, char *buf, int len) {
	//server_t *svr = (server_t *)cxt;
	//printf("server: recv[%s] [%d]\n", buf, len);
	//server_send(svr, "10.200.148.80", buf, len);

	return NULL;
}

void* _cb_clt_recv(void *cxt, char *buf, int len) {
	printf("client: recv[%s] [%d]\n", buf, len);
	return NULL;
}

int main(int argc, char *argv[]) {
	int i = 0;

	if (0 == strcmp(argv[1], "-s")) {
		server_t *svr = (server_t *) malloc(sizeof(server_t));
		server_init(svr, 12345, _cb_svr_connect, svr,
				_cb_svr_disconnect, svr,
				_cb_svr_recv, svr );
		server_start(svr);
		server_join(svr);
		free(svr);
		svr = NULL;

	} else {
		client_t *clt = (client_t *) malloc(sizeof(client_t));
		char buf[64 * 1024] = {1};
		client_init(clt, "10.200.148.80", 12345, _cb_clt_connect, clt,
				_cb_svr_disconnect, clt,
				_cb_clt_recv, clt,
				NULL, NULL );
		client_start(clt);

		while (!clt->session) {
			sleep(1);
		}

		for (i=0; i<1000000; i++) {
			client_send(clt, buf, sizeof(buf));
		}

		client_join(clt);
		free(clt);
		clt = NULL;
	}

	return 0;
}
