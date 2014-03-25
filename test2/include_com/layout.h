/*
 * layout.h - All DIFS associated on-disk structures.
 */

#ifndef _WIN_DIFS_LAYOUT_H
#define _WIN_DIFS_LAYOUT_H

#include "difstypes.h"
#include "difsdiskext.h"

#pragma pack(push,1)
/*
 * Constant endianness conversion defines.
 */

/* The DIFS oem_id "DIFS    "*/
#define magicDIFS       const_cpu_to_le64(0x303030306457655fULL)
#define Max64Number     const_cpu_to_le64(0xFFFFFFFFFFFFFFFF)
/*
 * Defined bits for the flags field in the _Volume structure.
 */
typedef enum {
    NV_Errors           =   0x1,    /* 1: Volume has errors, prevent remount rw. */
    NV_ShowSystemFiles  =   0x2,    /* 1: Return system files in Difs_readdir(). */
    NV_CaseSensitive    =   0x4,    /* 1: Treat file names as case sensitive and  create filenames in the POSIX namespace.
                      Otherwise be case insensitive and create  file names in WIN32 namespace. */
} Volume_flags;

// the type of the internal partition type
typedef enum 
{
    VolType_System      = const_cpu_to_le16(0x0001),
    VolType_Data        = const_cpu_to_le16(0x0002)
} VolumeType ;

/*
 * Location of bootsector on partition:
 *  The standard DIFS_BOOT_SECTOR is on sector 0 of the partition.
 *  there is one backup copy of the boot sector to
 *  be found on the last sector of the partition (not normally accessible
 *  from within Windows as the bootsector contained number of sectors
 *  value is one less than the actual value!).
 */



typedef struct _DIFS_SECOND_SECTOR
{
    u32 VolumeLabelLength;
    WCHAR VolumeLabel[16];
}DIFS_SECOND_SECTOR, *PDIFS_SECOND_SECTOR;

/*
 * loglist 
 */
typedef struct  
{
    u32 volid;           /*sysvol = 0   datavol = 1..2..   END = 0xFFFFFFFF*/
    s64 logstart_lcn;    
    s64 loglength;    
}DIFS_LOGLIST,*PDIFS_LOGLIST;

/*
 * Magic identifiers present at the beginning of all DIFS record containing
 * records (like mft records for example).
 */
typedef enum 
{
    magic_BAAD = const_cpu_to_le32(0x44414142), /* BAAD == corrupt record */
    magic_CHKD = const_cpu_to_le32(0x424b4843), /* CHKD == chkdsk ??? */
    magic_FILE = const_cpu_to_le32(0x454c4946), /* FILE == mft entry */
    magic_HOLE = const_cpu_to_le32(0x454c4f48), /* HOLE == ?  */
    magic_INDX = const_cpu_to_le32(0x58444e49), /* INDX == index buffer */
    magic_SIDX = const_cpu_to_le32(0x58444953), /* SIDX == space index buffer */
} DIFS_RECORD_TYPES;

/*
 * Generic magic comparison macros. Finally found a use for the ## preprocessor
 * operator! (-8
 */
#define is_magic(x, m)          (   (ul32)(x) == magic_##m )
#define is_magicp(p, m)         ( *(ul32*)(p) == magic_##m )

/*
 * Specialised magic comparison macros.
 */
#define is_baad_record(x)       ( is_magic (x, BAAD) )
#define is_baad_recordp(p)      ( is_magicp(p, BAAD) )
#define is_chkd_record(x)       ( is_magic (x, CHKD) )
#define is_chkd_recordp(p)      ( is_magicp(p, CHKD) )
#define is_file_record(x)       ( is_magic (x, FILE) )
#define is_file_recordp(p)      ( is_magicp(p, FILE) )
#define is_hole_record(x)       ( is_magic (x, HOLE) )
#define is_hole_recordp(p)      ( is_magicp(p, HOLE) )
#define is_indx_record(x)       ( is_magic (x, INDX) )
#define is_indx_recordp(p)      ( is_magicp(p, INDX) )

#define is_mft_record(x)        ( is_file_record(x) )
#define is_mft_recordp(p)       ( is_file_recordp(p) )

typedef struct 
{
    DIFS_RECORD_TYPES magic;    /* A four-byte magic identifying the record type and/or status. */
    u16 usa_ofs;        /* Offset to the Update Sequence Array (usa) from the start of the DIFS record. */
    u16 usa_count;      /* Number of u16 sized entries in the usa  including the Update Sequence Number (usn),
                   thus the number of fixups is the usa_count minus 1. */
}  DIFS_RECORD, *PDIFS_RECORD;

/*
 * System files mft record numbers. All these files are always marked as used
 * in the bitmap attribute of the mft; presumably in order to avoid accidental
 * allocation for random other mft records. Also, the sequence number for each
 * of the system files is always equal to their mft record number and it is
 * never modified.
 */
typedef enum 
{
    FILE_SV_SMFT        =   0,  /*SYSTEM Master file table (mft). Data attribute
                   contains the entries and bitmap attribute records which ones are in use (bit==1). */
    FILE_SV_SMFTMirr    =   1,  /*System Mft mirror: copy of first four mft records
                   in data attribute. If cluster size > 4kiB,copy of first N mft records, with
                    N = cluster_size / mft_record_size. */
    FILE_SV_LogFile         =   2,  /* Journalling log in data attribute. */
    FILE_SV_SysVol          =   3,  /* system Volume name attribute and volume information  attribute (flags and DIFS version).*/
    File_SV_DataVolList =   4,
    FILE_SV_AttrDef         =   5,  /* Array of attribute definitions in data attribute. */
    FILE_SV_Root            =   6,  /* Root directory. */
    FILE_SV_IdleAreaMap     =   7 ,//idle area map   
    FILE_SV_Boot            =   8,  /* Boot sector (always at cluster 0) in data attribute. */
    FILE_SV_Bitmap          =   9,  /* Allocation bitmap of all clusters (lcns) in data attribute. */
 
    FILE_SV_BadClus         =   10, /* Contains all bad clusters in the non-resident data attribute. */
    FILE_SV_Quota           =   11,
    FILE_SV_Secure         =    12, /* Shared security descriptors in data attribute
                   and two indexes into the descriptors. Appeared in Windows 2000. Before that, this
                   file was named $Quota but was unused. */
    FILE_SV_UpCase          =   13, /* Uppercase equivalents of all 65536 Unicode characters in data attribute. */
    FILE_SV_Extend          =   14, /* Directory containing other system files (eg. $ObjId, $Quota, $Reparse and $UsnJrnl). */
    FILE_SV_DataVol     =   15, /* data Volume name attribute and volume information  attribute (flags and DIFS version).*/
    FILE_SV_DirDir          =   16,
} DIFS_SYSVOL_FILES;

