/*
 * net_msg.c
 *
 *  Created on: Mar 19, 2014
 *      Author: shev
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "net_msg.h"
#include "net_crypto.h"
#include "net_global.h"

void encode(msg_t *m, char **buf, int *len) {
	char *p = NULL;

	*len = m->msglen + 4 * sizeof(int);
	*buf = (char *)malloc(*len);
	p = *buf;

	memcpy(p, &m->msgid, sizeof(int));
	p += sizeof(int);

	memcpy(p, &m->msglen, sizeof(int));
	p += sizeof(int);

	memcpy(p, &m->crc, sizeof(int));
	p += sizeof(int);

	memcpy(p, &m->magic, sizeof(int));
	p += sizeof(int);

	memcpy(p, m->buf, m->msglen);
	p += m->msglen;

}

void decode(msg_t **m, char *buf, int len) {
	return ;
}

msg_t* malloc_msg(char *buf, int len) {
	msg_t *res = (msg_t *)malloc(sizeof(msg_t));
	assert(res);

	memset(res, 0x00, sizeof(msg_t));
	INIT_LIST_HEAD(&res->acr);
	res->buf = buf;
	res->msglen = len;
	res->magic = MSG_MAGIC;
	crypto_crc(buf, len, &res->crc);

	return res;
}

void free_msg(msg_t **msg) {
	assert(msg);
	assert(*msg);
	assert(NULL == (*msg)->buf);

	memset(*msg, 0x00, sizeof(msg_t));
	free(*msg);
	*msg = NULL;
}
