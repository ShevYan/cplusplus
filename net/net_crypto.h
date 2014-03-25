/*
 * net_crypto.h
 *
 *  Created on: Mar 18, 2014
 *      Author: shev
 */

#ifndef NET_CRYPTO_H_
#define NET_CRYPTO_H_

void crypto_md5(unsigned char *data, int len, unsigned char *md);
void crypto_crc(unsigned char *src, int len, int* crc);
int crypto_hash(unsigned char *src, int len, int mod);

#endif /* NET_CRYPTO_H_ */
