#pragma once
#include "difstypes.h"

//#define _t1000hdl

#define MAKE_METACMD(type,code) \
((DWORD) (((unsigned long)(type)<<24) | ((unsigned long)(code))) )

#define   MCMD_CLASS_DIFS           0

#define     DI_IC_FsdToDisk                        (ULONG)CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x0f, METHOD_BUFFERED,  FILE_READ_ACCESS )
#define     MCMD_REQ_FSD2DISK_FLUSH                 MAKE_METACMD(MCMD_CLASS_DIFS,950)
#define     MCMD_REQ_FSD2DISK_1STREADFILE           MAKE_METACMD(MCMD_CLASS_DIFS,951)
#define     MCMD_REQ_FSD2DISK_READ_AHEAD            MAKE_METACMD(MCMD_CLASS_DIFS,952)
#define     MCMD_REQ_FSD2DISK_READDISK              MAKE_METACMD(MCMD_CLASS_DIFS,953)
#define     MCMD_REQ_FSD2DISK_WRITEDISK             MAKE_METACMD(MCMD_CLASS_DIFS,954)
#define     MCMD_REQ_FSD2DISK_SETRUNLIST            MAKE_METACMD(MCMD_CLASS_DIFS,956)
#define     MCMD_REQ_FSD2DISK_CLOSEFILE             MAKE_METACMD(MCMD_CLASS_DIFS,957)
#define     MCMD_REQ_FSD2DISK_OPENFILE              MAKE_METACMD(MCMD_CLASS_DIFS,958)

//typedef unsigned long DWORD;

typedef struct _FSDTODIFSDISKFLUSHDATA
{
    void * pToDifsDiskRlist;
}FsdToDifsDiskFlushData ,*PFsdToDifsDiskFlushData;

typedef struct _FSDTODIFSDISK1STREADFILE
{
    void * pToDifsDiskRlist;
}FsdToDifsDisk1stReadFile ,*PFsdToDifsDisk1stReadFile;

typedef struct _FsdReadAhead
{
    unsigned long        uClusterSize;
    void                 *pRunlist;
    PLARGE_INTEGER       FileOffset;
    ULONG                Length;
}FsdReadAhead, *PFsdReadAhead;

#ifndef _difsdisk1000
typedef struct _DIRC_IOCONTEXT
{
    u64 HandleIdentifier;
    u64 fileId;
    u64 FileOffset;
    u32 Filelength;
}DIRC_IOCONTEXT,*PDIRC_IOCONTEXT;


typedef struct _FSD2DISK_DATAIO
{
    DIRC_IOCONTEXT dirc_iocontext;
    u64 diskoffset;
    u32 disklength;
    PVOID pbuffer;
}FSD2DISK_DATAIO,*PFSD2DISK_DATAIO;

typedef struct _FSD2DISK_RUNLIST
{
    u64 fileId;
    Run_list runlist;
}FSD2DISK_RUNLIST,*PFSD2DISK_RUNLIST;

typedef struct _FSD2DISKCLOSEFILE
{
    u64 fileId;
}FSD2DISKCLOSEFILE,*PFSD2DISKCLOSEFILE;

typedef struct _FSD2DISKOPENFILE
{
    u64 fileId;
}FSD2DISKOPENFILE,*PFSD2DISKOPENFILE;
#endif

typedef struct _DIFSDISKCMD
{
    DWORD   Command;
    union
    {
        FsdToDifsDiskFlushData   fsdtodifsdiskflushdata;  
        FsdToDifsDisk1stReadFile fsdtodifsdisk1streadfile;
        FsdReadAhead             fsdReadAhead;
        #ifdef _t1000hdl
        FSD2DISK_DATAIO          fsdio;
        FSD2DISK_RUNLIST         fsd2disk_runlist;
        FSD2DISKCLOSEFILE        fsd2diskclosefile;
        FSD2DISKOPENFILE         fsd2diskopenfile;
        #endif
    }Data;
}DifsDiskCmd,*pDifsDiskCmd;