typedef enum 
{
    FILE_DV_DMFT        =   0,  /*Data Master file table (mft). Data attribute
                   contains the entries and bitmap attribute records which ones are in use (bit==1). */
    FILE_DV_DMFTMirr   =    1,  /*Data Mft mirror: copy of first four mft records
                   in data attribute. If cluster size > 4kiB, copy of first N mft records, with
                    N = cluster_size / mft_record_size. */
    FILE_DV_LogFile     =   2,  /* Journalling log in data attribute. */
    FILE_DV_DataVol    =    3,  /* data Volume name attribute and volume information  attribute (flags and DIFS version).*/
    FILE_DV_Bitmap      =   4,  /* Allocation bitmap of all clusters (lcns) in data attribute. */
    FILE_DV_BadClus     =   5,  /* Contains all bad clusters in the non-resident data attribute. */
    FILE_DV_DirFile =   6,
    FILE_DV_UsrFile =   7,
 } DIFS_DATAVOL_FILES;

/*
 * These are the so far known MFT_RECORD_* flags (16-bit) which contain 
 * information about the mft record in which they are present.
 */
typedef enum 
{
    MFT_RECORD_IN_USE               = const_cpu_to_le16(0x0001),
    MFT_RECORD_IS_DIRECTORY_DIR     = const_cpu_to_le16(0x0002),
    MFT_RECORD_IS_DIRECTORY_FILE    = const_cpu_to_le16(0x0003),
    MFT_RECORD_IS_DATA_VOL          = const_cpu_to_le16(0x0004),
    MFT_REC_SPACE_FILLER            = 0xffff    /* Just to make flags 16-bit. */
}  MFT_RECORD_FLAGS;



/*
 * Typedef the MFT_REF as a 64-bit value for easier handling.
 * Also define two unpacking macros to get to the reference (MREF) and
 * sequence number (MSEQNO) respectively.
 * The _LE versions are to be applied on little endian MFT_REFs.
 * Note: The _LE versions will return a CPU endian formatted value!
 */
#define     MFT_REF_MASK_CPU    0x0000ffffffffffffUL
#define     MFT_REF_MASK_LE     const_cpu_to_le64(0x0000ffffffffffffUL)


typedef u64 MFT_REF;

#define MAKE_MFT_REF(seqNo, mftNo) ((u64) ((((u64)seqNo) << 48) + (u64)mftNo)) 
#define MREF(x)     ((unsigned long)((x) & MFT_REF_MASK_CPU))
#define MSEQNO(x)   ((u16)(((x) >> 48) & 0xffff))
#define MREF_LE(x)  ((unsigned long)(le64_to_cpu(x) & MFT_REF_MASK_CPU))
#define MSEQNO_LE(x)    ((u16)((le64_to_cpu(x) >> 48) & 0xffff))

#define IS_ERR_MREF(x)  (((x) & 0x0000800000000000ULL) ? 1 : 0)
#define ERR_MREF(x) ((u64)((s64)(x)))
#define MREF_ERR(x) ((int)((s64)(x)))


typedef struct 
{
/*Ofs*/
    DIFS_RECORD_TYPES magic;/* Usually the magic is "FILE". */
    u16 usa_ofs;        /* See DIFS_RECORD definition above. */
    u16 usa_count;      /* See DIFS_RECORD definition above. */

/*  8*/ u64 lsn;        /* $LogFile sequence number for this record. Changed every time the record is modified. */
/* 16*/ u16 sequence_number;    /* Number of times this mft record has been reused. (See description for MFT_REF
                   above.) NOTE: The increment (skipping zero)  is done when the file is deleted. NOTE: If
                   this is zero it is left zero. */
/* 18*/ u16 link_count;     /* Number of hard links, i.e. the number of directory entries referencing this record.
                   NOTE: Only used in mft base records. NOTE: When deleting a directory entry we
                   check the link_count and if it is 1 we  delete the file. Otherwise we delete the
                   FILE_NAME_ATTR being referenced by the  directory entry from the mft record and
                   decrement the link_count. Careful with Win32 + DOS names! */
/* 20*/ u16 attrs_offset;   /* Byte offset to the first attribute in this mft record from the start of the mft record.
                   NOTE: Must be aligned to 8-byte boundary. */
/* 22*/ u16 flags;  /* Bit array of MFT_RECORD_FLAGS. When a file
                   is deleted, the MFT_RECORD_IN_USE flag is  set to zero. */
/* 24*/ u32 bytes_in_use;  /* Number of bytes used in this mft record.not aligned to 8-byte boundary. */
/* 28*/ u32 bytes_allocated;   /* Number of bytes allocated for this mft
                   record. This should be equal to the mft record size. */
/* 32*/ MFT_REF base_mft_record; /* This is zero for base mft records.
                   When it is not zero it is a mft reference pointing to the base mft record to which
                   this record belongs (this is then used to locate the attribute list attribute present
                   in the base record which describes this extension record and hence might need
                   modification when the extension record itself is modified, also locating the
                   attribute list also means finding the other potential extents, belonging to the non-base
                   mft record). */
/* 40*/ u16 next_attr_instance; /* The instance number that will be
                   assigned to the next attribute added to this mft record. NOTE: Incremented each time
                   after it is used. NOTE: Every time the mft  record is reused this number is set to zero.
                   NOTE: The first instance number is always 0. */
        u16 attrNumber; /*the total number of attribute in this file record*/
/* sizeof() = 42 bytes */
/* 42*/ u16 reserved;       /* Reserved/alignment. */
/* 44*/ u32 mft_record_number;/* Number of this mft record. */
/*48*/  u32 next_mft_record_number;/* Number of next relative mft record. */ 
        u64 next_mftfile_sector_no;/*mft file extending number*/
/*50*/  u16 chkdsk_usn;
        u32 parent_dir_base_mftno ;//parent dir file record no.
/* sizeof() = 54 bytes */
/*
 * When (re)using the mft record, we place the update sequence array at this
 * offset, i.e. before we start with the attributes. This also makes sense,
 * otherwise we could run into problems with the update sequence array
 * containing in itself the last two bytes of a sector which would mean that
 * multi sector transfer protection wouldn't work. As you can't protect data
 * by overwriting it since you then can't get it back...
 * When reading we obviously use the data from the DIFS record header.
 */
}  MFT_RECORD, *PMFT_RECORD;


