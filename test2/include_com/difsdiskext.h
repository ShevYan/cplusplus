#pragma once
#ifndef _FILE_DISK_EXT_
#define _FILE_DISK_EXT_

#ifdef _DIFS_DISK_SYS_
	typedef unsigned char       u8 ;
	typedef unsigned short      u16 ;
	typedef unsigned long       ul32 ;
	typedef unsigned long       u32;
	typedef unsigned __int64    u64 ;
	
	typedef char        s8 ;
	typedef short       s16 ;
	typedef int         s32 ;
	typedef __int64     s64 ;
#endif

#pragma pack(push, 1)

/*
 * BIOS parameter block (bpb) structure.
 */
typedef struct 
{
    u16     bytes_per_sector;       /* Size of a sector in bytes. */
    u16     sectors_per_cluster;    /* Size of a cluster in sectors. */
    u64     sectors;            /* total sector number on this partition */
    u8      media_type;         /* 0xf8 = hard disk */
    u16     sectors_per_track;      /* irrelevant */
    u16     heads;          /* irrelevant */
    u32     bytes_per_filerecord ;
    u32     reserved1 ;
    u64     reserved2 ;
    u64     reserved3 ;
    u64     reserved4 ;
}  BIOS_PARAMETER_BLOCK, *PBIOS_PARAMETER_BLOCK;

/*
 * DIFS boot sector structure.
 */
typedef struct 
{
    u8      jump[3];            /* Irrelevant (jump to boot up code).*/
    u64     oem_id;         /* Magic "DIFS    ". */
    BIOS_PARAMETER_BLOCK bpb;   /* See BIOS_PARAMETER_BLOCK. */
    u8      unused[4];          /* zero, DIFS diskedit.exe states that
                       this is actually:
                        __u8 physical_drive;    // 0x80
                        __u8 current_head;  // zero
                        __u8 extended_boot_signature;
                                    // 0x80
                        __u8 unused;        // zero
                     */
    s64     number_of_sectors;      /* Number of sectors in volume. Gives
                       maximum volume size of 2^63 sectors.
                       Assuming standard sector size of 512
                       bytes, the maximum byte size is
                       approx. 4.7x10^21 bytes. (-; */
    s64     smft_lbn;           /* sector location of smft data. logic block number*/
    s64     smftmirr_lbn;       /* sector location of copy of smft.logic block number */
    s64     loglist_lbn;        /* sector location of loglist*/
    u32     loglist_len;        /*the size of loglist array*/
    s8      sectors_per_mft_record;    /* Mft record size in sectors. */
    u8      reserved0[3];       /* zero */
    s8      sectors_per_index_record;  /* Index block size in clusters. */
    u16     formatversion;   /* v1.0 = 0x10*/
    u8      reserved1;       /* zero */
    u32     volume_serial_number;   /* Irrelevant (serial number). */
    ul32    checksum;           /* Boot sector checksum. */
    u64     difs_formattime;//The time of the DIFS psrtition format time.
    u8      bootstrap[386];     /* Irrelevant (boot up code). */
    u16     end_of_sector_marker;   /* End of bootsector magic. Always is  0xaa55 in little endian. */
/* sizeof() = 512 (0x200) bytes */
}  DIFS_BOOT_SECTOR, *PDIFS_BOOT_SECTOR;

/*
 * DIFS boot sector structure.
 */
typedef struct 
{
    u8      jump[3];            /* Irrelevant (jump to boot up code).*/
    u64     oem_id;         /* Magic "DIFS    ". */
    BIOS_PARAMETER_BLOCK bpb;   /* See BIOS_PARAMETER_BLOCK. */
    u8      unused[4];          /* zero, DIFS diskedit.exe states that
                       this is actually:
                        __u8 physical_drive;    // 0x80
                        __u8 current_head;  // zero
                        __u8 extended_boot_signature;
                                    // 0x80
                        __u8 unused;        // zero
                     */
    s64     number_of_sectors;      /* Number of sectors in volume. Gives
                       maximum volume size of 2^63 sectors.
                       Assuming standard sector size of 512
                       bytes, the maximum byte size is
                       approx. 4.7x10^21 bytes. (-; */
    s64     smft_lbn;           /* sector location of smft data. logic block number*/
    s64     smftmirr_lbn;       /* sector location of copy of smft.logic block number */
    s64     loglist_lbn;        /* sector location of loglist*/
    u32     loglist_len;        /*the size of loglist array*/
    s8      sectors_per_mft_record;    /* Mft record size in sectors. */
    u8      reserved0[3];       /* zero */
    s8      sectors_per_index_record;  /* Index block size in clusters. */
    u16     formatVersion;   /* v1.0 = 0x10*/
    u8      reserved1;       /* zero */
    u32     volume_serial_number;   /* Irrelevant (serial number). */
    ul32    checksum;           /* Boot sector checksum. */
    u64     difs_formattime;//The time of the DIFS psrtition format time.
//  u8      bootstrap[386];     /* Irrelevant (boot up code). */
//  u16     end_of_sector_marker;   /* End of bootsector magic. Always is  0xaa55 in little endian. */
/* sizeof() = 512 (0x200) bytes */
}  DIFS_BOOT_SECTOR_SHORT, *PDIFS_BOOT_SECTOR_SHORT;

#pragma pack(pop)
#endif

