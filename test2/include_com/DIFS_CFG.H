#ifndef _DIFS_CFG_H_
#define _DIFS_CFG_H_


#define DIFS_DIRDIR_INITSIZE                0x40000  //128k BYTES  FOR POSSIBLE 140 SUBDIR
#define DIFS_DIRFILE_INITSIZE               0x80000  //512k BYTES  FOR POSSIBLE 570 FILE
#define DIFS_USERFILE_INITSIZE              0x6000000  //96M bytes

#define DIFS_USRFILE_INITSIZE                   0x100000      
#define DIFS_DIR_MARGINSPACE                    0x40000      //for every dir ,keep 128k margin

#define DIFS_DIR_INDEX_BLOCK_SIZE               8192        //in bytes  , 8k
#define DIFS_DIR_INDEX_ENTRY_NUMBER             10          //B+ tree rank
#define DIFS_DIR_BALANCE_NUMBER                 6           //B+ tree node's number that need to banlance 
#define DIFS_DIR_MERGE_NUMBER                   3           //B+ tree node's number that need to merge

#define DIFS_WRITE_BYTES_MAX                    (64*1024)

#define DIFS_FILENAME_MAXLENGTH                 256         //in wchar , 512bytes
    
#define DIFS_SPACE_IDX_RECORD_SIZE              4096        //BYTES ,MUST BE intergal multiple of sector size

#define DIFS_MAX_DATAVOL_NUM                    32 

#define DIFS_MAX_SYSVOL_NUM                     1

#define CHKDSK_USN                              0xabcd      //usn that is used to check if there are bad sectors.
#define USN_SIZE                                2

#define DIR_CACHE_IN_CLIENT                     (1024*16)

//INFO: assit parameters
#define DIFS_CLUSTER_SIZE_IN_BTYE               4096

#define DIFS_MFT_MARGINSPACE                    (128*1024*1024) 
#define DIFS_DISKALLOC_MARGINSPACE                  (128 * 1024*1024) 

//  Define stack overflow read threshhold.  For the x86 we'll use a smaller
//  threshold than for a risc platform.
//
//  Empirically, the limit is a result of the (large) amount of stack
//  neccesary to throw an exception.
#ifdef _M_IX86
    #define OVERFLOW_READ_THRESHHOLD         (0xE00)
#else
    #define OVERFLOW_READ_THRESHHOLD         (0x1000)
#endif // defined(_M_IX86)

#define BMP_CACHE_NUM                           0x5 //5 item for bmp cache

#define SPACENDOE_ENTRYOFFSET                   1024 //BYTES
#define SPACENDOE_TYPE12_ENTRYDESCOFFSET        2560 //BYTES
#define SPACENDOE_TYPE1_ENTRY_CLUSTERNUM            ((u32)(9 * 1024 * 1024 ))    //in cluster , for 4k cluster is 32G
#define SPACENDOE_TYPE2_ENTRY_CLUSTERNUM            ((u32)(24 * 1024  )  )    //in cluster , for 4k cluster is 96M
#define SPACENDOE_TYPE3_MAX_ENTRYNUM            0x6000

#define SCM2SSMFILEMAXNUM                       500

//INFO: Format
#define SMFT_SIZE                               0.01
#define SMFT_in_VOL                             0.08
#define SLOG_in_VOL                             0.02
#define DIFSMFTSIZE                             2560                   //   2560 Ди= 10M
#define DIFSLOGSIZE                             25600                  //   25600 Ди= 100M
#define DIFS_MAJOR_VER                          1
#define DIFS_MINOR_VER                          0
#define MFTSTRRTLCN                             16
#define LogListPostion                          8
#define BADCLUSLEN                              2
#define DIFSCLUSTERSIZE                         4096

#define FSD_DRIVER_NAME                         L"DifsFsd"
#define FSD_DEVICE_NAME                         L"\\??\\DifsFsd"
#define FSD_DOS_DEVICE_NAME                     L"\\DosDevices\\DifsFsd"
#define FSD_DRIVER_NAME_A                       "DifsFsd"

#define FSD_REGISTRY_PATH                       L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\DifsFsd"
#define FSD_VALIDDIFSVOL_KEY                    L"\\ValidDifsVol"
#define FSD_SERIAL_NUMBER_SUB_KEY               L"serial_number"


#define SCMD_NAME                               L"ScmDriver"
#define SCMD_NAME_A                             "ScmDriver"
#define SCMD_DEVICE_NAME                        L"\\??\\ScmDriver"

#define SSM_SVR_NAME                            L"SsmSvr.exe"
#define SCM_SVR_NAME                            L"ScmSvr.exe"

// device name string definition
#define FSLD_DEVICE_NAME                        L"\\??\\Fsld"

//for debug
#define DEBUG_MSG_HEAD_MAX_LEN                  0x100
#define DEBUG_MSG_MAX_LEN                       0x512
#define DEBUG_FILE_NAME_MAX_LEN                 256
#define DEBUG_DUMP_BYTESNUM_PER_LINE            16

#define DI_DBG_SERIAL_NUMBER                    0x11111111

//the length of resident memory that is for runlist reading and writing
#define RUNLIST_RW_RESIDENT_MEM_LEN             4096

#define DIFS_OPLIST_INIT_LEN                    10

#define     DIFS_FILE_SEARCH_ITEMNUM            20 

//  file access mode : FileAccessMode
#define         FileAccess_Read_Radom       0x01
#define         FileAccess_Write_Radom      0x02
#define         FileAccess_Write_Append     0x03

//  uFileCacheMode   
#define     DIFS_FILE_READ_CACHE            0x01
#define     DIFS_FILE_WRITE_CACHE           0x02
#define     DIFS_FILE_NO_CACHE              0x03
#define		DIFS_FILE_DELETE_CACHE			0x04
#define		DIFS_FILE_SIZECHANGE			0x05

#define		DIFS_DIR_ADDCHANGE				0x06
#define		DIFS_DIR_REMOVECHANGE			0x07

#define     DIFS_NO_READ                    'ERON'

#endif