typedef enum 
{
    AT_UNUSED                       =   const_cpu_to_le32(         0),
    AT_STANDARD_INFORMATION         =   const_cpu_to_le32(      0x10),
    AT_ATTRIBUTE_LIST               =   const_cpu_to_le32(      0x20),
    AT_FILE_NAME                    =   const_cpu_to_le32(      0x30),
    AT_OBJECT_ID                    =   const_cpu_to_le32(      0x40),
    AT_SECURITY_DESCRIPTOR          =   const_cpu_to_le32(      0x50),
    AT_VOLUME_NAME                  =   const_cpu_to_le32(      0x60),
    AT_VOLUME_INFORMATION           =   const_cpu_to_le32(      0x70),
    AT_DATA                         =   const_cpu_to_le32(      0x80),
    AT_INDEX_ROOT                   =   const_cpu_to_le32(      0x90),
    AT_INDEX_ALLOCATION             =   const_cpu_to_le32(      0xa0),
    AT_BITMAP                       =   const_cpu_to_le32(      0xb0),
    AT_REPARSE_POINT                =   const_cpu_to_le32(      0xc0),
    AT_EA_INFORMATION               =   const_cpu_to_le32(      0xd0),
    AT_EA                           =   const_cpu_to_le32(      0xe0),
    AT_PROPERTY_SET                 =   const_cpu_to_le32(      0xf0),
    AT_LOGGED_UTILITY_STREAM        =   const_cpu_to_le32(     0x100),
    AT_DATAAREALIST                 =   const_cpu_to_le32(     0x200),
    AT_SPACE_IDX_ROOT               =   const_cpu_to_le32(     0x300),
    AT_SPACE_IDX_ALLOCATION         =   const_cpu_to_le32(     0x400),
    AT_DATAVOL_ID                   =   const_cpu_to_le32(     0x500),
    AT_DVOLDIR_ID                   =   const_cpu_to_le32(     0x600),
    AT_SVOLDIR_ID                   =   const_cpu_to_le32(     0x700),
    AT_FIRST_USER_DEFINED_ATTRIBUTE =   const_cpu_to_le32(    0x1000),
    AT_END                          =   const_cpu_to_le32(0xffffffff),
} ATTR_TYPES;


typedef enum 
{
    COLLATION_BINARY                = const_cpu_to_le32(0), /* Collate by binary
                    compare where the first byte is most significant. */
    COLLATION_FILE_NAME             = const_cpu_to_le32(1), /* Collate file names as Unicode strings. */
    COLLATION_UNICODE_STRING        = const_cpu_to_le32(2), /* Collate Unicode
                    strings by comparing their binary   Unicode values, except that when a
                    character can be uppercased, the upper case value collates before the lower case one. */
    COLLATION_NTOFS_ULONG           = const_cpu_to_le32(16),
    COLLATION_NTOFS_SID             = const_cpu_to_le32(17),
    COLLATION_NTOFS_SECURITY_HASH   = const_cpu_to_le32(18),
    COLLATION_NTOFS_ULONGS          = const_cpu_to_le32(19),
} COLLATION_RULES;

/*
 * The flags (32-bit) describing attribute properties in the attribute
 * definition structure. */
typedef enum 
{
    INDEXABLE           = const_cpu_to_le32(0x02),  /* Attribute can be  indexed. */
    NEED_TO_REGENERATE  = const_cpu_to_le32(0x40),  /* Need to regenerate during regeneration phase. */
    CAN_BE_NON_RESIDENT = const_cpu_to_le32(0x80),  /* Attribute can be non-resident. */
} ATTR_DEF_FLAGS;


typedef struct 
{
/*hex ofs*/
/*  0*/ wchar_t name[0x40];     /* Unicode name of the attribute. Zero
                       terminated. */
/* 80*/ ATTR_TYPES type;        /* Type of the attribute. */
/* 84*/ ul32 display_rule;      /* Default display rule. FIXME: What does it mean? (AIA) */
/* 88*/ COLLATION_RULES collation_rule; /* Default collation rule. */
/* 8c*/ ATTR_DEF_FLAGS flags;       /* Flags describing the attribute. */
/* 90*/ u64 min_size;           /* Optional minimum attribute size. */
/* 98*/ u64 max_size;           /* Maximum size of attribute. */
/* sizeof() = 0xa0 or 160 bytes */
}  ATTR_DEF, *PATTR_DEF;

/*
 * Attribute flags (16-bit). 
 */
typedef enum 
{
    ATTR_IS_NORMAL          = const_cpu_to_le16(0x0000),
    ATTR_IS_COMPRESSED      = const_cpu_to_le16(0x0001),
    ATTR_COMPRESSION_MASK   = const_cpu_to_le16(0x00ff),  /* Compression method mask. Also, first   illegal value. */
    ATTR_IS_ENCRYPTED       = const_cpu_to_le16(0x4000),
    ATTR_IS_SPARSE          = const_cpu_to_le16(0x8000),
    ATTR_IS_EXTENDED        = const_cpu_to_le16(0x0100),
}  ATTR_FLAGS;

/*
 * Flags of resident attributes (8-bit).
 */
typedef enum 
{
    RESIDENT_ATTR_NO_INDEXED = 0X00 ,
    RESIDENT_ATTR_IS_INDEXED = 0x01, /* Attribute is referenced in an index
                        (has implications for deleting and modifying the attribute). */
}  RESIDENT_ATTR_FLAGS;

/*
 * Attribute record header. Always aligned to 8-byte boundary.
 */
typedef struct 
{
/*Ofs*/
/*  0*/ ATTR_TYPES      type;   /* The (32-bit) type of the attribute. */
/*  4*/ u32             length;     /* Byte size of the resident part of the attribute (aligned to 8-byte boundary). Used to get to the next attribute. */
/*  8*/ u8              non_resident;   /* If 0, attribute is resident. If 1, attribute is non-resident. */
/*  9*/ u8              name_length;        /* Unicode character size of name of attribute.  0 if unnamed. */
/* 10*/ u16             name_offset;    /* If name_length != 0, the byte offset to the beginning of the name from the attribute
                   record. Note that the name is stored as a Unicode string. When creating, place offset
                   just at the end of the record header. Then, follow with attribute value or mapping pairs
                   array, resident and non-resident attributes respectively, aligning to an 8-byte
                   boundary. */
/* 12*/ u16             flags;  /* Flags describing the attribute. */
/* 14*/ u16             instance;       /* The instance of this attribute record. This number is unique within this mft record (see 
                   MFT_RECORD/next_attribute_instance notes in mft.h for more details). */
/* 16*/ union 
        {
        /* Resident attributes. */
            struct 
            {
    /* 16 */        ul32 value_length; /* Byte size of attribute value. */
    /* 20 */        u16 value_offset; /* Byte offset of the attribute value from the start of the
                             attribute record. When creating, align to 8-byte boundary if we 
                             have a name present as this might not have a length of a multiple of 8-bytes. */
    /* 22 */        u8 flags; /* See above. */
    /* 23 */        s8 reserved;      /* Reserved/alignment to 8-byte boundary. */
            }  resident;
            /* Non-resident attributes. */
            struct 
            {
    /* 16*/         VCN lowest_vcn; /* Lowest valid virtual cluster number for this portion of the attribute value or
                    0 if this is the only extent (usually the case). - Only when an attribute list is used
                    does lowest_vcn != 0 ever occur. */
    /* 24*/         VCN highest_vcn; /* Highest valid vcn of this extent of the attribute value. - Usually there is only one
                    portion, so this usually equals the attribute value size in clusters minus 1. Can be -1 for
                    zero length files. Can be 0 for "single extent" attributes. */
    /* 32*/         u16 mapping_pairs_offset; /* Byte offset from the beginning of the structure to the mapping pairs
                    array which contains the mappings between the vcns and the logical cluster numbers (lcns).
                    When creating, place this at the end of this record header aligned to 8-byte boundary. */
    /* 34*/         u8 compression_unit; /* The compression unit expressed as the log to the base 2 of the number of
                    clusters in a compression unit. 0 means not compressed. (This effectively limits the
                    compression unit size to be a power of two clusters.) WinNT4 only uses a value of 4. */
    /* 35*/         u8 reserved[5];     /* Align to 8-byte boundary. */
    /* The sizes below are only used when lowest_vcn is zero, as otherwise it would be difficult to keep them up-to-date.*/
    /* 40*/         s64 allocated_size; /* Byte size of disk space  allocated to hold the attribute value. Always
                    is a multiple of the cluster size. When a file  is compressed, this field is a multiple of the
                    compression block size (2^compression_unit) and it represents the logically allocated space
                    rather than the actual on disk usage. For this use the compressed_size (see below). */
    /* 48*/         s64 data_size;  /* Byte size of the attribute value. Can be larger than allocated_size if
                    attribute value is compressed or sparse. */
    /* 56*/         s64 initialized_size;   /* Byte size of initialized portion of the attribute value. Usually equals
                    data_size. */
    /* sizeof(uncompressed attr) = 64*/
    /* 64*/         s64 compressed_size;    /* Byte size of the attribute value after compression. Only present when
                    compressed. Always is a multiple of the cluster size. Represents the actual amount of
                    disk space being used on the disk. */
    /*72*/          s64 file_size;
    /* sizeof(compressed attr) = 80*/
            }  non_resident;
        }  data;
}  ATTR_RECORD, *PATTR_RECORD;

