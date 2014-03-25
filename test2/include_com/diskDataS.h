#ifndef _DISKDATAS_H_
#define _DISKDATAS_H_

#include "BaseType.h"
#include "metacmddifs.h"
#include "ShareLock.h"
#include "difsdiskcmd.h"

#pragma pack(push,1)

typedef  CShareLock ERESOURCE;
typedef  CShareLock* PERESOURCE;

typedef COSEvent _KEVENT, KEVENT;
typedef COSEvent *PRKEVENT, *PKEVENT;

#define MAX_PATH          260
#define VOLINFOARRYSIZE 6*1024*1024

typedef     DWORD                      PVOID32_U; //must 4 byte,because of it has written on disk in earlier version

#ifndef _AMD64_              //32 bit driver 
#define     LIST_ENTRY_DEF(Var)         union{LIST_ENTRY       Var;ULONGLONG Var##_placeholder[2];}
#define     PKEVENT_DEF(pVar)           union{PKEVENT          pVar;ULONGLONG pVar##_placeholder;}
#define     PMDL_DEF(pVar)              union{PMDL             pVar;ULONGLONG pVar##_placeholder;}
#define     PIO_APC_ROUTINE_DEF(pVar)   union{PIO_APC_ROUTINE  pVar;ULONGLONG pVar##_placeholder;}
#define     PDIFSDISKIONODE_FORTIME_DEF(pVar)   union{struct DISKIONODE_FORTIME *  pVar;ULONGLONG pVar##_placeholder;}
#define     PDIFSDISKIONODE_DEF(pVar)           union{struct DISKIONODE         *  pVar;ULONGLONG pVar##_placeholder;}
#define     HANDLE_DEF(Var)             union{HANDLE           Var;ULONGLONG pVar##_placeholder;}
#define     PVOID_DEF(pVar)             union{PVOID            pVar;ULONGLONG pVar##_placeholder;}
#define     PULONG_DEF(pVar)            union{PULONG           pVar;ULONGLONG pVar##_placeholder;}
#define     PDIFS_MULTI_DISK_DEF(pVar)  union{struct _MULTI_DISK * pVar;ULONGLONG pVar##_placeholder;}
#define     ULONG_PTR_DEF(pVar)         union{ULONG_PTR        pVar;ULONGLONG pVar##_placeholder;}
#define     _IO_STATUS_BLOCK_DEF(Var)   struct _IO_STATUS_BLOCK_MY  Var

#else                           //64 bit driver
#define     LIST_ENTRY_DEF(Var)         LIST_ENTRY       Var
#define     PKEVENT_DEF(pVar)           PKEVENT          pVar
#define     PMDL_DEF(pVar)              PMDL             pVar
#define     PIO_APC_ROUTINE_DEF(pVar)   PIO_APC_ROUTINE  pVar
#define     PDIFSDISKIONODE_FORTIME_DEF(pVar)   struct DISKIONODE_FORTIME *  pVar
#define     PDIFSDISKIONODE_DEF(pVar)           struct DISKIONODE         *  pVar
#define     HANDLE_DEF(Var)             HANDLE           Var
#define     PVOID_DEF(pVar)             PVOID            pVar
#define     PULONG_DEF(pVar)            PULONG           pVar
#define     PDIFS_MULTI_DISK_DEF(pVar)  struct _MULTI_DISK * pVar
#define     ULONG_PTR_DEF(pVar)         ULONG_PTR        pVar
#define     _IO_STATUS_BLOCK_DEF(Var)   struct _IO_STATUS_BLOCK  Var
#endif


typedef void *HANDLE  ;

typedef ULONG *PULONG  ;   //
typedef u32 DISTATUS;
typedef byte   BYTE  ;
typedef BYTE   *PBYTE  ;

