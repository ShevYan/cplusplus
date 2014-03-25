/*++

Copyright (c) 2006  sobey Corporation

Module Name:

    error.h

Abstract:

    define error code that will be used in file system development.

Author:

    difs_sobey   2006-7-31  

Revision History:

--*/



#ifndef _ERROR_H_
#define _ERROR_H_

/* INCLUDES **************************************************************/

#include "difstypes.h"
#include "kernel_user.h"

#ifdef DIFS_KERNELMODE_COMPILE
#include "ntifs.h"
#endif

/* DEFINITIONS ***********************************************************/

// //there are only 2 type error code. difsError, ntstatus
// #define DI_ERROR_TYPE                       2
                                          
// the layout of DISTATUS code                                                 
//                                               
//    31     28    27     16  15       0
//   /----------/-----------/------------/
//   | Types    |  Class    |  ErrorCode |
//   |          |           |            |
//   \----------\-----------\------------\
// 
// 1. types: sucess, information, warning, error
// 2. class: define as following
// 3. errorCode: the code that programmers can define themself. 
// 
                                       
//types define
#define DISTSTYPE_SUCCESS                       1
#define DISTSTYPE_INFO                          2
#define DISTSTYPE_WARNING                       4
#define DISTSTYPE_ERROR                         8

//class define 
#define DISTSCLASS_DIFS                         1
#define DISTSCLASS_OS                           2 
#define DISTSCLASS_SSMSVR                       3 
#define DISTSCLASS_SCMSVR                       4 
#define DISTSCLASS_SCMD                         5 
#define DISTSCLASS_RPC                          6
#define DISTSCLASS_FSLD                         7
//add class define here
                               
#define MAKE_ERR_CODE(type,classes,code)    \
    (DWORD)(((unsigned long)(type) << 28) | ( (unsigned long)(classes) << 16 ) | ((unsigned long)(code)))

#define DI_STS_SUCCESS(status)      ((status) & 0x10000000L)
#define DI_STS_INFO(status)         ((status) & 0x20000000L)
#define DI_STS_WARNING(status)      ((status) & 0x40000000L)
#define DI_STS_ERROR(status)        ((status) & 0x80000000L)
                                               
//INFO: define difs error code
//INFO: success
#define DISTS_DI_SUCCESS                    MAKE_ERR_CODE(DISTSTYPE_SUCCESS, DISTSCLASS_DIFS, 1) 

//INFO: info

//INFO: error
#define DISTS_DI_UNSUCCESSFUL               MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 1)
#define DISTS_DI_INSUFFICIENT_RESOURCES     MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 2) 
#define DISTS_DI_FILE_EXIST                 MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 3)
#define DISTS_DI_FILE_NOT_EXIST             MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 4)
#define DISTS_DI_NOT_FIND_SCMD              MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 5)
#define DISTS_DI_VOLUMESN_ERROR             MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 6)
#define DISTS_DI_INVALID_PARAMETERS         MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 7)
#define DISTS_DI_WRITEDISK_FAIL             MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 8)
#define DISTS_DI_UNDO_WRITE_DISK_FAIL       MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 9)
#define DISTS_DI_NOT_ENOUGH_DISKSPACE       MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 10)
#define DISTS_DI_ERROR_INSERT_INDEX_ENTRY   MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 11)
#define DISTS_DI_ERROR_DELETE_INDEX_ENTRY   MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 12)
#define DISTS_DI_NO_SUCH_FILES              MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 13)   
#define DISTS_DI_NO_MORE_FILES              MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 14)
#define DISTS_DI_NO_ENOUGH_DISKSPACE        MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 15)
#define DISTS_DI_ERR_LOAD_INDEX_INODE       MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 16)
#define DISTS_DI_ERR_LOAD_BITMAP            MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 17)
#define DISTS_DI_ERR_DELBTREE               MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 18)
#define DISTS_DI_ERR_INIT_SIMQ              MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 19)
#define DISTS_DI_ERR_READ_DISK_DATA         MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 20)
#define DISTS_DI_DEVICE_NOT_CONNECTED       MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_DIFS, 21)//对应NTSTATUS中的STATUS_DEVICE_NOT_CONNECTED