#define ATTRNODE_BASE_LEN 16
#define ATTRNODE_RESIDENT_LEN 8
#define ATTRNODE_NONRESIDENT_LEN 64

/*
 * NOTE on times in DIFS: All times are in MS standard time format, i.e. they
 * are the number of 100-nanosecond intervals since 1st January 1601, 00:00:00
 * universal coordinated time (UTC). (In Linux time starts 1st January 1970,
 * 00:00:00 UTC and is stored as the number of 1-second intervals since then.)
 */

/*
 * Attribute: Standard information (0x10).
 *
 * NOTE: Always resident.
 * NOTE: Present in all base file records on a volume.
 * NOTE: There is conflicting information about the meaning of each of the time
 *   fields but the meaning as defined below has been verified to be
 *   correct by practical experimentation on Windows NT4 SP6a and is hence
 *   assumed to be the one and only correct interpretation.
 */
typedef struct 
{
/*Ofs*/
/*  0*/ s64 creation_time;      /* Time file was created. Updated when a filename is changed(?). */
/*  8*/ s64 last_data_change_time;  /* Time the data attribute was last modified. */
/* 16*/ s64 last_mft_change_time;   /* Time this mft record was last modified. */
/* 24*/ s64 last_access_time;       /* Approximate time when the file was
                       last accessed (obviously this is not updated on read-only volumes). In
                       Windows this is only updated when accessed if some time delta has
                       passed since the last update. Also, last access times updates can be disabled altogether for speed. */
/* 32*/ FILE_ATTR_FLAGS file_attributes; /* Flags describing the file. */

    /* sizeof() = 48 bytes */
/* 36*/ ul32 maximum_versions;  /* Maximum allowed versions for file. Zero if version numbering is disabled. */
/* 40*/ ul32 version_number;    /* This file's version (if any).Set to zero if maximum_versions is zero. */
/* 44*/ ul32 class_id;      /* Class id from bidirectional class id index (?). */
/* 48*/ ul32 owner_id;      /* Owner_id of the user owning the file. Translate via $Q index in FILE_Extend
        /$Quota to the quota control entry for the user owning the file. Zero if quotas are disabled. */
/* 52*/ ul32 security_id;   /* Security_id for the file.
        Translate via $SII index and $SDS data stream in FILE_Secure to the security descriptor. */
/* 56*/ u64 quota_charged;  /* Byte size of the charge to
        the quota for all streams of the file. Note: Is zero if quotas are disabled. */
/* 64*/ u64 usn;        /* Last update sequence number
        of the file. This is a direct index into the  change (aka usn) journal file. It is zero if
        the usn journal is disabled.
        NOTE: To disable the journal need to delete the journal file itself and to then walk the
        whole mft and set all Usn entries in all mft records to zero! (This can take a while!)
        The journal is FILE_Extend/$UsnJrnl. Win2k will recreate the journal and initiate
        logging if necessary when mounting the partition. This, in contrast to disabling the
        journal is a very fast process, so the user won't even notice it. */
 
    /* sizeof() = 72 bytes (DIFS 3.x) */

}  STANDARD_INFORMATION, *PSTANDARD_INFORMATION;


typedef struct 
{
/*Ofs*/
/*  0*/ ATTR_TYPES type;    /* Type of referenced attribute. */
/*  4*/ u16 length;     /* Byte size of this entry (8-byte aligned). */
/*  6*/ u8 name_length;     /* Size in Unicode chars of the name of the attribute or 0 if unnamed. */
/*  7*/ u8 name_offset;     /* Byte offset to beginning of attribute name (always set this to where the name wouldstart even if unnamed). */
/*  8*/ VCN lowest_vcn;     /* Lowest virtual cluster number of this portion
                   of the attribute value. This is usually 0. It is non-zero for the case where one attribute
                   does not fit into one mft record and thus several mft records are allocated to hold
                   this attribute. In the latter case, each mft record holds one extent of the attribute and
                   there is one attribute list entry for each  extent. 
                   NOTE: This is DEFINITELY a signed value! The windows driver uses cmp, followed
                   by jg when comparing this, thus it treats it as signed. */
/* 16*/ MFT_REF mft_reference;  /* The reference of the mft record holding the ATTR_RECORD for this portion of the  attribute value. */
/* 24*/ u16 instance;       /* If lowest_vcn = 0, the instance of the attribute being referenced; otherwise 0. */
/* 26*/ wchar_t* name;  /* Use when creating only. When reading use name_offset to determine the location of the name. */
/* sizeof() = 26 + (attribute_name_length * 2) bytes */
}  ATTR_LIST_ENTRY, *PATTR_LIST_ENTRY;

/*
 * The maximum allowed length for a file name.
 */
#define MAXIMUM_FILE_NAME_LENGTH    255

/*
 * Possible namespaces for filenames in DIFS (8-bit).
 */
typedef enum 
{
    FILE_NAME_POSIX         = 0x00,
        /* This is the largest namespace. It is case sensitive and 
           allows all Unicode characters except for: '\0' and '/'.
           Beware that in WinNT/2k files which eg have the same name
           except for their case will not be distinguished by the
           standard utilities and thus a "del filename" will delete
           both "filename" and "fileName" without warning. */
    FILE_NAME_WIN32         = 0x01,
        /* The standard WinNT/2k DIFS long filenames. Case insensitive.
           All Unicode chars except: '\0', '"', '*', '/', ':', '<', 
           '>', '?', '\' and '|'. Further, names cannot end with a '.'
           or a space. */
    FILE_NAME_DOS           = 0x02,
        /* The standard DOS filenames (8.3 format). Uppercase only.
           All 8-bit characters greater space, except: '"', '*', '+',
           ',', '/', ':', ';', '<', '=', '>', '?' and '\'. */
    FILE_NAME_WIN32_AND_DOS     = 0x03, 
        /* 3 means that both the Win32 and the DOS filenames are
           identical and hence have been saved in this single filename
           record. */
}  FILE_NAME_TYPE_FLAGS;


