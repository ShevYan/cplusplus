#pragma once
#include "FiErr.h"

const u8 M_Fics                                         =   1; //??
const u8 M_Btree                                        =   2;
const u8 M_Rpc                                          =   3;
const u8 M_Disk                                         =   4;
const u8 M_NetSyn                                       =   5;
const u8 M_Journal                                      =   6;
const u8 M_Other                                        =   7;
const u8 M_Net                                          =   8;
const u8 M_Scm											= 	9;
const u8 M_Osd                                       	=   10;
const u8 M_Bitmap                                   = 10;


extern const u32 MFics_ThrdPoolStop;
extern const u32 MFics_ThrdPoolExcdMaxTask;  
extern const u32 MFics_ThrdPoolExcdEntry;
extern const u32 MFics_ThrdPoolTaskClr;
extern const u32 MDisk_fseekFailed;
extern const u32 MDisk_fwriteFailed;
extern const u32 MDisk_freadFailed;
extern const u32 MDisk_fileNotExist;
extern const u32 MDisk_endOfFile;
extern const u32 MNetSyn_ZKFailed;
extern const u32 MOsd_MainNodeIsOK;
extern const u32 MOsd_LocNodeIsNotOK;
extern const u32 MOsd_RemoteNodeIsNotOK;
extern const u32 MJournal_headErr;
extern const u32 MJournal_endoffile;
extern const u32 MJournal_stop;
extern const u32 MJournal_stopping;
extern const u32 MOther_fileError;
extern const u32 MNet_SendFailed;

/// btree error start
extern const u32 MBtree_fileHasExisted;
extern const u32 MBtree_notExist;
extern const u32 MBtree_noMorefile;
extern const u32 MBitmap_readdiskerror; 
/// btree error end

// scm error
extern const u32 Scm_RouterSendFailed;

