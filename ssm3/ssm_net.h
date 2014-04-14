/*
 * ssm_msg.h
 *
 *  Created on: Apr 2, 2014
 *      Author: shev
 */

#ifndef SSM_NET_H_
#define SSM_NET_H_
#include "net_server.h"

#define BLOCK_SIZE (512 << 10)
#define MSG_BLOCK_WRITE		1001

extern server_t *ssm_svr;
#pragma pack(1)
typedef struct _resp_file_block {
	int msgid;
	char filename[32];
	long fullsize;
	long filesize;
	long offset;
	int block_size;
	char block[BLOCK_SIZE];
}resp_file_block_t;

#define MSG_TYPE_REQ_CHARON_LAYOUT	2001
#define MSG_TYPE_RESP_CHARON_LAYOUT	2002

#define MSG_TYPE_REQ_CHARON_IO		2003
#define MSG_TYPE_RESP_CHARON_IO		2004

#define MSG_TYPE_REQ_READ_SEGMENT	2005
#define MSG_TYPE_RESP_READ_SEGMENT	2006

#define MSG_TYPE_REQ_LARGE_BLOCK	2007
#define MSG_TYPE_RESP_LARGE_BLOCK	2008

#define MSG_TYPE_REQ_CHARONCP		2009

typedef struct _req_large_block_test {
	int 	msgtype;
	int 	len;
	char	buf[128<<10];
}req_large_block_test_t;
void req_large_block_test_be(req_large_block_test_t *req);

typedef struct _req_charon_layout {
	int msgtype;
}req_charon_layout_t;
void req_charon_layout_be(req_charon_layout_t *req);

typedef struct _resp_charon_layout {
	int 	msgtype;
	long 	len;
	char 	layout_buf[1];
}resp_charon_layout_t;
void resp_charon_layout_be(resp_charon_layout_t *resp);

/* reserved */
typedef struct _req_charon_io {
	int msgtype;
}req_charon_io_t;
void req_charon_io_be(req_charon_io_t *req);

typedef struct _resp_charon_io {
	int msgtype;
	int is_read;
	int len;
	char io_cxt[1];
}resp_charon_io_t;
void resp_charon_io_be(resp_charon_io_t *resp);

typedef struct _req_read_segment {
	int msgtype;
	char diskname[32];
	long offset;
	long len;
}req_read_segment_t;
void req_read_segment_be(req_read_segment_t *req);

typedef struct _resp_read_segment {
	int msgtype;
	char diskname[32];
	long offset;
	long len;

	char segment_buf[1];
}resp_read_segment_t;
void resp_read_segment_be(resp_read_segment_t *resp);

typedef struct _req_charoncp {
	int msgtype;
	char src[256];
	char des[256];
}req_charoncp_t;
void req_charoncp_be(req_charoncp_t *req);
extern char g_srcfile[256];
extern char g_desfile[256];
extern int usr_interrupt;
#pragma pack()
extern server_t *ssm_svr;
int ssm_net_init();
int ssm_net_client_init();
int ssm_send_file(server_t *svr, const char *filename, long size);
void resp_charon_io(server_t *svr, int is_read, int len, char *io_cxt);

#endif /* SSM_MSG_H_ */