typedef struct 
{
/* ofs*/
/*  0*/ MFT_REF         parent_directory;   /* Directory this filename is referenced from. */
/*  8*/ s64             creation_time;          /* Time file was created. */
/* 16*/ s64             last_data_change_time;  /* Time the data attribute was last modified. */
/* 24*/ s64             last_mft_change_time;   /* Time this mft record was last  modified. */
/* 32*/ s64             last_access_time;       /* Last time this mft record was accessed. */
/* 40*/ s64             allocated_size;         /* Byte size of allocated space for the
                       data attribute. NOTE: Is a multiple of the cluster size. */
/* 48*/ s64             data_size;              /* Byte size of actual data in data  attribute. */
        s64             file_size;
/* 56*/ u16             VolIndex ;//which volume this file is on
/* 58*/ FILE_ATTR_FLAGS file_attributes;    /* Flags describing the file. */
/* 62*/ union 
        {
    /* 62*/ struct 
            {
        /* 62*/ u16 packed_ea_size; /* Size of the buffer needed to pack the extended attributes (EAs), if such are present.*/
        /* 64*/ u16 reserved;       /* Reserved for alignment. */
            }  ea;
    /* 62*/ struct 
            {
        /* 62*/ ul32 reparse_point_tag; /* Type of reparse point,present only in reparse points and only if there are no EAs. */
            }  rp;
        }  type;
/* 66*/ u16 file_name_length;            /* Length of file name in (Unicode) characters. */
/* 74*/ u8  file_name_type; /* Namespace of the file name.*/
}  FILE_NAME_ATTR, *PFILE_NAME_ATTR;


typedef struct 
{
    MFT_REF mft_reference;  /* Mft record containing the object_id in the index entry key. */
    union 
    {
        struct 
        {
            GUID birth_volume_id;
            GUID birth_object_id;
            GUID domain_id;
        }  origin;
        u8 extended_info[48];
    }  opt;
}  OBJ_ID_INDEX_DATA, *POBJ_ID_INDEX_DATA;

/*
 * Attribute: Object id  (0x40).
 *
 * NOTE: Always resident.
 */
typedef struct 
{
    GUID object_id;             /* Unique id assigned to the file.*/
    /* The following fields are optional. The attribute value size is 16
       bytes, i.e. sizeof(GUID), if these are not present at all. Note,
       the entries can be present but one or more (or all) can be zero
       meaning that that particular value(s) is(are) not defined. */
    union 
    {
        struct 
        {
            GUID birth_volume_id;   /* Unique id of volume on which  the file was first created.*/
            GUID birth_object_id;   /* Unique id of file when it was first created. */
            GUID domain_id;     /* Reserved, zero. */
        }  origin;
        u8 extended_info[48];
    }  opt;
}  OBJECT_ID_ATTR, *POBJECT_ID_ATTR;

/*
 * The index entry key used in the $SII index. The collation type is COLLATION_NTOFS_ULONG. 
 */
typedef struct 
{
    ul32 security_id; /* The security_id assigned to the descriptor. */
}  SII_INDEX_KEY;

/*
 * The index entry key used in the $SDH index. The keys are sorted first by
 * hash and then by security_id. The collation rule is
 * COLLATION_NTOFS_SECURITY_HASH.
 */
typedef struct 
{
    ul32 hash;     /* Hash of the security descriptor. */
    ul32 security_id;   /* The security_id assigned to the descriptor. */
}  SDH_INDEX_KEY;


/*
 * Possible flags for the volume (16-bit).
 */
typedef enum 
{
    DIFS_VOLUME_IS_DIRTY                = const_cpu_to_le16(0x0001),
    DIFS_VOLUME_RESIZE_LOG_FILE         = const_cpu_to_le16(0x0002),
    DIFS_VOLUME_UPGRADE_ON_MOUNT        = const_cpu_to_le16(0x0004),
    DIFS_VOLUME_MOUNTED_ON_NT4          = const_cpu_to_le16(0x0008),
    DIFS_VOLUME_DELETE_USN_UNDERWAY     = const_cpu_to_le16(0x0010),
    DIFS_VOLUME_REPAIR_OBJECT_ID        = const_cpu_to_le16(0x0020),
    DIFS_VOLUME_MODIFIED_BY_CHKDSK      = const_cpu_to_le16(0x8000),
    DIFS_VOLUME_FLAGS_MASK              = const_cpu_to_le16(0x803f),
}  DIFS_VOLUME_FLAGS;

/*
 * Attribute: Volume information (0x70).
 *
 * NOTE: Always resident.
 * NOTE: Present only in FILE_Volume.
 */
typedef struct
{
    u16             VolIndex ;              //volume index in global ,Exterior partition is 0 .
    u16             volType ;
    LCN             VolumeStartLcn;         /*relative to the start of exterior partition, for exterior partition this is 0 */
    u8              major_ver;              /* Major version of the DIFS format. */
    u8              minor_ver;              /* Minor version of the DIFS format. */
    u16             flags;                  /* Bit array of VOLUME_* flags. */
    u8              sectors_per_cluster;    /* Size of a cluster in sectors. */
    s64             number_of_sectors;
}  VOLUME_INFORMATION, *PVOLUME_INFORMATION;

/*
 * Attribute: Data attribute (0x80).
 *
 * NOTE: Can be resident or non-resident.
 *
 * Data contents of a file (i.e. the unnamed stream) or of a named stream.
 */
typedef struct 
{
    Run_list    runlist ;
    int         nDataLength;
    u8*         pData;      /* The file's data contents. */
}  DATA_ATTR, *PDATA_ATTR;


/*
 * The system file FILE_Extend/$Reparse contains an index named $R listing
 * all reparse points on the volume. The index entry keys are as defined
 * below. Note, that there is no index data associated with the index entries.
 *
 * The index entries are sorted by the index key file_id. The collation rule is
 * COLLATION_NTOFS_ULONGS. FIXME: Verify whether the reparse_tag is not the
 * primary key / is not a key at all. (AIA)
 */
typedef struct 
{
    ul32 reparse_tag;   /* Reparse point type (inc. flags). */
    MFT_REF file_id;    /* Mft record of the file containing the reparse point attribute. */
}  DIFS_REPARSE_INDEX_KEY, *PDIFS_REPARSE_INDEX_KEY;

/*
 * Quota flags (32-bit).
 */
