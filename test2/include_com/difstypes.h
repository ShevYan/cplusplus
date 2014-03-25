#ifndef _DIFS_TYPES_H
#define _DIFS_TYPES_H

#include "BaseType.h"
#include "DIFS_CFG.H"
/* 2-byte Unicode character type. */

typedef char        s8 ;
typedef short       s16 ;
typedef int         s32 ;
typedef __int64     s64 ;

#define CONVERT_TO_U8(value)               ((u8)(value))
#define CONVERT_TO_U16(value)              ((u16)(value))
#define CONVERT_TO_UL32(value)             ((ul32)(value))
#define CONVERT_TO_U32(value)              ((u32)(value))
#define CONVERT_TO_PU32(pvalue)            ((u32*)(pvalue))
#define CONVERT_TO_U64(value)              ((u64)(value))

#define CONVERT_TO_S8(value)               ((s8)(value))
#define CONVERT_TO_S16(value)              ((s16)(value))
#define CONVERT_TO_S32(value)              ((s32)(value))
#define CONVERT_TO_S64(value)              ((s64)(value))



#define __constant_le16_to_cpu(x)  x
#define __constant_le32_to_cpu(x)  x
#define __constant_le64_to_cpu(x)  x

#define __constant_cpu_to_le16(x)  x
#define __constant_cpu_to_le32(x)  x
#define __constant_cpu_to_le64(x)  x

#define const_le16_to_cpu(x)    __constant_le16_to_cpu(x)
#define const_le32_to_cpu(x)    __constant_le32_to_cpu(x)
#define const_le64_to_cpu(x)    __constant_le64_to_cpu(x)

#define const_cpu_to_le16(x)    __constant_cpu_to_le16(x)
#define const_cpu_to_le32(x)    __constant_cpu_to_le32(x)
#define const_cpu_to_le64(x)    __constant_cpu_to_le64(x)


#define UCHAR_T_SIZE_BITS 1

/*
 * Clusters are signed 64-bit values on NTFS volumes. We define two types, LCN
 * and VCN, to allow for type checking and better code readability.
 */
typedef s64 VCN;
typedef s64 LCN;
typedef u64 LSN;

/**
 * run_list_element - in memory vcn to lcn mapping array element
 * @vcn:    starting vcn of the current array element
 * @lcn:    starting lcn of the current array element
 * @length: length in clusters of the current array element
 * 
 * The last vcn (in fact the last vcn + 1) is reached when length == 0.
 * 
 * When lcn == -1 this means that the count vcns starting at vcn are not 
 * physically allocated (i.e. this is a hole / data is sparse).
 */
typedef struct
{   /* In memory vcn to lcn mapping structure element. */
    VCN vcn;    /* vcn = Starting virtual cluster number. */
    LCN lcn;    /* lcn = Starting logical cluster number. */
    u64 length; /* Run length in clusters. */
} Run_List_Element , * PRun_List_Element;

/**
 * run_list - in memory vcn to lcn mapping array including a read/write lock
 * @rl:     pointer to an array of run list elements
 * @lock:   read/write spinlock for serializing access to @rl
 * 
 */

typedef struct 
{
    ULONG  nElementNum ;
    PRun_List_Element pRle_array;
} Run_list, * PRun_list;

/* encoded runlist format
Offset in nibble to the beginning of the element  , Size in nibble ,    Description
0                       1               F=Size of the Offset field
1                       1               L=Size of the Length field
2                       2*L             Length of the run
2+2*L                   2*F             Offset to the starting LCN of the previous element
*/

typedef enum 
{
    CASE_SENSITIVE = 0,
    IGNORE_CASE = 1,
} IGNORE_CASE_BOOL;