//INFO:Define FSLD ERROR CODE
#define FSLD_DI_SUCCESS                    MAKE_ERR_CODE(DISTSTYPE_SUCCESS, DISTSCLASS_FSLD, 1) 
//INFO: error
#define FSLD_DI_UNSUCCESSFUL               MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_FSLD, 1)
#define FSLD_DI_NOENOUGH_MEM               MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_FSLD, 2)//内存不够
#define FSLD_DI_WRITEDISK_FAIL             MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_FSLD, 3)//写磁盘失败    
#define FSLD_DI_LISETERR                   MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_FSLD, 4)//list错误
#define FSLD_DI_QUEUEERR                   MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_FSLD, 5)//queue错误
                                                                                             // 
typedef u32     DISTATUS;
typedef u32*    PDISTATUS;


//INFO: define SSMSvr error code
//INFO: success
#define SSM_DI_SUCCESS                    MAKE_ERR_CODE(DISTSTYPE_SUCCESS, DISTSCLASS_SSMSVR, 1) 
//INFO: error
#define SSM_DI_UNSUCCESSFUL               MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 1)
#define SSM_DI_NOENOUGH_MEM               MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 2)//内存不够
#define SSM_DI_PARAM_INVALID              MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 3)//该元命令参数无效
#define SSM_DI_DELETE_PENDING             MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 4)//该文件(目录)正在删除
#define SSM_DI_NOPARENTDIROBJECT          MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 5)//父目录没有打开(根据父目录物理id没有找到目录对象)
#define SSM_DI_NOFILEOBJECT               MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 6)//根据文件(目录)物理id没有找到该文件(目录)对象
#define SSM_DI_RENAME_PENDING             MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 7)//该文件(目录)正在更名
#define SSM_DI_PARENTDIR_DELETE_PENDING   MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 8)//父目录正在删除
#define SSM_DI_NOTSHAREDELETE             MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 9)//文件(目录)不共享删除
#define SSM_DI_NOTSHAREREADWRITE          MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 10)//文件(目录)不支持共享读写
#define SSM_DI_NOTSHAREWRITE              MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 11)//文件(目录)不支持共享写
#define SSM_DI_FILEISOPEN                 MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 12)//文件已经打开(CREATE NEW时 或者删除和更名时)
#define SSM_DI_DESTFILEISOPEN             MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 13)//目标文件已经打开(删除和更名时)
#define SSM_DI_FILETYPEERROR_DIR          MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 14)//文件类型不对(fsd返回的文件类型不符,不是目录)
#define SSM_DI_FILEEXIST                  MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 15)//文件已经存在
#define SSM_DI_SWITCHDATA_ERROR           MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 16)//scm切换数据不完整
#define SSM_DI_SWITCHDATA_NULL            MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 17)//scm切换数据为空
#define SSM_DI_FILETYPEERROR_FILE         MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 18)//文件类型不对(fsd返回的文件类型不符,不是文件)
#define SSM_DI_BUFFER_OVERFLOW            MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 19)//查询卷标时内存不够
#define SSM_DI_CMDSTATE_ERROR             MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 20)//当前SSM状态不能处理的命令
#define SSM_DI_SECONDARY_CREATE			  MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_SSMSVR, 21)//

// INFO: define RPC error code
#define  RPC_DI_SENDERR                   MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_RPC, 1)//发送RPC消息失败
#define  RPC_DI_WRITESHAREMEMERR          MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_RPC, 2)//写共享内存失败
#define  RPC_DI_CALL_TIMEOUT              MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_RPC, 3)//RPCCALL超时

#define  RPC_DI_CANCELCALL                MAKE_ERR_CODE(DISTSTYPE_ERROR, DISTSCLASS_RPC, 4)//CANCELCALL

// 为windows上层的错误码定义一个驱动里的替身，以方便进行转化
#ifdef DIFS_KERNELMODE_COMPILE

#define D_ERROR_FILE_NOT_FOUND              2
#define D_ERROR_PATH_NOT_FOUND              3
#define D_ERROR_ACCESS_DENIED               5
#define D_ERROR_WRITE_PROTECT               19
#define D_ERROR_FILE_EXISTS                 80
#define D_ERROR_DISK_FULL                   112
#define D_ERROR_ALREADY_EXISTS              183
#define D_ERROR_NO_SYSTEM_RESOURCES         1450