typedef struct _IO_STATUS_BLOCK_MY
{
	union 
	{
        LONG   Status;
		DISTATUS Status_In;
		PVOID_DEF(   Pointer);
	};

	ULONG_PTR_DEF(  Information);
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef enum _MEDIA_TYPE {
	Unknown,                // Format is unknown
	F5_1Pt2_512,            // 5.25", 1.2MB,  512 bytes/sector
	F3_1Pt44_512,           // 3.5",  1.44MB, 512 bytes/sector
	F3_2Pt88_512,           // 3.5",  2.88MB, 512 bytes/sector
	F3_20Pt8_512,           // 3.5",  20.8MB, 512 bytes/sector
	F3_720_512,             // 3.5",  720KB,  512 bytes/sector
	F5_360_512,             // 5.25", 360KB,  512 bytes/sector
	F5_320_512,             // 5.25", 320KB,  512 bytes/sector
	F5_320_1024,            // 5.25", 320KB,  1024 bytes/sector
	F5_180_512,             // 5.25", 180KB,  512 bytes/sector
	F5_160_512,             // 5.25", 160KB,  512 bytes/sector
	RemovableMedia,         // Removable media other than floppy
	FixedMedia,             // Fixed hard disk media
	F3_120M_512,            // 3.5", 120M Floppy
	F3_640_512,             // 3.5" ,  640KB,  512 bytes/sector
	F5_640_512,             // 5.25",  640KB,  512 bytes/sector
	F5_720_512,             // 5.25",  720KB,  512 bytes/sector
	F3_1Pt2_512,            // 3.5" ,  1.2Mb,  512 bytes/sector
	F3_1Pt23_1024,          // 3.5" ,  1.23Mb, 1024 bytes/sector
	F5_1Pt23_1024,          // 5.25",  1.23MB, 1024 bytes/sector
	F3_128Mb_512,           // 3.5" MO 128Mb   512 bytes/sector
	F3_230Mb_512,           // 3.5" MO 230Mb   512 bytes/sector
	F8_256_128,             // 8",     256KB,  128 bytes/sector
	F3_200Mb_512,           // 3.5",   200M Floppy (HiFD)
	F3_240M_512,            // 3.5",   240Mb Floppy (HiFD)
	F3_32M_512              // 3.5",   32Mb Floppy
} MEDIA_TYPE, *PMEDIA_TYPE;

typedef struct _DISK_EXTENT {
	DWORD             DiskNumber[3];
	LARGE_INTEGER     StartingOffset;
	LARGE_INTEGER     ExtentLength;
} DISK_EXTENT, *PDISK_EXTENT;

typedef struct _VOLUME_DISK_EXTENTS {
	DWORD         NumberOfDiskExtents;
	DISK_EXTENT Extents[1];
} VOLUME_DISK_EXTENTS, *PVOLUME_DISK_EXTENTS;



typedef struct _DISK_GEOMETRY {
	LARGE_INTEGER   Cylinders;
	MEDIA_TYPE MediaType;
	DWORD   TracksPerCylinder;
	DWORD   SectorsPerTrack;
	DWORD   BytesPerSector;
} DISK_GEOMETRY, *PDISK_GEOMETRY;

#define ZeroMemory(x,y) memset(x,0,y)

#ifndef DISKBEGINRESERVEDSIZE  
#define DISKBEGINRESERVEDSIZE (20*1024*1024)
#endif
#ifndef DISKENDRESERVEDSIZE  
#define DISKENDRESERVEDSIZE (20*1024*1024)
#endif

#define DISKSIZEOFFSET (18*1024)
#define VOLINFOOFFSET (40*1024)

#define ABSOLUTE(wait) (wait)

#define RELATIVE(wait) (-(wait))

#define NANOSECONDS(nanos)    (((signed __int64)(nanos))   /  100L)

#define MICROSECONDS(micros)  (((signed __int64)(micros))  *  NANOSECONDS(1000L))

#define MILLISECONDS(milli)   (((signed __int64)(milli))   *  MICROSECONDS(1000L))

#define SECONDS(seconds)      (((signed __int64)(seconds)) *  MILLISECONDS(1000L))

#define DELAY_ONE_MICROSECOND    (RELATIVE(MICROSECONDS(1)))
#define DELAY_ONE_MILLISECOND    (RELATIVE(MILLISECONDS(1)))
#define DELAY_ONE_SECOND         (RELATIVE(SECONDS(1)))

#define ONE_MICROSECOND    (ABSOLUTE(MICROSECONDS(1)))
#define ONE_MILLISECOND    (ABSOLUTE(MILLISECONDS(1)))
#define ONE_SECOND         (ABSOLUTE(SECONDS(1)))

typedef enum _DATAMODE                                                    
{
	LOCMODE,                                                              
	LANMODE,                                                              
	MIXMODE                                                               
}DATAMODE; 
typedef struct _DISKSPEEDNODE                                             
{
	ULONGLONG                              uReadSpeed;                   
	ULONGLONG                              uWriteSpeed;                  
	ULONGLONG                              uAllSpeed;                    

	LARGE_INTEGER                        uReadTime;                    
	LARGE_INTEGER                        uWriteTime;                   
	LARGE_INTEGER                        uAllTime;                     

}DISKSPEEDNODE,*PDISKSPEEDNODE; 

typedef struct _GETDISKSPEEDNODE
{
	GUID 								  disk;
	DISKSPEEDNODE 				diskspeednode;

}GETDISKSPEEDNODE,*PGETDISKSPEEDNODE;

typedef struct _SPEEDARY                                                  
{
	ULONG                                getindex;                     
	ULONG                                readindex;                    
	ULONG                                writeindex;                   
	ULONG                                allindex;                     
	ULONG                                entrynumber;                  
	DWORD                              reservediskspeedary;                          

}SPEEDARY,*PSPEEDARY;
typedef struct _IOINONESECOND                                             
{
	ULONGLONG                              uIototalmb;                   
	ULONGLONG                              uCurSpeed;                    
	ULONGLONG                              uLimitSpeed;                  
	LARGE_INTEGER                        lTimeBegin;                   

}IOINONESECOND,*PIOINONESECOND;   
typedef struct _DISKSPEED                                                 
{
	SPEEDARY                           speedary;                          
	DWORD                              reserveplock;                                 
	LARGE_INTEGER                        SpeedTimeout;                 
	IOINONESECOND                      Read;                              
	IOINONESECOND                      Write;                             
	IOINONESECOND                      All;              

}DISKSPEED,*PDISKSPEED; 



typedef enum _DISKTYPE                                        
{
	FILE_TYPE,                                                
	PARTIOTION_TYPE,                                          
	DISK_TYPE                                                 

}DiskType; 

typedef struct _VIRTUALDISK                                                 
{
	LONGLONG                             DiskSize;                            
	GUID                                 DiskGuid;                       
	GUID                                 VolGuid;                        
	UCHAR                                diskAlias[MAX_PATH];            
	DISK_GEOMETRY                      diskgeometry;                        
	DWORD                                DiskNumber;                     
	DWORD                                DiskNoInVol;                    
	UCHAR                                DiskName[MAX_PATH];             
	ULONG                                uDiskNameLength;                
	DiskType                           disktype;                            
	ULONG                                diskMetaDataRate;               
}VIRTUALDISK,*PVIRTUALDISK;

typedef struct   _MULTI_DISK
{
	volatile ULONG                       uDisConnectDiskNum;          
	volatile ULONG                       uDiskTotalNumber;            
	VIRTUALDISK                        pVirtualdiskAry[1];               

}DIFS_MULTI_DISK,*PDIFS_MULTI_DISK;



typedef struct _VOLIO_TO_DISKIO_ERY                                         
{
	ULONG                                uDiskIdx;                       
	PVOID                                Buffer;                              
	ULONG                                Length;                         
	LARGE_INTEGER                        ByteOffset;                     
	DISTATUS                           status;                              

}VOLIO_TO_DISKIO_ERY,*PVOLIO_TO_DISKIO_ERY;

typedef struct _VOLIO_TO_DISKIO                                             
{
	ULONG                                uCurIoNumber;                   
	PVOLIO_TO_DISKIO_ERY               pvolio_to_diskio_ery;                

}VOLIO_TO_DISKIO,*PVOLIO_TO_DISKIO;


typedef enum _DISKIOOPTYPE                                                  
{
	READFILE,
	WRITEFILE,
	FLUSHDATAS,
	CACHEADDNOTIFYINFO,
	CACHEREMOVENOTIFYINFO
}DISKIOOPTYPE;

//typedef struct _CACHEUESEDINFO
//{
//}CACHEUESEDINFO,P_CACHEUESEDINFO;
//////////
//
//typedef struct _FSDTODIFSDISKFLUSHDATA
//{
//    void * pToDifsDiskRlist;
//}FsdToDifsDiskFlushData ,*PFsdToDifsDiskFlushData;
//
//typedef struct _FSDTODIFSDISK1STREADFILE
//{
//    void * pToDifsDiskRlist;
//}FsdToDifsDisk1stReadFile ,*PFsdToDifsDisk1stReadFile;
//
//typedef struct _FsdReadAhead
//{
//    unsigned long        uClusterSize;
//    void                 *pRunlist;
//    PLARGE_INTEGER       FileOffset;
//    ULONG                Length;
//}FsdReadAhead, *PFsdReadAhead;
//
//typedef struct _DIRC_IOCONTEXT
//{
//    u64 HandleIdentifier;
//    u64 fileId;
//    u64 FileOffset;
//    u32 Filelength;
//}DIRC_IOCONTEXT,*PDIRC_IOCONTEXT;
//
//typedef struct _FSD2DISK_DATAIO
//{
//    DIRC_IOCONTEXT dirc_iocontext;
//    u64 diskoffset;
//    u32 disklength;
//    PVOID pbuffer;
//}FSD2DISK_DATAIO,*PFSD2DISK_DATAIO;
//
//typedef struct _FSD2DISK_RUNLIST
//{
//    u64 fileId;
//    Run_list runlist;
//}FSD2DISK_RUNLIST,*PFSD2DISK_RUNLIST;
//
//typedef struct _FSD2DISKCLOSEFILE
//{
//    u64 fileId;
//}FSD2DISKCLOSEFILE,*PFSD2DISKCLOSEFILE;
//
//typedef struct _FSD2DISKOPENFILE
//{
//    u64 fileId;
//}FSD2DISKOPENFILE,*PFSD2DISKOPENFILE;
//
//typedef struct _DIFSDISKCMD
//{
//    DWORD   Command;
//    union
//    {
//        FsdToDifsDiskFlushData   fsdtodifsdiskflushdata;
//        FsdToDifsDisk1stReadFile fsdtodifsdisk1streadfile;
//        FsdReadAhead             fsdReadAhead;
//        #ifdef _t1000hdl
//        FSD2DISK_DATAIO          fsdio;
//        FSD2DISK_RUNLIST         fsd2disk_runlist;
//        FSD2DISKCLOSEFILE        fsd2diskclosefile;
//        FSD2DISKOPENFILE         fsd2diskopenfile;
//        #endif
//    }Data;
//}DifsDiskCmd,*pDifsDiskCmd;


///
typedef struct   DISKIO
{
	DWORD                              CRC1;                                   //
	DWORD                              CRC2;                                   //
	GUID                               ComputerId;                             //
	BOOLEAN                            bCheckMasData_CRC;                      //
	GUID                               VolGuid;                                //
	GUID                               MasHandle;                              //
	HANDLE_DEF(                        Event);                                  //

	PVOID_DEF(             ApcRoutine);                             //
	PVOID_DEF(                         ApcContext);                             //
	_IO_STATUS_BLOCK_DEF(              IoStatusBlock);                          //
	PVOID_DEF(                         Buffer);                                 //
	ULONG                              Length;                                 //
	LARGE_INTEGER                      ByteOffset;                             //
	PULONG_DEF(                        Key);                                    //
	PVOID_DEF(                          pMdl);                                   //
	ULONG                              uIoSerialNumber;                        //
	ULONG                              uIoPacketNumber;	
#ifdef _t1000hdl
    DIRC_IOCONTEXT dirc_iocontext;
    ULONG nElementNum;
    ULONG PCID;
    BOOLEAN isNetIo;
    ULONG rev1;
    ULONG getRunList;
#endif
}DIFSDISKIO,*PDIFSDISKIO;

typedef struct   DISKIONODE                                             
{
	PDIFSDISKIONODE_FORTIME_DEF(       pionodetime);                           //
	PDIFSDISKIONODE_DEF(               curnode);                               //
	LIST_ENTRY_DEF(                    entry);                               //
	BOOLEAN                            bInList;                                //
	DISKIOOPTYPE                       optype;                                 //
	PVOID_DEF(                       pcompleteevent);                         //
	GUID                               MasHandle;                              //
	GUID                               VolGuid;                                //
	DIFSDISKIO                         difsdiskio;                      

}DIFSDISKIONODE,*PDIFSDISKIONODE;

typedef struct   DISKIONODE_APP                                         
{
	GUID                                 VolGuid;                       
	LARGE_INTEGER                        MasNodeTimeOut;                
	LARGE_INTEGER                        AppGetNodeTime;                
	DIFSDISKIONODE                     difsdiskionode;                     

}DIFSDISKIONODE_APP,*PDIFSDISKIONODE_APP;

typedef struct   DISKIONODE_FORTIME                                     
{
	LARGE_INTEGER                        MasNodeTimeOut;                
	LARGE_INTEGER                        AppGetNodeTime;                
	PDIFSDISKIONODE                    pdifsdiskionode;                   

}DIFSDISKIONODE_FORTIME,*PDIFSDISKIONODE_FORTIME;



typedef struct _OPEN_FILE_INFORMATION 
{
	DWORD  NextVolInfo;//PUCHAR_DEF(NextVolInfo) ;	//PVOID                                NextVolInfo;                        
	LARGE_INTEGER                        VolumeSize;                    
	DWORD                                VolumeNumber;                  
	GUID                                 VolGuid;                       
	WCHAR                                DiFsDiskDevice[100];           
	UCHAR                                VolAlias[MAX_PATH];            
	BOOLEAN                              bReadOnly;                     
	BOOLEAN                              bIsStrip;                     
	WCHAR                                SymbolicLink_LetterShort[7];   
	WCHAR                                SymbolicLink_Letter[13];       
	VOLUME_DISK_EXTENTS                  volumediskextents;                  

	struct                                                                 
	{
		ULONG                            uSectorSize;                   
		ULONG                            uHashListNum;                  
		ULONG                            uCacheUnitSize;                
		ULONG                            uTotalCacheUnitNum;            
		ULONG                            WriteDiskThreadNumber;         
		ULONG                            uBufferSize;                   

		ULONG                            QuadPart;            
		int                            ReadAheadInfArryNum;                   
		int                            SeriesNum;                             
		int                            bLayzeWrite;                           

		GUID                             ComputerId;                     
		BOOLEAN                          bCheckMasData_CRC;              

		BOOLEAN                          bUseLanNow;                     
		DATAMODE                       DataMode;                            

		DISKSPEED                      diskspeed;                           

		LARGE_INTEGER                    MasNodeTimeOut;                 

	}volconfig;                                                             

	DIFS_BOOT_SECTOR_SHORT             diskFormatData;                      

	DIFS_MULTI_DISK                    difs_multi_disk;                     

}OPEN_FILE_INFORMATION, *POPEN_FILE_INFORMATION;

typedef struct _DEVINFO
{
	CCritSec *qosRes;
	CShareLock *qosEvt;

    POPEN_FILE_INFORMATION             pVolumeInfo;

}DEVINFO,*PDEVINFO;

typedef struct _VIRTUALVOLUME_ALL
{
    DWORD                              ActiveNumberOfDevices;                  //
    DWORD                              MappedNumberofDevices;                  //
    DWORD                              NumberOfDevices;                        //
    POPEN_FILE_INFORMATION             pVolumeAll;

}VIRTUALVOLUME_ALL,*PVIRTUALVOLUME_ALL;

typedef struct _STRIPNODE                     
{
    u32                 	diskTot;     
    u32                 	diskNum;     
    LONGLONG				diskSize;      
    LONGLONG				stripBlockSize;      
}STRIPNODE,*P_STRIPNODE;

typedef struct _STRIPDATA                     
{
    volatile u32                 	stripNum;     
    volatile u32                 	stripSize;     
    STRIPNODE						stripArry[1];      
}STRIPDATA,*P_STRIPDATA;

typedef struct _STRIPCONFIG
{
	POPEN_FILE_INFORMATION  open_file_information;
	P_STRIPDATA				stripData;
}STRIPCONFIG, *PSTRIPCONFIG;

#pragma pack(pop)

#endif