//
//  The following are masks for the predefined standard access types
//
typedef ULONG DiFS_ACCESS_MASK;
typedef DiFS_ACCESS_MASK *PDiFS_ACCESS_MASK;
#define DiFS_DELETE                           (0x00010000L)
#define DiFS_READ_CONTROL                     (0x00020000L)
#define DiFS_WRITE_DAC                        (0x00040000L)
#define DiFS_WRITE_OWNER                      (0x00080000L)
#define DiFS_SYNCHRONIZE                      (0x00100000L)
#define DiFS_STANDARD_RIGHTS_REQUIRED         (0x000F0000L)
#define DiFS_STANDARD_RIGHTS_READ             (READ_CONTROL)
#define DiFS_STANDARD_RIGHTS_WRITE            (READ_CONTROL)
#define DiFS_STANDARD_RIGHTS_EXECUTE          (READ_CONTROL)
#define DiFS_STANDARD_RIGHTS_ALL              (0x001F0000L)
#define DiFS_SPECIFIC_RIGHTS_ALL              (0x0000FFFFL)
#define DiFS_ACCESS_SYSTEM_SECURITY           (0x01000000L)
#define DiFS_MAXIMUM_ALLOWED                  (0x02000000L)
#define DiFS_GENERIC_READ                     (0x80000000L)
#define DiFS_GENERIC_WRITE                    (0x40000000L)
#define DiFS_GENERIC_EXECUTE                  (0x20000000L)
#define DiFS_GENERIC_ALL                      (0x10000000L)

////
////**************   access mode ****************************
/*  5432 1098 7654 3210
    rWrC rLLL rSSS rAAA

    W - Write through mode. No read ahead or write behind allowed on
    this file or device. When the response is returned, data is
    expected to be on the disk or device.
    S - Sharing mode:
    0 - Compatibility mode
    1 - Deny read/write/execute (exclusive)
    2 - Deny write
    3 - Deny read/execute
    4 - Deny none
    A - Access mode
    0 - Open for reading
    1 - Open for writing
    2 - Open for reading and writing
    3 - Open for execute
    rSSSrAAA = 11111111 (hex FF) indicates FCB open (???)
    C - Cache mode
    0 - Normal file
    1 - Do not cache this file
    L - Locality of reference
    0 - Locality of reference is unknown
    1 - Mainly sequential access
    2 - Mainly random access
    3 - Random access with some locality
    4 to 7 - Currently undefined
    */
typedef USHORT DiFS_ACCESS_MODE ;
typedef DiFS_ACCESS_MODE * PDiFS_ACCESS_MODE ;

typedef ULONG   DiFS_SHARE_MODE ;
#define DiFS_FILE_SHARE_READ                 0x00000001  
#define DiFS_FILE_SHARE_WRITE                0x00000002  
#define DiFS_FILE_SHARE_DELETE               0x00000004  
#define DiFS_FILE_SHARE_VALID_FLAGS          0x00000007

//////
//********* Define the create disposition values **************//
typedef ULONG   DiFS_CREATE_DISPOSITION ;

#define DiFS_FILE_SUPERSEDE                  0x00000000
#define DiFS_FILE_OPEN                       0x00000001
#define DiFS_FILE_CREATE                     0x00000002
#define DiFS_FILE_OPEN_IF                    0x00000003
#define DiFS_FILE_OVERWRITE                  0x00000004
#define DiFS_FILE_OVERWRITE_IF               0x00000005
#define DiFS_FILE_MAXIMUM_DISPOSITION        0x00000005

//////
//***********   FILE ATTRIBUTE  ***************//
typedef ULONG  DiFS_FILE_ATTRIBUTE ;

/*
 * File attribute flags (32-bit).
 */