typedef enum 
{
    /* The user quota flags. Names explain meaning. */
    QUOTA_FLAG_DEFAULT_LIMITS   = const_cpu_to_le32(0x00000001),
    QUOTA_FLAG_LIMIT_REACHED    = const_cpu_to_le32(0x00000002),
    QUOTA_FLAG_ID_DELETED       = const_cpu_to_le32(0x00000004),

    QUOTA_FLAG_USER_MASK        = const_cpu_to_le32(0x00000007),
        /* Bit mask for user quota flags. */
    
    /* These flags are only present in the quota defaults index entry,
       i.e. in the entry where owner_id = QUOTA_DEFAULTS_ID. */
    QUOTA_FLAG_TRACKING_ENABLED = const_cpu_to_le32(0x00000010),
    QUOTA_FLAG_ENFORCEMENT_ENABLED  = const_cpu_to_le32(0x00000020),
    QUOTA_FLAG_TRACKING_REQUESTED   = const_cpu_to_le32(0x00000040),
    QUOTA_FLAG_LOG_THRESHOLD    = const_cpu_to_le32(0x00000080),
    QUOTA_FLAG_LOG_LIMIT        = const_cpu_to_le32(0x00000100),
    QUOTA_FLAG_OUT_OF_DATE      = const_cpu_to_le32(0x00000200),
    QUOTA_FLAG_CORRUPT      = const_cpu_to_le32(0x00000400),
    QUOTA_FLAG_PENDING_DELETES  = const_cpu_to_le32(0x00000800),
} QUOTA_FLAGS;

typedef struct 
{
    ul32 version;       /* Currently equals 2. */
    QUOTA_FLAGS flags;  /* Flags describing this quota entry. */
    u64 bytes_used;     /* How many bytes of the quota are in use. */
    s64 change_time;    /* Last time this quota entry was changed. */
    s64 threshold;      /* Soft quota (-1 if not limited). */
    s64 limit;      /* Hard quota (-1 if not limited). */
    s64 exceeded_time;  /* How long the soft quota has been exceeded. */
    SID sid;        /* The SID of the user/object associated with
                   this quota entry. Equals zero for the quota
                   defaults entry. */
}  QUOTA_CONTROL_ENTRY;

/*
 * Predefined owner_id values (32-bit).
 */
typedef enum 
{
    QUOTA_INVALID_ID    = const_cpu_to_le32(0x00000000),
    QUOTA_DEFAULTS_ID   = const_cpu_to_le32(0x00000001),
    QUOTA_FIRST_USER_ID = const_cpu_to_le32(0x00000100),
} PREDEFINED_OWNER_IDS;


/*
 * Index header flags (8-bit).
 */
typedef enum 
{
    /* When index header is in an index root attribute: */
    SMALL_INDEX = 0, /* The index is small enough to fit inside the
                index root attribute and there is no index
                allocation attribute present. */
    LARGE_INDEX = 1, /* The index is too large to fit in the index
                root attribute and/or an index allocation
                attribute is present. */
    /*
     * When index header is in an index block, i.e. is part of index
     * allocation attribute:
     */
    LEAF_NODE   = 0, /* This is a leaf node, i.e. there are no more nodes branching off it. */
    INDEX_NODE  = 1, /* This node indexes other nodes, i.e. is not a leaf node. */
    NODE_MASK   = 1, /* Mask for accessing the *_NODE bits. */
}  INDEX_HEADER_FLAGS;

/*
 * This is the header for indexes, describing the INDEX_ENTRY records, which
 * follow the INDEX_HEADER. Together the index header and the index entries
 * make up a complete index.
 *
 * IMPORTANT NOTE: The offset, length and size structure members are counted
 * relative to the start of the index header structure and not relative to the
 * start of the index root or index allocation structures themselves.
 */
typedef struct 
{
/*4*/   ul32 entries_offset;        /* Byte offset to first INDEX_ENTRY aligned to 8-byte boundary. */
/*8*/   ul32 index_length;      /* Data size of the index in bytes, i.e. bytes used from allocated size, aligned to 8-byte boundary. */
/*12*/  ul32 allocated_size;        /* Byte size of this index (block), multiple of 8 bytes. */
    /* NOTE: For the index root attribute, the above two numbers are always
       equal, as the attribute is resident and it is resized as needed. In
       the case of the index allocation attribute the attribute is not
       resident and hence the allocated_size is a fixed value and must
       equal the index_block_size specified by the INDEX_ROOT attribute
       corresponding to the INDEX_ALLOCATION attribute this INDEX_BLOCK belongs to. */
/*13*/  u8       flags; /* Bit field of INDEX_HEADER_FLAGS. */
/*15*/  u16     nEntryNumber ;//number of entry in this indx record or dir root attr
/*23*/  VCN parent_index_block_vcn;
/*31*/  VCN backward_index_block_vcn;   /* Virtual cluster number of the backward  index block.
                   index_block_vcn counts in units of  units of allocation_block . */
/*39*/  VCN forward_index_block_vcn;    /* Virtual cluster number of the forward  index block.
                   index_block_vcn counts in units of  units of allocation_block . */
        ul32 max_entry_num;
/*40*/  u8 reserved;            /* Reserved/align to 8-byte boundary. */
}  INDEX_HEADER, *PINDEX_HEADER;

/*
 * Attribute: Index root (0x90).
 *
 * NOTE: Always resident.
 *
 * This is followed by a sequence of index entries (INDEX_ENTRY structures)
 * as described by the index header.
 *
 * When a directory is small enough to fit inside the index root then this
 * is the only attribute describing the directory. When the directory is too
 * large to fit in the index root, on the other hand, two aditional attributes
 * are present: an index allocation attribute, containing sub-nodes of the B+
 * directory tree (see below), and a bitmap attribute, describing which virtual
 * cluster numbers (vcns) in the index allocation attribute are in use by an
 * index block.
 *
 * NOTE: The root directory (FILE_root) contains an entry for itself. Other
 * dircetories do not contain entries for themselves, though.
 */
typedef struct 
{
/*4*/   ATTR_TYPES type;    /* Type of the indexed attribute. Is  $FILE_NAME for directories, zero
                       for view indexes. No other values  allowed. */
/*8*/   COLLATION_RULES collation_rule; /* Collation rule used to sort the
                       index entries. If type is $FILE_NAME,
                       this must be COLLATION_FILE_NAME. */
/*12*/  ul32 index_block_size;      /* Size of each index block in bytes (in
                       the index allocation attribute). */
/*13*/  u8 sectors_per_index_block; /* sector size of each index block */
/*16*/  u8 reserved[3];         /* Reserved/align to 8-byte boundary. */
/*48*/  INDEX_HEADER index;     /* Index header describing the following index entries. */
}  INDEX_ROOT, *PINDEX_ROOT;

/*
 * Attribute: Index allocation (0xa0).
 *
 * NOTE: Always non-resident (doesn't make sense to be resident anyway!).
 * 
 * This is an array of index blocks. Each index block starts with an
 * INDEX_BLOCK structure containing an index header, followed by a sequence of
 * index entries (INDEX_ENTRY structures), as described by the INDEX_HEADER.
 */
