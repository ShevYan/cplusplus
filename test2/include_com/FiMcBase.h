#pragma once

#include "metacmddifs.h"

typedef struct _MB_HEADER {
	DWORD	ClientAddr;                 // DWORD 客户端机IP地址
	DWORD	MainMdsAddr;				// 主Mds的IP地址(路径各级各不相同)
	DWORD	BackMdsAddr;				// 备Mds的IP地址(路径各级各不相同)
	USHORT	MajorVersion ;
	USHORT	MinorVersion ;
	DWORD	Size ;						// whole message block size
	DWORD	Command;                    // metaCmd code
	DWORD	CmdFlag;				    // 0：正常元命令，1：客户端缓存直接发送的元命令
	DWORD	Flags;                      // DiMB Flags
	DWORD	Flags1;                     // DiMB Flags
	DWORD	Flags2;                     // DiMB Flags
	DWORD	Flags3;                     // DiMB Flags(未用) 
	DWORD	Flags4;                     // DiMB Flags(未用)
	FiErrData	ErrorCode;              // GetLastError()
	USHORT	Tid ;						// Authenticated user/group
	USHORT	Uid ;	   	                // Unauthenticated user id
	char    ByteOrder;                  // 0---big endian 1---little endian									
} MB_HEADER, * PMB_HEADER;

enum _MB_CMD_FLAG_
{
	MB_CMD_FLAG_NORMAL,
	MB_CMD_FLAG_CACHE,
	MB_CMD_FLAG_MDS
};