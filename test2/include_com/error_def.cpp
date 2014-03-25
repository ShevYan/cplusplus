#include "error_def.h"


static FiFileNameTableUnit gFileTable[] = {
        "FiThreads.cpp",                        1,
        "ficsLocIF.cpp",                        2,
        "ficsNetSyn.cpp",                       3,
        "jnstore.cpp",                          4,
        "fiOsd.h",                              5,
		 "MetaCommandRouter.cpp", 6
};

FiErrFileNameRegitor errFileNameReg(vector<FiFileNameTableUnit>(gFileTable, gFileTable+sizeof(gFileTable)/sizeof(gFileTable[0])) );

#if defined(_WIN64) || defined(_WIN32)
const u32 MFics_ThrdPoolStop                        = FiErr::MakeBaseCode_Erro(M_Fics,          1,  STATUS_IO_TIMEOUT,                  "MFics_ThrdPoolStop");
const u32 MFics_ThrdPoolExcdMaxTask                 = FiErr::MakeBaseCode_Erro(M_Fics,          2,  STATUS_IO_TIMEOUT,                  "MFics_ThrdPoolExcdMaxTask");
const u32 MFics_ThrdPoolExcdEntry                   = FiErr::MakeBaseCode_Erro(M_Fics,          3,  STATUS_IO_TIMEOUT,                  "MFics_ThrdPoolExcdEntry");   
const u32 MFics_ThrdPoolTaskClr                     = FiErr::MakeBaseCode_Erro(M_Fics,          4,  STATUS_IO_TIMEOUT,                  "MFics_ThrdPoolTaskClr");  
   
const u32 MDisk_fseekFailed                         = FiErr::MakeBaseCode_Erro(M_Disk,          1,  STATUS_INVALID_PARAMETER,           "MDisk_fseekFailed");     
const u32 MDisk_fwriteFailed                        = FiErr::MakeBaseCode_Erro(M_Disk,          2,  STATUS_VHD_METADATA_WRITE_FAILURE,  "MDisk_fwriteFailed");     
const u32 MDisk_freadFailed                         = FiErr::MakeBaseCode_Erro(M_Disk,          3,  STATUS_VHD_METADATA_READ_FAILURE,   "MDisk_freadFailed");     
const u32 MDisk_fileNotExist                        = FiErr::MakeBaseCode_Erro(M_Disk,          4,  STATUS_VHD_METADATA_READ_FAILURE,   "MDisk_fileNotExist");     
const u32 MDisk_endOfFile                           = FiErr::MakeBaseCode_Erro(M_Disk,          5,  STATUS_VHD_METADATA_READ_FAILURE,   "MDisk_endOfFile");  
   
const u32 MNetSyn_ZKFailed                          = FiErr::MakeBaseCode_Erro(M_NetSyn,        1,  STATUS_INTERNAL_ERROR,              "MNetSyn_ZKFailed");     

const u32 MOsd_MainNodeIsOK                         = FiErr::MakeBaseCode_Erro(M_Osd,        	1,  STATUS_INTERNAL_ERROR,              "MOsd_MainNodeIsOK");     
const u32 MOsd_LocNodeIsNotOK                       = FiErr::MakeBaseCode_Erro(M_Osd,        	2,  STATUS_INTERNAL_ERROR,              "MOsd_LocNodeIsNotOK");     
const u32 MOsd_RemoteNodeIsNotOK                       = FiErr::MakeBaseCode_Erro(M_Osd,        3,  STATUS_INTERNAL_ERROR,              "MOsd_RemoteNodeIsNotOK");     

const u32 MJournal_headErr                          = FiErr::MakeBaseCode_Erro(M_Journal,       1,  STATUS_ACCESS_DENIED,               "MJournal_headErr");
const u32 MJournal_endoffile                        = FiErr::MakeBaseCode_Erro(M_Journal,       2,  STATUS_ACCESS_DENIED,               "MJournal_endoffile");
const u32 MJournal_stop                             = FiErr::MakeBaseCode_Erro(M_Journal,       3,  STATUS_ACCESS_DENIED,               "MJournal_stop");
const u32 MJournal_stopping                         = FiErr::MakeBaseCode_Erro(M_Journal,       4,  STATUS_ACCESS_DENIED,               "MJournal_stopping");

const u32 MOther_fileError                          = FiErr::MakeBaseCode_Erro(M_Other,         1,  STATUS_ACCESS_DENIED,               "MOther_fileError");

const u32 MNet_SendFailed                           = FiErr::MakeBaseCode_Erro(M_Net,           1,  STATUS_INTERNAL_ERROR,              "MNet_SendFailed");     