typedef struct 
{
/*  8   DIFS_RECORD; -- Unfolded here as gcc doesn't like unnamed structs. */
    DIFS_RECORD_TYPES magic;/* Magic is "INDX". */
    u16 usa_ofs;        /* See DIFS_RECORD definition. */
    u16 usa_count;      /* See DIFS_RECORD definition. */

/* 16 */    s64 lsn;        /* $LogFile sequence number of the last
                   modification of this index block. */
/* 24*/ VCN index_block_vcn;    /* Virtual cluster number of the index block.
                   index_block_vcn counts in units of  units of ALLOCATION . */
/*25*/  u8              filename_len_index ;/*the index of filename len in 2 power style, 0 to 8*/
/* 57*/ INDEX_HEADER    index;  /* Describes the following index entries. */
/* sizeof()= 40 (0x28) bytes */
/*
 * When creating the index block, we place the update sequence array at this
 * offset, i.e. before we start with the index entries. This also makes sense,
 * otherwise we could run into problems with the update sequence array
 * containing in itself the last two bytes of a sector which would mean that
 * multi sector transfer protection wouldn't work. As you can't protect data
 * by overwriting it since you then can't get it back...
 * When reading use the data from the DIFS record header.
 */
}  INDEX_BLOCK, *PINDEX_BLOCK;

typedef INDEX_BLOCK INDEX_ALLOCATION;

/*
 * Index entry flags (16-bit).
 */
typedef enum 
{
    INDEX_ENTRY_NODE = const_cpu_to_le16(1), /* This entry contains a sub-node,
                          i.e. a reference to an index   block in form of a virtual
                          cluster number (see below). */
    INDEX_ENTRY_END  = const_cpu_to_le16(2), /* This signifies the last entry in
                          an index block. The index entry   does not represent a file but it
                          can point to a sub-node. */
    INDEX_ENTRY_SPACE_FILLER = 0xffff, /* Just to force 16-bit width. */
}  INDEX_ENTRY_FLAGS;

/*
 * This the index entry header (see below).
 */
typedef struct 
{
/*  0*/ 
    union 
    {
        struct 
        { /* Only valid when INDEX_ENTRY_END is not set. */
            MFT_REF indexed_file;   /* The mft reference of the file described by this index
                           entry. Used for directory indexes. */
        }  dir;
        struct 
        { /* Used for views/indexes to find the entry's data. */
            u16 data_offset;    /* Data byte offset from this INDEX_ENTRY. Follows the  index key. */
            u16 data_length;    /* Data length in bytes. */
            ul32 reservedV;     /* Reserved (zero). */
        }  vi;
    }  data;
/*  8*/ u16 length;      /* Byte size of this index entry, multiple of 8-bytes. */
/* 10*/ u16 key_length;      /* Byte size of the key value, which is in the
                    index entry. It follows field reserved. Not multiple of 8-bytes. */
/* 12*/ u16  flags; /* Bit field of INDEX_ENTRY_* flags. */
/* 14*/ u16 reserved;        /* Reserved/align to 8-byte boundary. */
/* sizeof() = 16 bytes */
}  INDEX_ENTRY_HEADER, *PINDEX_ENTRY_HEADER;

/*
 * This is an index entry. A sequence of such entries follows each INDEX_HEADER
 * structure. Together they make up a complete index. The index follows either
 * an index root attribute or an index allocation attribute.
 *
 * NOTE: Before DIFS 3.0 only filename attributes were indexed.
 * 16 + 592 + 8 = 716 bytes
 */
typedef struct 
{
/*Ofs*/
/*  0   INDEX_ENTRY_HEADER; -- Unfolded here as gcc dislikes unnamed structs. */
    union 
    {
        struct 
        { /* Only valid when INDEX_ENTRY_END is not set. */
            MFT_REF indexed_file;   /* The mft reference of the file  described by this index
                           entry. Used for directory   indexes. */
        }  dir;
        struct 
        { /* Used for views/indexes to find the entry's data. */
            u16 data_offset;    /* Data byte offset from this  INDEX_ENTRY. Follows the  index key. */
            u16 data_length;    /* Data length in bytes. */
            ul32 reservedV;     /* Reserved (zero). */
        }  vi;
    }  data;
    u16 length;      /* Byte size of this index entry, multiple of    8-bytes. */
    u16 key_length;      /* Byte size of the key value, which is in the
                    index entry. It follows field reserved. Not    multiple of 8-bytes. */
    u16  flags; /* Bit field of INDEX_ENTRY_* flags. */
    ul32 entry_insert_index;
    u16 reserved;        /* Reserved/align to 8-byte boundary. */

/* 16*/ 
    union 
    {       /* The key of the indexed attribute. NOTE: Only present
               if INDEX_ENTRY_END bit in flags is not set. */
        FILE_NAME_ATTR file_name;/* $I30 index in directories. 336 bytes*/
        SII_INDEX_KEY sii;  /* $SII index in $Secure.4 */
        SDH_INDEX_KEY sdh;  /* $SDH index in $Secure. 8*/
        GUID object_id;     /* $O index in FILE_Extend/$ObjId: The
                       object_id of the mft record found in the data part of the index. 16*/
        DIFS_REPARSE_INDEX_KEY reparse;  /* $R index in  FILE_Extend/$Reparse.12 */
        SID sid;        /* $O index in FILE_Extend/$Quota: SID of the owner of the user_id.12 */
        ul32 owner_id;      /* $Q index in FILE_Extend/$Quota:  user_id of the owner of the quota
                       control entry in the data part of the index. 4*/
    }  key;
    /* The (optional) index data is inserted here when creating. */
    VCN Child_IndxBlock_Vcn;    /* If INDEX_ENTRY_NODE bit in flags is set, the last
    //         eight bytes of this index entry contain the virtual
    //         cluster number of the index block that holds the
    //         entries immediately preceding the current entry (the
    //         vcn references the corresponding cluster in the data
    //         of the non-resident index allocation attribute). If
    //         the key_length is zero, then the vcn immediately
    //         follows the INDEX_ENTRY_HEADER. Regardless of
    //         key_length, the address of the 8-byte boundary
    //         alligned vcn of INDEX_ENTRY{_HEADER} *ie is given by
    //         (char*)ie + le16_to_cpu(ie*)->length) - sizeof(VCN),
    //         where sizeof(VCN) can be hardcoded as 8 if wanted. */
}  INDEX_ENTRY, *PINDEX_ENTRY;

/*
 * Attribute: Bitmap (0xb0).
 *
 * Contains an array of bits (aka a bitfield).
 *
 * When used in conjunction with the index allocation attribute, each bit
 * corresponds to one index block within the index allocation attribute. Thus
 * the number of bits in the bitmap * index block size / cluster size is the
 * number of clusters in the index allocation attribute.
 */
typedef struct 
{
    Run_list    runlist ;
    int         bitmapLength;
    u8 *        bitmap;         /* Array of bits. */
}  BITMAP_ATTR, *PBITMAP_ATTR;

/*
 * Attribute: Reparse point (0xc0).
 *
 * NOTE: Can be resident or non-resident.
 */
typedef struct 
{
    ul32 reparse_tag;       /* Reparse point type (inc. flags). */
    u16 reparse_data_length;    /* Byte size of reparse data. */
    u16 reserved;           /* Align to 8-byte boundary. */
    u8  * reparse_data;     /* Meaning depends on reparse_tag. */
}  REPARSE_POINT;

/*
 * Attribute: Extended attribute (EA) information (0xd0).
 *
 * NOTE: Always resident. (Is this true???)
 */
