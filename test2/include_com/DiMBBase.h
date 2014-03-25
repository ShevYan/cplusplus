#ifndef _DiMBBase_h_
#define _DiMBBase_h_

#include "BaseType.h"
#include "metacmdbase.h"

#define DiMB_HEADER_PROTOCOL  (('D'<<24 )+ ('i' << 16) + ('M' << 8) + 'B' )


#ifndef DIFS_KERNELMODE_COMPILE

//错误码
//
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//   +------------------------------+------------------------------+
//   |  						    |           error id           |
//   +------------------------------+------------------------------+
#define   STATUS_DIFS_SUCCESS					0x00000000//成功
#define   STATUS_DIFS_ERROR_CREATE               0x00000001//文件打开失败
//...

#endif

//flags for DiMB
#define     DiMB_FLAG_RESP		0x00000001  //if this bit is set , the mb is for response
#pragma pack(push,1)
//typedef unsigned int			DWORD;
//typedef unsigned long			ULONG;
typedef struct _DiMB_HEADER {
    DWORD	Protocol;                   // Contains 'DiMB'
	USHORT	MajorVersion ;
	USHORT	MinorVersion ;
	DWORD	Size ;						// whole message block size
    DWORD	Command;                    // metaCmd code
    DWORD	Status;						// 32-bit error code
	DWORD	CmdFlag;				    // 0: 正常的命令, 1: 主备切换完成后，scm发送缓存的命令
    DWORD	Flags;                      // DiMB Flags
	DWORD	Flags1;                     // DiMB Flags
	DWORD	Flags2;                     // DiMB Flags
	DWORD	Flags3;                     // DiMB Flags(未用)
	DWORD	Flags4;                     // DiMB Flags(未用)
	DWORD	ErrorCode;                  // GetLastError()
	ULONG   ntStatus;
    USHORT	Tid ;						// Authenticated user/group
    USHORT	Uid ;	   	                // Unauthenticated user id
	char    ByteOrder;                  // 0---big endian 1---little endian									
} DiMB_HEADER, * PDiMB_HEADER;



#pragma pack(pop)

#endif