typedef enum 
{
    /* These flags are only present in the STANDARD_INFORMATION attribute (in the field file_attributes). */
    FILE_ATTR_READONLY              = const_cpu_to_le32(0x00000001),
    FILE_ATTR_HIDDEN                = const_cpu_to_le32(0x00000002),
    FILE_ATTR_SYSTEM                = const_cpu_to_le32(0x00000004),
    /* Old DOS volid. Unused in NT. = cpu_to_le32(0x00000008), */

    FILE_ATTR_DIRECTORY             = const_cpu_to_le32(0x00000010),
    /* FILE_ATTR_DIRECTORY is not considered valid in NT. It is reserved  for the DOS SUBDIRECTORY flag. */
    FILE_ATTR_ARCHIVE               = const_cpu_to_le32(0x00000020),
    FILE_ATTR_DEVICE                = const_cpu_to_le32(0x00000040),
    FILE_ATTR_NORMAL                = const_cpu_to_le32(0x00000080),

    FILE_ATTR_TEMPORARY             = const_cpu_to_le32(0x00000100),
    FILE_ATTR_SPARSE_FILE           = const_cpu_to_le32(0x00000200),
    FILE_ATTR_REPARSE_POINT         = const_cpu_to_le32(0x00000400),
    FILE_ATTR_COMPRESSED            = const_cpu_to_le32(0x00000800),

    FILE_ATTR_OFFLINE               = const_cpu_to_le32(0x00001000),
    FILE_ATTR_NOT_CONTENT_INDEXED   = const_cpu_to_le32(0x00002000),
    FILE_ATTR_ENCRYPTED             = const_cpu_to_le32(0x00004000),

    FILE_ATTR_VALID_FLAGS           = const_cpu_to_le32(0x00007fb7),
    /* FILE_ATTR_VALID_FLAGS masks out the old DOS VolId and the FILE_ATTR_DEVICE and preserves everything else. This mask
       is used to obtain all flags that are valid for reading. */
    FILE_ATTR_VALID_SET_FLAGS       = const_cpu_to_le32(0x000031a7),
    /* FILE_ATTR_VALID_SET_FLAGS masks out the old DOS VolId, the F_A_DEVICE, F_A_DIRECTORY, F_A_SPARSE_FILE, F_A_REPARSE_POINT,
       F_A_COMPRESSED and F_A_ENCRYPTED and preserves the rest. This mask is used to to obtain all flags that are valid for setting. */

    /* These flags are only present in the FILE_NAME attribute (in the field file_attributes). */
    FILE_ATTR_DUP_FILE_NAME_INDEX_PRESENT   = const_cpu_to_le32(0x10000000),
    /* This is a copy of the corresponding bit from the mft record, telling
       us whether this is a directory or not, i.e. whether it has an index root attribute or not. */
    FILE_ATTR_DUP_VIEW_INDEX_PRESENT        = const_cpu_to_le32(0x20000000),
    /* This is a copy of the corresponding bit from the mft record, telling
       us whether this file has a view index present (eg. object id index,
       quota index, one of the security indexes or the encrypting file system related indexes). */
} FILE_ATTR_FLAGS;



////////
//***********  Define the create/open option flags  ****************//
typedef ULONG  DiFS_CREATE_OPTION ;

#define DiFS_FILE_DIRECTORY_FILE                     0x00000001
#define DiFS_FILE_WRITE_THROUGH                      0x00000002
#define DiFS_FILE_SEQUENTIAL_ONLY                    0x00000004
#define DiFS_FILE_NO_INTERMEDIATE_BUFFERING          0x00000008
#define DiFS_FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define DiFS_FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define DiFS_FILE_NON_DIRECTORY_FILE                 0x00000040
#define DiFS_FILE_CREATE_TREE_CONNECTION             0x00000080
#define DiFS_FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define DiFS_FILE_NO_EA_KNOWLEDGE                    0x00000200
#define DiFS_FILE_OPEN_FOR_RECOVERY                  0x00000400
#define DiFS_FILE_RANDOM_ACCESS                      0x00000800
#define DiFS_FILE_DELETE_ON_CLOSE                    0x00001000
#define DiFS_FILE_OPEN_BY_FILE_ID                    0x00002000
#define DiFS_FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define DiFS_FILE_NO_COMPRESSION                     0x00008000
#define DiFS_FILE_RESERVE_OPFILTER                   0x00100000
#define DiFS_FILE_OPEN_REPARSE_POINT                 0x00200000
#define DiFS_FILE_OPEN_NO_RECALL                     0x00400000
#define DiFS_FILE_OPEN_FOR_FREE_SPACE_QUERY          0x00800000
#define DiFS_FILE_COPY_STRUCTURED_STORAGE            0x00000041
#define DiFS_FILE_STRUCTURED_STORAGE                 0x00000441
#define DiFS_FILE_VALID_OPTION_FLAGS                 0x00ffffff
#define DiFS_FILE_VALID_PIPE_OPTION_FLAGS            0x00000032
#define DiFS_FILE_VALID_MAILSLOT_OPTION_FLAGS        0x00000032
#define DiFS_FILE_VALID_SET_FLAGS                    0x00000036

