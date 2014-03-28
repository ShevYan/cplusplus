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
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

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

#define BLOCK_SIZE (64 << 10)

static int g_fd = -1;
static const char *g_svr_filename = "./recv.dat";


typedef struct _data_block {
	long size;
	long offset;
	int block_size;
	char block[BLOCK_SIZE];
}data_block_t;

void* _cb_svr_recv(void *cxt, char *buf, int len) {
	data_block_t *db = (data_block_t *)buf;
	int write_bytes = 0;

	if (g_fd <= 0) {
		g_fd = open(g_svr_filename, O_CREAT | O_WRONLY, 0666);
		if (g_fd <= 0) {
			fprintf(stderr, "open file[%s] error[%d]\n", g_svr_filename, -errno);
		}
	}
	write_bytes = write(g_fd, db->block, db->block_size);
	db->offset += write_bytes;
	if (db->offset >= db->size) {
		assert(db->offset == db->size);
		close(g_fd);
		g_fd = -1;
	}

	return NULL;
}

void* _cb_clt_recv(void *cxt, char *buf, int len) {
	printf("client: recv[%s] [%d]\n", buf, len);
	return NULL;
}

void usage() {
	fprintf(stderr, "usage: \n");
	fprintf(stderr, "      net -s                     #start a server\n");
	fprintf(stderr, "      net -c <absolute_filepath> #start a client to transfer a file\n");
	exit(-1);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		usage();
	}

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
		client_t *clt = NULL;
		data_block_t *datablockp = NULL;
		int fd = -1;
		const char *filename = NULL;
		struct stat st;
		struct timeval tv;
		struct timeval tv2;
		if (argc != 3) {
			usage();
		}

		filename = argv[2];
		datablockp = (data_block_t *)malloc(sizeof(data_block_t));
		memset(datablockp, 0x00, sizeof(data_block_t));

		clt = (client_t *) malloc(sizeof(client_t));
		client_init(clt, "10.200.148.80", 12345, _cb_clt_connect, clt,
				_cb_svr_disconnect, clt,
				_cb_clt_recv, clt,
				NULL, NULL );
		client_start(clt);

		while (!clt->session) {
			sleep(1);
		}

		fd = open(filename, O_RDONLY);
		if (fd <= 0) {
			perror("open file");
			exit(-1);
		}

		gettimeofday(&tv, NULL);
		fstat(fd, &st);
		datablockp->size = st.st_size;
		while (datablockp->offset < datablockp->size) {
			datablockp->block_size = BLOCK_SIZE;
			int read_bytes = datablockp->block_size;//read(fd, datablockp->block, datablockp->block_size);
			if (read_bytes < datablockp->block_size) {
				datablockp->block_size = read_bytes;
			}


			client_async_send(clt, datablockp, sizeof(data_block_t), NULL, NULL);
			datablockp->offset += datablockp->block_size;
			gettimeofday(&tv2, NULL);
			printf("offset [%lld] [%.2f]MB/s\n", datablockp->offset,
					(double)datablockp->offset * 1.0 / ((tv2.tv_sec - tv.tv_sec)*1000000 + tv2.tv_usec-tv.tv_usec));
		}

		close(fd);
		fd = -1;

		client_join(clt);
		free(clt);
		clt = NULL;
	}

	return 0;
}
