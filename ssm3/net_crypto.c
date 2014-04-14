/*
 * net_crypto.c
 *
 *  Created on: Mar 19, 2014
 *      Author: shev
 */


#include <stdio.h>
#include <string.h>
#include "net_crypto.h"
#include "openssl/md5.h"

void crypto_md5(unsigned char *data, int len, unsigned char *md) {
	MD5_CTX ctx = { 0 };
	MD5_Init(&ctx);
	MD5_Update(&ctx, data, len);
	MD5_Final(md, &ctx);

}

void crypto_crc(char *src, int len, int* crc) {
	int i = 0;
	*crc = 0;

	for (i=0; i<len; i++) {
		*crc += src[i];
	}
}

int crypto_hash(unsigned char *src, int len, int mod) {
	int crc = 0;
	crypto_crc(src, len, &crc);
	return crc % mod;
}