///////////
//********** Define the I/O status information return values for NtCreateFile/NtOpenFile
typedef ULONG   DiFS_STATUS ;

#define DiFS_FILE_SUPERSEDED                 0x00000000
#define DiFS_FILE_OPENED                     0x00000001
#define DiFS_FILE_CREATED                    0x00000002
#define DiFS_FILE_OVERWRITTEN                0x00000003
#define DiFS_FILE_EXISTS                     0x00000004
#define DiFS_FILE_DOES_NOT_EXIST             0x00000005


typedef struct _DIFS_FILE_BOTH_DIR_INFORMATION 
{
    ULONG  NextEntryOffset;
    ULONG  FileIndex;
    __int64  CreationTime;
    __int64  LastAccessTime;
    __int64  LastWriteTime;
    __int64  ChangeTime;
    __int64  EndOfFile;
    __int64  AllocationSize;
    ULONG  FileAttributes;
    ULONG  FileNameLength;
    ULONG  EaSize;
    CCHAR  ShortNameLength;
    WCHAR  ShortName[12];
    WCHAR  FileName[1];
} DIFS_FILE_BOTH_DIR_INFORMATION, *DIFS_PFILE_BOTH_DIR_INFORMATION;

typedef struct _DIFS_FILE_DIRECTORY_INFORMATION 
{
    ULONG  NextEntryOffset;
    ULONG  FileIndex;
    LARGE_INTEGER  CreationTime;
    LARGE_INTEGER  LastAccessTime;
    LARGE_INTEGER  LastWriteTime;
    LARGE_INTEGER  ChangeTime;
    LARGE_INTEGER  EndOfFile;
    LARGE_INTEGER  AllocationSize;
    ULONG  FileAttributes;
    ULONG  FileNameLength;
    WCHAR  FileName[1];
} DIFS_FILE_DIRECTORY_INFORMATION, * PDIFS_FILE_DIRECTORY_INFORMATION;

typedef struct _DIFS_FILE_FULL_DIR_INFORMATION 
{
    ULONG  NextEntryOffset;
    ULONG  FileIndex;
    LARGE_INTEGER  CreationTime;
    LARGE_INTEGER  LastAccessTime;
    LARGE_INTEGER  LastWriteTime;
    LARGE_INTEGER  ChangeTime;
    LARGE_INTEGER  EndOfFile;
    LARGE_INTEGER  AllocationSize;
    ULONG  FileAttributes;
    ULONG  FileNameLength;
    ULONG  EaSize;
    WCHAR  FileName[1];
} DIFS_FILE_FULL_DIR_INFORMATION, *PDIFS_FILE_FULL_DIR_INFORMATION;


typedef struct _DIFS_FILE_NAMES_INFORMATION 
{
    ULONG  NextEntryOffset;
    ULONG  FileIndex;
    ULONG  FileNameLength;
    WCHAR  FileName[1];
} DIFS_FILE_NAMES_INFORMATION, *PDIFS_FILE_NAMES_INFORMATION;


typedef struct _DIFS_FILE_BASIC_INFORMATION {
    __int64  CreationTime;
    __int64  LastAccessTime;
    __int64  LastWriteTime;
    __int64  ChangeTime;
    ULONG  FileAttributes;
} DIFS_FILE_BASIC_INFORMATION, *PDIFS_FILE_BASIC_INFORMATION;

typedef struct _LockArea
{
    __int64  FileOffset ;
    __int64  Length ;
} LockArea, *PLockArea;

typedef struct _DIFS_FILE_NAME_INFORMATION 
{
    ULONG  FileNameLength;
    WCHAR  FileName[1];
} DIFS_FILE_NAME_INFORMATION, *PDIFS_FILE_NAME_INFORMATION;

typedef struct _DIFS_FILE_STANDARD_INFORMATION 
{
    __int64  AllocationSize;
    __int64  EndOfFile;
    ULONG  NumberOfLinks;
    BOOLEAN  DeletePending;
    BOOLEAN  Directory;
} DIFS_FILE_STANDARD_INFORMATION, *PDIFS_FILE_STANDARD_INFORMATION;