typedef struct 
{
    u16 ea_length;      /* Byte size of the packed extended
                   attributes. */
    u16 need_ea_count;  /* The number of extended attributes which have
                   the NEED_EA bit set. */
    ul32 ea_query_length;   /* Byte size of the buffer required to query
                   the extended attributes when calling
                   ZwQueryEaFile() in Windows NT/2k. I.e. the
                   byte size of the unpacked extended
                   attributes. */
}  EA_INFORMATION;

/*
 * Extended attribute flags (8-bit).
 */
typedef enum 
{
    NEED_EA = 0x80,
}  EA_FLAGS;

/*
 * Attribute: Extended attribute (EA) (0xe0).
 *
 * NOTE: Always non-resident. (Is this true?)
 * 
 * Like the attribute list and the index buffer list, the EA attribute value is 
 * a sequence of EA_ATTR variable length records.
 *
 * FIXME: It appears weird that the EA name is not unicode. Is it true?
 */
typedef struct
{
    ul32        next_entry_offset;  /* Offset to the next EA_ATTR. */
    u8          flags;      /* Flags describing the EA. */
    u8          ea_name_length; /* Length of the name of the EA in bytes. */
    u16         ea_value_length;    /* Byte size of the EA's value. */
    u8 *        ea_name;        /* Name of the EA. */
    u8 *        ea_value;       /* The value of the EA. Immediately follows
                   the name. */
}  EA_ATTR;

//definition for DISK IDLE SPACE DESCRIPTION in $Bitmap

typedef struct
{
    LCN     i64StartLCN ;
    u64     i64Length ; //in CLUSTER unit
}Disk_Area  , * PDisk_Area ;

typedef struct _DiskAreaLogicDesc
{
    VCN         StartVcnInAlloc ; //start VCN of this Disk Area bmp Desc in space_allocation 
    u32         uType1NodeNum ;//the number  this disk area's TYPE1 node, from the startvcn ,the TYPE1 NODE WILL BE write
    Disk_Area   PhyDiskAreaDesc ;
}DiskAreaLogicDesc , *PDiskAreaLogicDesc ;
/*
 * Attribute: IDLE SPACE Index root AT_SPACE_IDX_ROOT.
 *
 * NOTE: Always resident.
 *
 * This is followed by a sequence of index entries (SPACE_IDX_ENTRY structures)
 * as described by the space index header.
 *
 * an SPACE index allocation attribute, containing array of space_idx_entry ;
 * bitmap attribute, describing which virtual
 * cluster numbers (vcns) in the space index allocation has free space_idx_entry.
 *
 */
typedef struct 
{
    ul32                    space_idx_block_size;       /* Size of each index block in bytes (in the space index allocation attribute). */
    u8                      sectors_per_index_block;    
    u8                      uDiskAreaNum ; 
    PDiskAreaLogicDesc      pDiskAreaLogicDescList ;
    //follow the DiskAreaLogicDesc list, disk area must be sort by ascending LCN
}SPACE_IDX_ROOT, *PSPACE_IDX_ROOT;

typedef enum
{
    DataArea_Idle       =   0x0000 ,
    DataArea_BootRecord =   0x0001,
    DataArea_SMFT       =   0x0002,
    DataArea_DMFT       =   0x0003,
    DataArea_FileData   =   0x0004,
    DataArea_Log        =   0x0005,

}DataAreaType ;

typedef struct _DataAreaEntry
{
    u16     DataAreaType;
    u64     i64StartLCN ;
    u64     i64Length ;
}DataAreaEntry  , * PDataAreaEntry ;

typedef struct _DataAreaListAttr
{
    u16     uDataAreaEntryNum;
    PDataAreaEntry  pDataAreaEntry; //this is a indication to the list of dataarealist
}DataAreaListAttr, *PDataAreaListAttr;

typedef struct _DataVolIdAttr
{
    u32     uDataVolId ;
}DataVolIdAttr , *PDataVolIdAttr;

//  AT_DVOLDIR_ID   
typedef struct _DataVolDirId
{
    u16     uItemNumber; //note: when create svcb dir , this number equal the current total datavol number
    u32     DataDirMftIdList[DIFS_MAX_DATAVOL_NUM];

}DataVolDirId, *PDataVolDirId ;

//  AT_SVOLDIR_ID       
typedef struct _SysVolDirId
{
    u32     uSysDirMftId ;
}SysVolDirId, *PSysVolDirId ;

typedef enum
{
    AllocNode_UndefType = 0,
    AllocNode_Type1 = 1 , //every alloc element (32bit)==> 36G BYTE,BmpEntry_offset = 1KB ,BmpEntryDesc_offset = 2.5KB,BmpEntry_number = 384
    AllocNode_Type2 = 2 ,//every alloc element (32bit)==> 96M BYTE,BmpEntry_offset = 1KB ,BmpEntryDesc_offset = 2.5KB,BmpEntry_number = 384
    AllocNode_Type3 = 3 ,//every alloc element (1bit)==> 4kB == 1 cluster,BmpEntry_offset = 1KB ,no BmpEntryDesc BmpEntryDesc_offset = 4KB ,BmpEntry_number = 3K*8 = 24K
}AllocNodeType;

//space_alloc_record_type2 
typedef struct 
{
/*  8   DIFS_RECORD; -- Unfolded here as gcc doesn't like unnamed structs. */
    DIFS_RECORD_TYPES       magic;/* Magic is "SIDX". */
    u16                     usa_ofs;        /* See DIFS_RECORD definition. */
    u16                     usa_count;      /* See DIFS_RECORD definition. */

/* 16*/ s64                 lsn;        /* $LogFile sequence number of the last modification of this index block. */
/* 20*/ u32                 DiskAreaIdx ;//the diskarea index this allocbmp belong to
/* 28*/ u32                 vcn;  /* Virtual cluster number of the index block.index_block_vcn counts in units of allocation_block . */
/* 32*/ u32                 Parent_vcn;
/* 36*/ u32                 GrandParent_vcn;
/* 40*/ AllocNodeType       RecordType ;
/* 44*/ u32                 Entry_offset;        /* Byte offset to first bitmap entry aligned to 32-byte boundary. */
/* 48*/ u32                 EntryDesc_offset;    /* Byte offset to first bitmap entry description aligned to 32-byte boundary. */
/* 52*/ u32                 Entry_number;        /* number of entry in this record */
/* 60*/ LCN                 StartLCN; //the physical disk area start pos this record described
/* 68*/ LCN                 Length ;//the physical disk area length this record described
/* 72*/ u32                 EntryClusterNum ;//to indicate how many cluster every entry describe
/* 76*/ u32                 LastEntryValidClusterNum ;//to indicate how many cluster last entry describe , for the sake of non-enough disk to use
/* 80*/ u32                 EntryIdxInParent ;//this entry idx in parent of the record , 0xfffffff for type1 node.
/* 84*/ u64                 sequence_number;

}  SPACE_IDX_RECORD_TYPE2, *PSPACE_IDX_RECORD_TYPE2;

#pragma pack(pop)

#endif /* _WIN_DIFS_LAYOUT_H */

