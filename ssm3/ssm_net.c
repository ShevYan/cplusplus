/*
 * ssm_msg.c
 *
 *  Created on: Apr 2, 2014
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
#include "ssm_net.h"
#include "net_client.h"

void* _ssm_svr_connect(void *cxt, const char* host, int port) {
	printf("server: %s:%d connected.\n", host, port);
	fflush(stdout);
	server_t *svr = (server_t *)cxt;
	assert(svr);

	//resp_charon_io(svr, 0, 10, "0123456789");
	//ssm_send_file(ssm_svr, "file_3_64M_old", -1);
	//ssm_send_file(ssm_svr, "ssm_net.c", -1);

	return NULL;
}

void* _ssm_svr_disconnect(void *cxt, const char* host, int port) {
	printf("server: %s:%d disconnected.\n", host, port);
	fflush(stdout);
	return NULL;
}

void resp_charon_layout(server_t *svr, req_charon_layout_t *layout_req) {
	int res = -1;
	int fd = -1;
	struct stat st;
	resp_charon_layout_t *layout_resp = NULL;

	printf("req_charon_layout : %d\n", layout_req->msgtype);

	fd = open("ssm_out.txt", O_RDONLY);
	assert(fd > 0);

	res = fstat(fd, &st);
	assert(res == 0);

	layout_resp = (resp_charon_layout_t *)malloc(sizeof(resp_charon_layout_t) + st.st_size - 1);
	memset(layout_resp, 0x00, sizeof(resp_charon_layout_t) + st.st_size - 1);
	layout_resp->msgtype = MSG_TYPE_RESP_CHARON_LAYOUT;
	layout_resp->len = st.st_size;
	read(fd, layout_resp->layout_buf, layout_resp->len);

	close(fd);
	resp_charon_layout_be(layout_resp);
	server_send_all(svr, layout_resp, sizeof(resp_charon_layout_t) + st.st_size - 1);

	free(layout_resp);
	layout_resp = NULL;
}

void resp_charon_io(server_t *svr, int is_read, int len, char *io_cxt) {
	int res = -1;
	int fd = -1;
	struct stat st;
	resp_charon_io_t *io_resp = NULL;

	printf("=====resp_charon_io begin=====\n");
	printf("%s", io_cxt);
	printf("=====resp_charon_io end=====\n");

	io_resp = (resp_charon_io_t *)malloc(sizeof(resp_charon_io_t) + len - 1);
	memset(io_resp, 0x00, sizeof(resp_charon_io_t) + len - 1);
	io_resp->msgtype = MSG_TYPE_RESP_CHARON_IO;
	io_resp->is_read = is_read;
	io_resp->len = len;
	memcpy(io_resp->io_cxt, io_cxt, len);
	resp_charon_io_be(io_resp);
	server_send_all(svr, io_resp, sizeof(resp_charon_io_t) + len - 1);

	free(io_resp);
	io_resp = NULL;
}

void resp_read_segment(server_t *svr, req_read_segment_t *seg_req) {
	int res = -1;
	int fd = -1;
	long off = 0L;
	resp_read_segment_t *seg_resp = NULL;

//	printf("req_read_segment: %d %s %ld %ld\n",
//			seg_req->msgtype,
//			seg_req->diskname,
//			seg_req->offset,
//			seg_req->len);

	seg_resp = (resp_read_segment_t *)malloc(sizeof(resp_read_segment_t) + seg_req->len - 1);
	memset(seg_resp, 0x00, sizeof(resp_read_segment_t) + seg_req->len - 1);
	fd = open(seg_req->diskname, O_RDONLY);
	if (fd <= 0) {
		perror("open");
		goto out;
	}


	off = lseek(fd, seg_req->offset, 0);
	if (off != seg_req->offset) {
		perror("seek");
		goto out;
	}

	read(fd, seg_resp->segment_buf, seg_req->len);
	close(fd);

out:
	seg_resp->msgtype = MSG_TYPE_RESP_READ_SEGMENT;
	strcpy(seg_resp->diskname, seg_req->diskname);
	seg_resp->offset = seg_req->offset;
	seg_resp->len = seg_req->len;

	resp_read_segment_be(seg_resp);
	server_send_all(svr, seg_resp, sizeof(resp_read_segment_t) + seg_req->len - 1);

	free(seg_resp);
	seg_resp = NULL;
}



void* _ssm_svr_recv(void *cxt, char *buf, int len) {
	int msgtype = htobe32(*(int *)buf);
	server_t *svr = (server_t *)cxt;

	//printf("server: recv msg [%d]\n", len);

	switch (msgtype) {
	case MSG_TYPE_REQ_LARGE_BLOCK:
	{
		req_large_block_test_t *test = (req_large_block_test_t *)buf;
		req_large_block_test_be(buf);
	}
	break;

	case MSG_TYPE_REQ_CHARON_LAYOUT:
	{
		req_charon_layout_t *layout_req = (req_charon_layout_t *)buf;
		req_charon_layout_be(layout_req);
		assert(len == sizeof(req_charon_layout_t));
		resp_charon_layout(svr, layout_req);
	}
	break;

	case MSG_TYPE_REQ_CHARON_IO:
	{
		req_charon_io_t *io_req = (req_charon_io_t *)buf;
		req_charon_io_be(io_req);
		assert(len == sizeof(req_charon_io_t));
		//reserved
		assert(0);
	}
	break;

	case MSG_TYPE_REQ_READ_SEGMENT:
	{
		req_read_segment_t *seg_req = (req_read_segment_t *)buf;
		req_read_segment_be(seg_req);
		assert(len == sizeof(req_read_segment_t));
		resp_read_segment(svr, seg_req);
	}
	break;

	case MSG_TYPE_REQ_CHARONCP:
	{
		req_charoncp_t *cp_req = (req_charoncp_t *)buf;
		req_charoncp_be(cp_req);
		assert(len == sizeof(req_charoncp_t));
		resp_charoncp(svr, cp_req);
	}
	break;

	default:
		assert(0);
		break;
	}

	return NULL;
}

void req_large_block_test_be(req_large_block_test_t *req) {
	req->msgtype = htobe32(req->msgtype);
	req->len = htobe32(req->len);
}

void req_charon_layout_be(req_charon_layout_t *req) {
	 req->msgtype = htobe32(req->msgtype);
}

void resp_charon_layout_be(resp_charon_layout_t *resp) {
	resp->msgtype = htobe32(resp->msgtype);
	resp->len = htobe64(resp->len);
}

void req_read_segment_be(req_read_segment_t *req) {
	req->msgtype = htobe32(req->msgtype);
	req->offset = htobe64(req->offset);
	req->len = htobe64(req->len);
}

void resp_read_segment_be(resp_read_segment_t *resp) {
	resp->msgtype = htobe32(resp->msgtype);
	resp->offset = htobe64(resp->offset);
	resp->len = htobe64(resp->len);
}

void req_charoncp_be(req_charoncp_t *req) {
	req->msgtype = htobe32(req->msgtype);
}

void req_charon_io_be(req_charon_io_t *req) {
	assert(0); //reserved.
}

void resp_charon_io_be(resp_charon_io_t *resp) {
	resp->msgtype = htobe32(resp->msgtype);
	resp->is_read = htobe32(resp->is_read);
	resp->len = htobe32(resp->len);
}

server_t *ssm_svr = NULL;
client_t *ssm_client = NULL;

int ssm_net_init() {
	ssm_svr = (server_t *) malloc(sizeof(server_t));
	server_init(ssm_svr, 8080, _ssm_svr_connect, ssm_svr,
			_ssm_svr_disconnect, ssm_svr,
			_ssm_svr_recv, ssm_svr );
	return server_start(ssm_svr);
	//server_join(ssm_svr);
	//free(ssm_svr);
	//ssm_svr = NULL;


}

int ssm_net_uninit() {
	return 0;
}

int ssm_net_client_init() {
	ssm_client = (client_t *) malloc(sizeof(client_t));
	client_init(ssm_client, "127.0.0.1", 8080, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	client_start(ssm_client);
	if (ssm_client->session == NULL) {
		ms_sleep(15);
	}
	return 0;
}

int ssm_charoncp(const char *src, const char *des) {
	req_charoncp_t *cp_req = NULL;
	struct stat st;
	int res = -1;

	if (-1 == stat(src, &st)) {
		perror("open file: ");
		return -errno;
	}

	cp_req = (req_charoncp_t *) malloc(sizeof(req_charoncp_t));
	memset(cp_req, 0x00, sizeof(req_charoncp_t));

	cp_req->msgtype = MSG_TYPE_REQ_CHARONCP;
	strcpy(cp_req->src, src);
	strcpy(cp_req->des, des);

	req_charoncp_be(cp_req);

	res = client_send(ssm_client, cp_req, sizeof(req_charoncp_t));

	free(cp_req);
	cp_req = NULL;

	return res;
}

char g_srcfile[256];
char g_desfile[256];
void resp_charoncp(server_t *svr, req_charoncp_t *charoncp_req) {
	int res = -1;
	int fd = -1;
	long off = 0L;

	printf("recv charoncp cmd: [%s]->[%s]\n",
			charoncp_req->src,
			charoncp_req->des);


	memset(g_srcfile, 0x00, sizeof(g_srcfile));
	memset(g_desfile, 0x00, sizeof(g_desfile));

	strcpy(g_srcfile, charoncp_req->src);
	strcpy(g_desfile, charoncp_req->des);

	if (0 == strcmp("exit", g_desfile)) {
		exit(0);
	}
	usr_interrupt = 1;
}