/// btree error
const u32 MBtree_notExist                           = FiErr::MakeBaseCode_Erro(M_Btree,         1,  ESTATUS_INTERNAL_ERRO,                              "MBtree_notExist");
const u32 MBtree_fileHasExisted                         = FiErr::MakeBaseCode_Erro(M_Btree,         3, EIO,"MBtree_fileHasExisted");
const u32 MBtree_noMorefile                         = FiErr::MakeBaseCode_Suc(M_Btree,         2,  EIO,                                "MBtree_noMorefile");

const u32 MBitmap_readdiskerror                     = FiErr::MakeBaseCode_Suc(M_Bitmap,         1, EIO,"MBitmap_readddiskerror");

/// scm error
const u32 Scm_RouterSendFailed						= FiErr::MakeBaseCode_Erro(M_Scm,			6,	RPC_DI_SENDERR,			"Scm_RouterSendFailed");

#elif defined(__linux)
const u32 MFics_ThrdPoolStop                        = FiErr::MakeBaseCode_Erro(M_Fics,          1,  EBUSY,                              "MFics_ThrdPoolStop");
const u32 MFics_ThrdPoolExcdMaxTask                 = FiErr::MakeBaseCode_Erro(M_Fics,          2,  EBUSY,                              "MFics_ThrdPoolExcdMaxTask"); 
const u32 MFics_ThrdPoolExcdEntry                   = FiErr::MakeBaseCode_Erro(M_Fics,          3,  EBUSY,                              "MFics_ThrdPoolExcdEntry");   
const u32 MFics_ThrdPoolTaskClr                     = FiErr::MakeBaseCode_Erro(M_Fics,          4,  EBUSY,                              "MFics_ThrdPoolTaskClr");    

const u32 MDisk_fseekFailed                         = FiErr::MakeBaseCode_Erro(M_Disk,          1,  EIO,                                                "MDisk_fseekFailed");     
const u32 MDisk_fwriteFailed                        = FiErr::MakeBaseCode_Erro(M_Disk,          2,  EIO,                                                "MDisk_fwriteFailed");     
const u32 MDisk_freadFailed                         = FiErr::MakeBaseCode_Erro(M_Disk,          3,  EIO,                                                "MDisk_freadFailed");     
const u32 MDisk_fileNotExist                        = FiErr::MakeBaseCode_Erro(M_Disk,          4,  EIO,                                                "MDisk_fileNotExist"); 
const u32 MDisk_endOfFile                           = FiErr::MakeBaseCode_Erro(M_Disk,          5,  EIO,                                                "MDisk_endOfFile"); 
    
const u32 MNetSyn_ZKFailed                          = FiErr::MakeBaseCode_Erro(M_NetSyn,        1,  EIO,                                                "MNetSyn_ZKFailed");     

const u32 MOsd_MainNodeIsOK                         = FiErr::MakeBaseCode_Erro(M_Osd,        	1,  EIO,              					"MOsd_MainNodeIsOK");     
const u32 MOsd_LocNodeIsNotOK                       = FiErr::MakeBaseCode_Erro(M_Osd,        	2,  EIO,              					"MOsd_LocNodeIsNotOK");     
const u32 MOsd_RemoteNodeIsNotOK                    = FiErr::MakeBaseCode_Erro(M_Osd,        	3,  EIO,              					"MOsd_RemoteNodeIsNotOK");     

const u32 MJournal_headErr                          = FiErr::MakeBaseCode_Erro(M_Journal,       1,  EIO,                                "MJournal_headErr");
const u32 MJournal_endoffile                        = FiErr::MakeBaseCode_Erro(M_Journal,       2,  EIO,                                "MJournal_endoffile");
const u32 MJournal_stop                             = FiErr::MakeBaseCode_Erro(M_Journal,       3,  EIO,                                "MJournal_stop");
const u32 MJournal_stopping                         = FiErr::MakeBaseCode_Erro(M_Journal,       4,  EIO,                                "MJournal_stopping");

const u32 MOther_fileError                          = FiErr::MakeBaseCode_Erro(M_Other,         1,  EIO,                                "MOther_fileError");

const u32 MNet_SendFailed                           = FiErr::MakeBaseCode_Erro(M_Net,           1,  EIO,                                                "MNet_SendFailed");     

/// btree error
const u32 MBtree_notExist                           = FiErr::MakeBaseCode_Erro(M_Btree,         1,  EIO,                                "MBtree_notExist");
const u32 MBtree_fileHasExisted                         = FiErr::MakeBaseCode_Erro(M_Btree,         3, EIO,"MBtree_fileHasExisted");

const u32 MBtree_noMorefile                         = FiErr::MakeBaseCode_Suc(M_Btree,         2,  EIO,                                "MBtree_noMorefile");