typedef struct _DIFS_FILE_FS_CONTROL_INFORMATION 
{
    LARGE_INTEGER FreeSpaceStartFiltering;
    LARGE_INTEGER FreeSpaceThreshold;
    LARGE_INTEGER FreeSpaceStopFiltering;
    LARGE_INTEGER DefaultQuotaThreshold;
    LARGE_INTEGER DefaultQuotaLimit;
    ULONG FileSystemControlFlags;
}DIFS_FILE_FS_CONTROL_INFORMATION, *PDIFS_FILE_FS_CONTROL_INFORMATION;

typedef struct _DIFS_FILE_FS_FULL_SIZE_INFORMATION
{
    LARGE_INTEGER TotalAllocationUnits;
    LARGE_INTEGER CallerAvailableAllocationUnits;
    LARGE_INTEGER ActualAvailableAllocationUnits;
    ULONG SectorsPerAllocationUnit;
    ULONG BytesPerSector;
} DIFS_FILE_FS_FULL_SIZE_INFORMATION, *PDIFS_FILE_FS_FULL_SIZE_INFORMATION;


typedef struct _DIFS_FILE_FS_SIZE_INFORMATION 
{
    LARGE_INTEGER TotalAllocationUnits;
    LARGE_INTEGER AvailableAllocationUnits;
    ULONG SectorsPerAllocationUnit;
    ULONG BytesPerSector;
} DIFS_FILE_FS_SIZE_INFORMATION, *PDIFS_FILE_FS_SIZE_INFORMATION;

typedef struct _DIFS_FILE_FS_VOLUME_INFORMATION 
{
    LARGE_INTEGER VolumeCreationTime;
    ULONG VolumeSerialNumber;
    ULONG VolumeLabelLength;
    BOOLEAN SupportsObjects;
    WCHAR VolumeLabel[DIFS_FILENAME_MAXLENGTH];
} DIFS_FILE_FS_VOLUME_INFORMATION, *PDIFS_FILE_FS_VOLUME_INFORMATION;

typedef struct _DIFS_FILE_FS_DEVICE_INFORMATION 
{
//  DEVICE_TYPE  DeviceType; 
  ULONG  Characteristics;
} DIFS_FILE_FS_DEVICE_INFORMATION, *PDIFS_FILE_FS_DEVICE_INFORMATION;

typedef struct _DIFS_FILE_FS_ATTRIBUTE_INFORMATION 
{
    ULONG FileSystemAttributes;
    LONG MaximumComponentNameLength;
    ULONG FileSystemNameLength;
    WCHAR FileSystemName[DIFS_FILENAME_MAXLENGTH];
} DIFS_FILE_FS_ATTRIBUTE_INFORMATION, *PDIFS_FILE_FS_ATTRIBUTE_INFORMATION;

typedef struct _DIFS_FILE_DISPOSITION_INFORMATION 
{
  BOOLEAN  DeleteFile;
} DIFS_FILE_DISPOSITION_INFORMATION, *PDIFS_FILE_DISPOSITION_INFORMATION;

typedef struct _DIFS_FILE_RENAME_INFORMATION 
{
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} DIFS_FILE_RENAME_INFORMATION, *PDIFS_FILE_RENAME_INFORMATION;

//zwn add 2008-12-30
typedef struct _RECORDSTRUCT
{
	ULONG   dwProcessID;
    ULONG   dwThreadID;
    ULONG   dwIRPpointer;
    ULONG   dwIRP_MJ_Func;
    ULONG   dwIRP_MN_Func;
    ULONG   dwSrcFileObj;
    ULONG   dwSrcFcb;
    ULONG   dwSrcParentDDCb;
    ULONG   dwSrcParentSDCb;
	ULONG   dwDesFileObj;
    ULONG   dwDesFcb;
	ULONG   dwDesParentDDCb;
	ULONG   dwDesParentSDCb;
}RECORDSTRUCT, *PRECORDSTRUCT;

#endif /* _WIN_DIFS_TYPES_H */