typedef struct _W32ErrCodeTable
{
    u32         w32Status;
    u32         diStatus;
    u32         sysStatus;
}W32ErrCodeTable, *PW32ErrCodeTable;

#define ERR_TABLE_NUM (sizeof(gDifsErrorTable) / sizeof(W32ErrCodeTable))

static W32ErrCodeTable gDifsErrorTable[] = {
    D_ERROR_ALREADY_EXISTS,      0,                                 STATUS_OBJECT_NAME_COLLISION,
    D_ERROR_FILE_EXISTS,         0,                                 STATUS_OBJECT_NAME_COLLISION,
    D_ERROR_FILE_NOT_FOUND,      0,                                 STATUS_OBJECT_NAME_NOT_FOUND,
    D_ERROR_PATH_NOT_FOUND,      0,                                 STATUS_OBJECT_PATH_NOT_FOUND,
    D_ERROR_ACCESS_DENIED,       0,                                 STATUS_ACCESS_DENIED,
    D_ERROR_NO_SYSTEM_RESOURCES, 0,                                 STATUS_INSUFFICIENT_RESOURCES,
    D_ERROR_DISK_FULL,           0,                                 STATUS_DISK_FULL,
    D_ERROR_WRITE_PROTECT,       0,                                 STATUS_MEDIA_WRITE_PROTECTED,

    0,                           SSM_DI_PARAM_INVALID,              STATUS_INVALID_PARAMETER,
    0,                           SSM_DI_DELETE_PENDING,             STATUS_DELETE_PENDING,
    0,                           SSM_DI_NOPARENTDIROBJECT,          STATUS_INVALID_PARAMETER,
    0,                           SSM_DI_NOFILEOBJECT,               STATUS_OBJECT_PATH_NOT_FOUND,
    0,                           SSM_DI_RENAME_PENDING,             STATUS_SHARING_VIOLATION,
    0,                           SSM_DI_PARENTDIR_DELETE_PENDING,   STATUS_DELETE_PENDING,
    0,                           SSM_DI_NOTSHAREDELETE,             STATUS_SHARING_VIOLATION,
    0,                           SSM_DI_NOTSHAREREADWRITE,          STATUS_SHARING_VIOLATION,
    0,                           SSM_DI_NOTSHAREWRITE,              STATUS_SHARING_VIOLATION,
    0,                           SSM_DI_FILEISOPEN,                 STATUS_SHARING_VIOLATION,
    0,                           SSM_DI_DESTFILEISOPEN,             STATUS_SHARING_VIOLATION,
    0,                           SSM_DI_FILETYPEERROR_DIR,          STATUS_FILE_IS_A_DIRECTORY,
    0,                           SSM_DI_FILEEXIST,                  STATUS_OBJECT_NAME_COLLISION,
    0,                           SSM_DI_SWITCHDATA_ERROR,           STATUS_INVALID_PARAMETER,
    0,                           SSM_DI_SWITCHDATA_NULL,            STATUS_INVALID_PARAMETER,
    0,                           SSM_DI_FILETYPEERROR_FILE,         STATUS_NOT_A_DIRECTORY,
    0,                           SSM_DI_BUFFER_OVERFLOW,            STATUS_BUFFER_OVERFLOW,
    0,                           DISTS_DI_NO_MORE_FILES,            STATUS_NO_MORE_FILES,
    0,                           DISTS_DI_NO_SUCH_FILES,            STATUS_NO_SUCH_FILE,
    0,                           DISTS_DI_NO_ENOUGH_DISKSPACE,      STATUS_DISK_FULL,

};

#else

#define D_ERROR_ACCESS_DENIED               5
#define D_ERROR_PATH_NOT_FOUND              3
#define D_ERROR_ALREADY_DISCONNECTED        0x80000025

#endif 

// typedef struct DifsError {
//     u32     difsErrorCode;
//     u32     ntStatus;
//     u8      * errorText;
// };

// u32 g_difsErrSwapTable[] = {
//
//     DISTS_DI_SUCCESS,                STATUS_SUCCESS,                 "success",
//     DISTS_UNSUCCESS,              STATUS_UNSUCCESSFUL,            "unsuccessful"
//     DISTS_INSUFFICIENT_RESOURCES, STATUS_INSUFFICIENT_RESOURCES   "insufficient resources"
//
// };

#endif
