#ifndef _METACMDBASE_H_
#define _METACMDBASE_H_

#include "kernel_user.h"

// The layout of the metacmd bits is:
//
//   1 1 1 1 1 1 1 1 1                           
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +----------------+---------------+-----------------------------+
//  |  cmd class     |   sub class   |        cmd id               |
//  +----------------+---------------+-----------------------------+
//  class  type  definition 
//  0:  for difs 
//  1:  for san manage
//  2:  for rpc manage

#define   MCMD_CLASS_DIFS			0
#define   MCMD_CLASS_SANMAN			1
#define   MCMD_CLASS_RPCMAN			2


#define METACMD_CLASS(cmdval)  (((cmdval) >> 24) & 0x000000ff)

/*
#define MAKE_METACMD(type,subtype,code) \
((DWORD) (((unsigned long)(type)<<24) | (unsigned long)(subtype)<<16 | ((unsigned long)(code))) )
*/

#define MAKE_METACMD(type,code) \
((DWORD) (((unsigned long)(type)<<24) | ((unsigned long)(code))) )

//add by like(2006-6-1 15:07:43)
#define GET_METACMDID(cmd) \
((DWORD) ((unsigned long)(cmd)&0xfff))

#define		MCMD_DIFS_NODEF			MAKE_METACMD(MCMD_CLASS_DIFS,0,0)
#define		MCMD_SANMAN_NODEF		MAKE_METACMD(MCMD_CLASS_SANMAN,0,0)
#define		MCMD_RPCMAN_NODEF		MAKE_METACMD(MCMD_CLASS_RPCMAN,0,0)

typedef unsigned short USHORT;
typedef struct _DiMB_DATE
{
	USHORT Day : 5;
	USHORT Month : 4;
	USHORT Year : 7;
}DiMB_DATE, * PDiMB_DATE;

typedef struct _DiMB_TIME
{
	USHORT Seconds : 5;
	USHORT Minutes : 6;
	USHORT Hours : 5;
}DiMB_TIME, * PDiMB_TIME;


#endif