const u32 MBitmap_readdiskerror                     = FiErr::MakeBaseCode_Suc(M_Bitmap,         1, EIO,"MBitmap_readddiskerror");

/// scm error
const u32 Scm_RouterSendFailed						= FiErr::MakeBaseCode_Erro(M_Scm,			6,	EIO,			"Scm_RouterSendFailed");

#elif defined(__APPLE__)
const u32 MFics_ThrdPoolStop                        = FiErr::MakeBaseCode_Erro(M_Fics,          1,  EBUSY,                              "MFics_ThrdPoolStop");
const u32 MFics_ThrdPoolExcdMaxTask                 = FiErr::MakeBaseCode_Erro(M_Fics,          2,  EBUSY,                              "MFics_ThrdPoolExcdMaxTask"); 
const u32 MFics_ThrdPoolExcdEntry                   = FiErr::MakeBaseCode_Erro(M_Fics,          3,  EBUSY,                              "MFics_ThrdPoolExcdEntry");   
const u32 MFics_ThrdPoolTaskClr                     = FiErr::MakeBaseCode_Erro(M_Fics,          4,  EBUSY,                              "MFics_ThrdPoolTaskClr");    

const u32 MDisk_fseekFailed                         = FiErr::MakeBaseCode_Erro(M_Disk,          1,  EIO,                                                "MDisk_fseekFailed");     
const u32 MDisk_fwriteFailed                        = FiErr::MakeBaseCode_Erro(M_Disk,          2,  EIO,                                                "MDisk_fwriteFailed");     
const u32 MDisk_freadFailed                         = FiErr::MakeBaseCode_Erro(M_Disk,          3,  EIO,                                                "MDisk_freadFailed");     
const u32 MDisk_fileNotExist                        = FiErr::MakeBaseCode_Erro(M_Disk,          4,  EIO,                                                "MDisk_fileNotExist");     
const u32 MDisk_endOfFile                           = FiErr::MakeBaseCode_Erro(M_Disk,          5,  EIO,                                                "MDisk_endOfFile");     

const u32 MNetSyn_ZKFailed                          = FiErr::MakeBaseCode_Erro(M_NetSyn,    	1,  EIO,                                                "MNetSyn_ZKFailed");

const u32 MOsd_MainNodeIsOK                         = FiErr::MakeBaseCode_Erro(M_Osd,        	1,  EIO,              					"MOsd_MainNodeIsOK");     
const u32 MOsd_LocNodeIsNotOK                       = FiErr::MakeBaseCode_Erro(M_Osd,        	2,  EIO,              					"MOsd_LocNodeIsNotOK");     
const u32 MOsd_RemoteNodeIsNotOK                    = FiErr::MakeBaseCode_Erro(M_Osd,        	3,  EIO,              					"MOsd_RemoteNodeIsNotOK");     

const u32 MJournal_headErr                          = FiErr::MakeBaseCode_Erro(M_Journal,       1,  EIO,              ,                 "MJournal_headErr");     
const u32 MJournal_endoffile                        = FiErr::MakeBaseCode_Erro(M_Journal,       2,  EIO,                                "MJournal_endoffile");
const u32 MJournal_stop                             = FiErr::MakeBaseCode_Erro(M_Journal,       3,  EIO,                                "MJournal_stop");
const u32 MJournal_stopping                         = FiErr::MakeBaseCode_Erro(M_Journal,       4,  EIO,                                "MJournal_stopping");

const u32 MOther_fileError                          = FiErr::MakeBaseCode_Erro(M_Other,         1,  EIO,                                "MOther_fileError");

const u32 MNet_SendFailed                           = FiErr::MakeBaseCode_Erro(M_Net,           1,  EIO,                                                "MNet_SendFailed");     

/// btree error
const u32 MBtree_notExist                           = FiErr::MakeBaseCode_Erro(M_Btree,         1,  EIO,                                "MBtree_notExist");

const u32 MBtree_noMorefile                         = FiErr::MakeBaseCode_Suc(M_Btree,         2,  EIO,                                "MBtree_noMorefile");
const u32 MBtree_fileHasExisted                         = FiErr::MakeBaseCode_Erro(M_Btree,         3, EIO,"MBtree_fileHasExisted");

const u32 MBitmap_readdiskerror                     = FiErr::MakeBaseCode_Suc(M_Bitmap,         1, EIO,"MBitmap_readddiskerror");
/// scm error
const u32 Scm_RouterSendFailed						= FiErr::MakeBaseCode_Erro(M_Scm,			6,	EIO,			"Scm_RouterSendFailed");
#endif 
