#ifndef _FSMETACMDDEF_H_
#define _FSMETACMDDEF_H_

#include "layout.h"
#include "metacmdbase.h"
#include "difstypes.h"
#include "dblinklist.h"
#include "W2KWrapBase.h"
#include "../mds/metadb/diskmetadata.h"

#ifndef _AMD64_              //32 bit driver and application
//#define     PRun_List_Element_DEF(pVar)      PRun_List_Element    pVar;ULONG pVar##_placeholder
#define     PRun_List_Element_DEF(pVar)       union{PRun_List_Element    pVar;ULONGLONG pVar##_placeholder;}
#else                           //64 bit driver and application
#define     PRun_List_Element_DEF(pVar)      PRun_List_Element    pVar
#endif

#pragma warning(disable:4200)
#pragma pack(push,1)
//ssm state

#define     SSM_STATE_MAIN                  0
#define     SSM_STATE_SUB                   1
#define     SSM_STATE_SWITCH                2
#define     SSM_STATE_VOTING                3
#define     SSM_STATE_STARTING              4
#define		SSM_STATE_DISKERR				5
#define     SSM_STATE_ERROR                 100

//net
#define     NETWORK_ERROR                   0xFFFFFFFF

//subclass define
#define     MCMD_DIFS_FSD2SCM               0
#define     MCMD_DIFS_SCM2SSM               1
#define     MCMD_DIFS_SCM2FSD               2
#define     MCMD_DIFS_SSM2SCM               3

//number for backup vols
//must Align to 8
#define     MBD_BACKUP_MAX_NUMB             8

//meta_cmd definition for difs

#define     MCMD_NTF_SVR2FSD_REGISTER               MAKE_METACMD(MCMD_CLASS_DIFS,0)
//��ʾ�����ļ�ϵͳ׼�������ͶϿ�SAN SERVER�Ĺ���    
#define     MCMD_REQ_SCM2SSM_FS_MOUNT               MAKE_METACMD(MCMD_CLASS_DIFS,1)
#define     MCMD_REQ_SSM2SCM_FS_UNMOUNT             MAKE_METACMD(MCMD_CLASS_DIFS,2)
#define     MCMD_NTF_SCM2SSM_FS_UNMOUNT             MAKE_METACMD(MCMD_CLASS_DIFS,3)

#define     MCMD_REQ_SSM2FSD_FS_MOUNT               MAKE_METACMD(MCMD_CLASS_DIFS,4)
#define     MCMD_REQ_SCM2FSD_FS_MOUNT               MAKE_METACMD(MCMD_CLASS_DIFS,5)

#define     MCMD_REQ_SSM2FSD_FS_UNMOUNT             MAKE_METACMD(MCMD_CLASS_DIFS,6)
#define     MCMD_REQ_SCM2FSD_FS_UNMOUNT             MAKE_METACMD(MCMD_CLASS_DIFS,7)

#define     MCMD_NTY_FSD2SSM_FS_STATUS              MAKE_METACMD(MCMD_CLASS_DIFS,8)
#define     MCMD_NTY_FSD2SCM_FS_STATUS              MAKE_METACMD(MCMD_CLASS_DIFS,9)
                                                    
#define     MCMD_REQ_SSM2FSD_FS_READANDWRITE2FILE   MAKE_METACMD(MCMD_CLASS_DIFS,10)

//���ļ�ϵͳ�������ص�Ԫ����
/*
//��ѯ�ļ�ϵͳ������Ϣ��
#define     MCMD_QRY_FSD2SCM_FS_CTRLINFO            MAKE_METACMD(MCMD_CLASS_DIFS,5) 
#define     MCMD_QRY_SCM2SSM_FS_CTRLINFO            MAKE_METACMD(MCMD_CLASS_DIFS,6) 
*/
//��ѯ�ļ�ϵͳ��ϸ��Ϣ��
#define     MCMD_QRY_FSD2SCM_FS_FULLSIZEINFO        MAKE_METACMD(MCMD_CLASS_DIFS,1008)         
#define     MCMD_QRY_SCM2SSM_FS_FULLSIZEINFO        MAKE_METACMD(MCMD_CLASS_DIFS,1009)         
//��ѯ�ļ�ϵͳ��Ϣ��
#define     MCMD_QRY_FSD2SCM_FS_SIZEINFO            MAKE_METACMD(MCMD_CLASS_DIFS,11)            
#define     MCMD_QRY_SCM2SSM_FS_SIZEINFO            MAKE_METACMD(MCMD_CLASS_DIFS,12)            

//���ļ���Ŀ¼���ص�Ԫ����
//�����ļ���Ŀ¼������
#define     MCMD_REQ_FSD2SCM_CREATE                 MAKE_METACMD(MCMD_CLASS_DIFS,14)    
#define     MCMD_REQ_SCM2SSM_CREATE                 MAKE_METACMD(MCMD_CLASS_DIFS,15)    

//�ر��ļ���Ŀ¼������
#define     MCMD_REQ_FSD2SCM_CLEANUP                MAKE_METACMD(MCMD_CLASS_DIFS,17)    
#define     MCMD_REQ_SCM2SSM_CLEANUP                MAKE_METACMD(MCMD_CLASS_DIFS,18)    
#define     MCMD_REQ_FSD2SCM_CLOSE                  MAKE_METACMD(MCMD_CLASS_DIFS,19)    
#define     MCMD_REQ_SCM2SSM_CLOSE                  MAKE_METACMD(MCMD_CLASS_DIFS,20)    
//ɾ���ļ���Ŀ¼������

//��ѯ�ļ���Ŀ¼������Ϣ��
#define     MCMD_QRY_FSD2SCM_BASICINFO              MAKE_METACMD(MCMD_CLASS_DIFS,21)
#define     MCMD_QRY_SCM2SSM_BASICINFO              MAKE_METACMD(MCMD_CLASS_DIFS,22)
//��ѯ�ļ���Ŀ¼��׼��Ϣ��
#define     MCMD_QRY_FSD2SCM_STDINFO                MAKE_METACMD(MCMD_CLASS_DIFS,23)
#define     MCMD_QRY_SCM2SSM_STDINFO                MAKE_METACMD(MCMD_CLASS_DIFS,24)
//�����ļ���Ŀ¼������Ϣ��
#define     MCMD_REQ_FSD2SCM_SET_BASIC_INFO         MAKE_METACMD(MCMD_CLASS_DIFS,25)    
#define     MCMD_REQ_SCM2SSM_SET_BASIC_INFO         MAKE_METACMD(MCMD_CLASS_DIFS,26)    
//�����ļ���Ŀ¼���ƣ�
#define     MCMD_REQ_FSD2SCM_SET_FILENAME           MAKE_METACMD(MCMD_CLASS_DIFS,27)    
#define     MCMD_REQ_SCM2SSM_SET_FILENAME           MAKE_METACMD(MCMD_CLASS_DIFS,28)    
//�޸��ļ���Ŀ¼���ƣ�
#define     MCMD_REQ_FSD2SCM_RENAME_FILENAME        MAKE_METACMD(MCMD_CLASS_DIFS,29)    
#define     MCMD_REQ_SCM2SSM_RENAME_FILENAME        MAKE_METACMD(MCMD_CLASS_DIFS,30)    
#define     MCMD_REQ_SSM2FSD_RENAME_FILENAME        MAKE_METACMD(MCMD_CLASS_DIFS,31)    

//��ѯ�����к�
#define     MCMD_QRY_SERIAL_NUMBER                  MAKE_METACMD(MCMD_CLASS_DIFS,32)

//���¸�Ŀ¼SSMID
#define     MCMD_REQ_SCM2FSD_UPDATASSMID            MAKE_METACMD(MCMD_CLASS_DIFS,33)

//ֻ���ļ����ص�Ԫ����
//�ļ�����
#define     MCMD_REQ_FSD2SCM_FILE_LOCK                  MAKE_METACMD(MCMD_CLASS_DIFS,34)    
#define     MCMD_REQ_SCM2SSM_FILE_LOCK                  MAKE_METACMD(MCMD_CLASS_DIFS,35)    
//�ļ�������
#define     MCMD_REQ_FSD2SCM_FILE_UNLOCK                MAKE_METACMD(MCMD_CLASS_DIFS,36)    
#define     MCMD_REQ_SCM2SSM_FILE_UNLOCK                MAKE_METACMD(MCMD_CLASS_DIFS,37)    
//FLUSH������ 
#define     MCMD_REQ_FSD2SCM_FILE_FLUSH                 MAKE_METACMD(MCMD_CLASS_DIFS,38)    
#define     MCMD_REQ_SCM2SSM_FILE_FLUSH                 MAKE_METACMD(MCMD_CLASS_DIFS,39)    
//���ʱ����ȡ�ļ�λ�õĴ��̷�����Ϣ��
#define     MCMD_QRY_FSD2SCM_FILE_READ_ALLOCINFO        MAKE_METACMD(MCMD_CLASS_DIFS,40)    
#define     MCMD_QRY_SCM2SSM_FILE_READ_ALLOCINFO        MAKE_METACMD(MCMD_CLASS_DIFS,41) 
#define     MCMD_QRY_SSM2FSD_FILE_READ_ALLOCINFO        MAKE_METACMD(MCMD_CLASS_DIFS,42)    
//��ѯ�ļ���������Ϣ��
#define     MCMD_QRY_FSD2SCM_FILE_STRMINFO              MAKE_METACMD(MCMD_CLASS_DIFS,43)    
#define     MCMD_QRY_SCM2SSM_FILE_STRMINFO              MAKE_METACMD(MCMD_CLASS_DIFS,44)    
//�����ļ�������Ϣ��
#define     MCMD_REQ_FSD2SCM_SET_FILE_EOF               MAKE_METACMD(MCMD_CLASS_DIFS,46)    
#define     MCMD_REQ_SCM2SSM_SET_FILE_EOF               MAKE_METACMD(MCMD_CLASS_DIFS,47)    
#define     MCMD_REQ_SSM2FSD_SET_FILE_EOF               MAKE_METACMD(MCMD_CLASS_DIFS,48)    
//�����ļ��Ƿ�ɾ��
#define     MCMD_REQ_FSD2SCM_SET_FILE_DELETE            MAKE_METACMD(MCMD_CLASS_DIFS,49)    
#define     MCMD_REQ_SCM2SSM_SET_FILE_DELETE            MAKE_METACMD(MCMD_CLASS_DIFS,50)    
#define     MCMD_REQ_SSM2FSD_SET_FILE_DELETE            MAKE_METACMD(MCMD_CLASS_DIFS,51)    
//�����ļ���Ч���ȣ�
#define     MCMD_REQ_FSD2SCM_SET_FILE_VALID_LEN         MAKE_METACMD(MCMD_CLASS_DIFS,52)    
#define     MCMD_REQ_SCM2SSM_SET_FILE_VALID_LEN         MAKE_METACMD(MCMD_CLASS_DIFS,53)    
#define     MCMD_REQ_SSM2FSD_SET_FILE_VALID_LEN         MAKE_METACMD(MCMD_CLASS_DIFS,54)    
//�����ļ��Ĵ��̷����ռ䣺
#define     MCMD_REQ_FSD2SCM_SET_FILE_ALLOCSIZE         MAKE_METACMD(MCMD_CLASS_DIFS,55)    
#define     MCMD_REQ_SCM2SSM_SET_FILE_ALLOCSIZE         MAKE_METACMD(MCMD_CLASS_DIFS,56)    
#define     MCMD_REQ_SSM2FSD_SET_FILE_ALLOCSIZE         MAKE_METACMD(MCMD_CLASS_DIFS,57)    
//ֻ��Ŀ¼�йص�Ԫ���
//��ѯĿ¼�µ���Ϣ��
#define     MCMD_QRY_FSD2SCM_DIR_INFO                   MAKE_METACMD(MCMD_CLASS_DIFS,58)    
#define     MCMD_QRY_SCM2SSM_DIR_INFO                   MAKE_METACMD(MCMD_CLASS_DIFS,59)    
#define     MCMD_QRY_SSM2FSD_DIR_INFO                   MAKE_METACMD(MCMD_CLASS_DIFS,60)    
//��ѯĿ¼���ݵĸı䣺
#define     MCMD_QRY_FSD2SCM_DIR_NOTIFY_CHANGE          MAKE_METACMD(MCMD_CLASS_DIFS,70)    
#define     MCMD_QRY_SCM2SSM_DIR_NOTIFY_CHANGE          MAKE_METACMD(MCMD_CLASS_DIFS,71)    
#define     MCMD_QRY_SSM2FSD_DIR_NOTIFY_CHANGE          MAKE_METACMD(MCMD_CLASS_DIFS,72)    

//SCM���͵�SCMDע����
#define     MCMD_REQ_SCM2SCMD_REGISTER_VOLUME           MAKE_METACMD(MCMD_CLASS_DIFS,73)    
#define     MCMD_REQ_SCM2SCMD_UNREGISTER_VOLUME         MAKE_METACMD(MCMD_CLASS_DIFS,74)    
#define     MCMD_REQ_SCM2SCMD_GETMETACMD                MAKE_METACMD(MCMD_CLASS_DIFS,75)    
#define     MCMD_RESP_SCM2SCMD_METACMD                  MAKE_METACMD(MCMD_CLASS_DIFS,76)    

//������Ϣ��
#define     MCMD_SET_FSD2SCM_INFORMATION                MAKE_METACMD(MCMD_CLASS_DIFS,80)    
#define     MCMD_SET_SCM2SSM_INFORMATION                MAKE_METACMD(MCMD_CLASS_DIFS,81)    
#define     MCMD_SET_SSM2FSD_INFORMATION                MAKE_METACMD(MCMD_CLASS_DIFS,82)    

//SSM���͵�fsd�������Ƿ���Ч
#define     MCMD_REQ_SSM2FSD_CHECKVOLUME                MAKE_METACMD(MCMD_CLASS_DIFS,83)    

//SSM���͵�fsd������Ҫ��ر��ļ�������
#define     MCMD_REQ_SSM2FSD_PRECLOSE_HANDLE            MAKE_METACMD(MCMD_CLASS_DIFS, 84)

//scm֪ͨ�ͻ���fsd�Ͽ���Ӻ����½�����
#define     MCMD_REQ_SCM2FSD_DISCONNECT_SSM             MAKE_METACMD(MCMD_CLASS_DIFS, 85)
#define     MCMD_REQ_SCM2FSD_CONNECT_SSM                MAKE_METACMD(MCMD_CLASS_DIFS, 86)

#define     MCMD_SET_FSD2SCM_VOLUME_INFORMATION                MAKE_METACMD(MCMD_CLASS_DIFS,90)    
#define     MCMD_SET_SCM2SSM_VOLUME_INFORMATION                MAKE_METACMD(MCMD_CLASS_DIFS,91)    
#define     MCMD_SET_SSM2FSD_VOLUME_INFORMATION                MAKE_METACMD(MCMD_CLASS_DIFS,92)    

#define     MCMD_QRY_FSD2SCM_VOLUME_INFORMATION                MAKE_METACMD(MCMD_CLASS_DIFS,93)    
#define     MCMD_QRY_SCM2SSM_VOLUME_INFORMATION                MAKE_METACMD(MCMD_CLASS_DIFS,94)    
#define     MCMD_QRY_SSM2FSD_VOLUME_INFORMATION                MAKE_METACMD(MCMD_CLASS_DIFS,95)    

//SSM��FSD�ı��ļ�ALLOCATION_SIZE��FILE_SIZE��VALIDDATA_SIZE��Ԫ����
#define     MCMD_SET_SSM2FSD_SET_SIZEINFO                      MAKE_METACMD(MCMD_CLASS_DIFS,98)    

//�����л�ʱ��SCM���͵��µ���SSM�ĸþ�����Ϣ
#define     MCMD_INFO_SCM2SSM_VOLUME                    MAKE_METACMD(MCMD_CLASS_DIFS,100)    

#define     MCMD_INFO_SCM2SSM_CREATEFILE                MAKE_METACMD(MCMD_CLASS_DIFS,101)    
//SSM���͵�SCM�����ʼ�л�ָ��
#define     MCMD_REQ_SSM2SCM_MAINSWITCH_START           MAKE_METACMD(MCMD_CLASS_DIFS,102)
//SSM���͵�SCM�����������л�ָ��
#define     MCMD_REQ_SSM2SCM_MAINSWITCH_FINISH          MAKE_METACMD(MCMD_CLASS_DIFS,103)
//SSM���͵�SCM,֪ͨscm�����Ѿ�����
#define     MCMD_REQ_SSM2SCM_SUBSSMSTART                MAKE_METACMD(MCMD_CLASS_DIFS,104)
#define     MCMD_REQ_SSM2SCM_ERRORCREATE                MAKE_METACMD(MCMD_CLASS_DIFS,105)

//SSM���͵�ͶƱ������SCM
#define     MCMD_REQ_SSM2SCM_VOTESWITCH                 MAKE_METACMD(MCMD_CLASS_DIFS,106)

//SSM���͵�SCM�������л�ʧ��ָ��
#define     MCMD_REQ_SSM2SCM_MAINSWITCH_FAIL            MAKE_METACMD(MCMD_CLASS_DIFS,107)

//SSM����ĳ���ͻ����ļ��Ļ���������
#define     MCMD_REQ_SSM2SCM_OPLOCK_CHANGE              MAKE_METACMD(MCMD_CLASS_DIFS,200)
#define     MCMD_REQ_SCM2FSD_OPLOCK_CHANGE              MAKE_METACMD(MCMD_CLASS_DIFS,201)

//�޸�CACHEģʽ
#define     MCMD_REQ_SSM2SCM_CACHEMODE_CHANGE           MAKE_METACMD(MCMD_CLASS_DIFS,301)
#define     MCMD_REQ_SCM2FSD_CACHEMODE_CHANGE           MAKE_METACMD(MCMD_CLASS_DIFS,302)

//�޸�FILE ACCESSģʽ
#define     MCMD_REQ_APP2FSD_ACCESSMODE_CHANGE          MAKE_METACMD(MCMD_CLASS_DIFS,303)   
#define     MCMD_REQ_FSD2SCM_ACCESSMODE_CHANGE          MAKE_METACMD(MCMD_CLASS_DIFS,304)   
#define     MCMD_REQ_SCM2SSM_ACCESSMODE_CHANGE          MAKE_METACMD(MCMD_CLASS_DIFS,305)   

//SSM֪ͨ�ͻ���FSD�ļ��Ѿ�������Ϊɾ��
#define     MCMD_REQ_SSM2SCM_FILEDELETE                 MAKE_METACMD(MCMD_CLASS_DIFS,306)
#define     MCMD_REQ_SCM2FSD_FILEDELETE                 MAKE_METACMD(MCMD_CLASS_DIFS,307)

//SSM֪ͨfsd�ļ���deleteonclose��ʶ
#define     MCMD_SET_SSM2FSD_FILEDELETEONCLOSE          MAKE_METACMD(MCMD_CLASS_DIFS,350)

//fsld to fsd
#define     MCMD_REQ_FSD2FSLD_REGISTER                  MAKE_METACMD(MCMD_CLASS_DIFS,400)
#define     MCMD_REQ_FSD2FSLD_UNREGISTER                MAKE_METACMD(MCMD_CLASS_DIFS,401)
#define     MCMD_REQ_FSD2FSLD_SENDDATA                  MAKE_METACMD(MCMD_CLASS_DIFS,402)
#define     MCMD_REQ_FSLD2FSD_SENDDATA                  MAKE_METACMD(MCMD_CLASS_DIFS,403)
#define     MCMD_REQ_FSD2FSLD_ACTIVE                    MAKE_METACMD(MCMD_CLASS_DIFS,404)
#define     MCMD_REQ_FSD2FSLD_DEACTIVE                  MAKE_METACMD(MCMD_CLASS_DIFS,405)
#define     MCMD_REQ_FSD2FSLD_START_CACHE               MAKE_METACMD(MCMD_CLASS_DIFS,406)
#define     MCMD_REQ_FSD2FSLD_STOP_CACHE                MAKE_METACMD(MCMD_CLASS_DIFS,407)
#define     MCMD_REQ_FSD2FSLD_READ_DATA                 MAKE_METACMD(MCMD_CLASS_DIFS,408)

#define     MCMD_REQ_FSD2SCMD_FS_REGISTER               MAKE_METACMD(MCMD_CLASS_DIFS,500)
#define     MCMD_REQ_FSD2SCMD_FS_UNREGISTER             MAKE_METACMD(MCMD_CLASS_DIFS,501)

//SSM֪ͨFSD��������״̬
#define     MCMD_REQ_SSM2FSD_DEACTIVE                   MAKE_METACMD(MCMD_CLASS_DIFS,600)
//SSM֪ͨFSD������������״̬
#define     MCMD_REQ_SSM2FSD_ACTIVE                     MAKE_METACMD(MCMD_CLASS_DIFS,601)
#define     MCMD_REQ_SSM2FSD_PREDEACTIVE                MAKE_METACMD(MCMD_CLASS_DIFS,602)

//SCM����SSM�����л�
#define     MCMD_REQ_SCM2SSM_SWITCH                     MAKE_METACMD(MCMD_CLASS_DIFS,610)

//fsd to backup
#define     MCMD_REQ_FSD2MBD_REGISTER                  MAKE_METACMD(MCMD_CLASS_DIFS,700)
#define     MCMD_REQ_FSD2MBD_UNREGISTER                MAKE_METACMD(MCMD_CLASS_DIFS,701)
#define     MCMD_REQ_FSD2MBD_SENDDATA                  MAKE_METACMD(MCMD_CLASS_DIFS,702)

//fsd to diskperf
#define     MCMD_REQ_FSD2DISKPERF_REGISTER                  MAKE_METACMD(MCMD_CLASS_DIFS,800)

#define     MCMD_REQ_SSM2MBD_FS_MOUNT               MAKE_METACMD(MCMD_CLASS_DIFS,900)

#define     MCMD_REQ_SSM2FSD_MARK_FILE_ALLOCSIZE         MAKE_METACMD(MCMD_CLASS_DIFS,950) 

// Dir Cache Change
#define     MCMD_REQ_SSM2SCM_DIRCACHE_CHANGE           MAKE_METACMD(MCMD_CLASS_DIFS,956)

#define		MCMD_QRY_SSM2FSD_FILE_READ_SECUREINFO	   MAKE_METACMD(MCMD_CLASS_DIFS,1001)  //add by hc
#define		MCMD_QRY_SCM2SSM_FILE_READ_SECUREINFO	   MAKE_METACMD(MCMD_CLASS_DIFS,1002)

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

typedef struct _FILE_FS_SIZE_INFORMATION {
    LARGE_INTEGER   TotalAllocationUnits;
    LARGE_INTEGER   AvailableAllocationUnits;
    ULONG           SectorsPerAllocationUnit;
    ULONG           BytesPerSector;
} FILE_FS_SIZE_INFORMATION, *PFILE_FS_SIZE_INFORMATION;
// 兼容windows客户端，以上元命令定义不动，以下是集群新定义的元命令
// 
// 获取文件系统属性
#define		MCMD_REQ_GFSINFO					MAKE_METACMD(MCMD_CLASS_DIFS,1009)
#define		MCMD_RSP_GFSINFO					MAKE_METACMD(MCMD_CLASS_DIFS,1010)
// 创建文件
#define		MCMD_REQ_CREATE						MAKE_METACMD(MCMD_CLASS_DIFS,1011)
#define		MCMD_RSP_CREATE						MAKE_METACMD(MCMD_CLASS_DIFS,1012)
// 重命名
#define		MCMD_REQ_RENAME						MAKE_METACMD(MCMD_CLASS_DIFS,1013)
#define		MCMD_RSP_RENAME						MAKE_METACMD(MCMD_CLASS_DIFS,1014)
// 列举全目录
#define		MCMD_REQ_READDIR					MAKE_METACMD(MCMD_CLASS_DIFS,1015)
#define		MCMD_RSP_READDIR					MAKE_METACMD(MCMD_CLASS_DIFS,1016)
// 删除文件
#define		MCMD_REQ_DELETE						MAKE_METACMD(MCMD_CLASS_DIFS,1017)
#define		MCMD_RSP_DELETE						MAKE_METACMD(MCMD_CLASS_DIFS,1018)
// 修改文件属性
#define		MCMD_REQ_MODIFY						MAKE_METACMD(MCMD_CLASS_DIFS,1019)
#define		MCMD_RSP_MODIFY						MAKE_METACMD(MCMD_CLASS_DIFS,1020)
// 创建link
#define		MCMD_REQ_LINK						MAKE_METACMD(MCMD_CLASS_DIFS,1021)
#define		MCMD_RSP_LINK						MAKE_METACMD(MCMD_CLASS_DIFS,1022)
/*++
*	关于pid：
*	@目前的元命令处理中，路径需要逐级解析，
*	@每一级路径在对应的mds上处理，
*	@前一级mds得到当前目录的磁盘pid，
*	@pid传递到下一级mds以便定位元数据
*
*	同一条元命令在不同mds之间传递时
*	需要不断变换pid和uNamePos
--*/

//
// Get volume information
//
typedef struct _REQ_GFSINFO_PCKT
{
       int	FileInformationClass;
}REQ_GFSINFO_PCKT, *PREQ_GFSINFO_PCKT;
//
typedef struct _RSP_GFSINFO_PCKT
{
	FILE_FS_SIZE_INFORMATION FsSizeInfo;
}RSP_GFSINFO_PCKT, *PRSP_GFSINFO_PCKT;

//
// create
//
typedef struct _REQ_CREATE_PCKT
{
	basic_attr	Attribute;			// 创建文件的属性
	u32			CreationDisposition;// 针对文件是否存在的创建需求
}REQ_CREATE_PCKT, *PREQ_CREATE_PCKT;
//
typedef struct _RSP_CREATE_PCKT
{
	attr_data	Attribute;		// 创建文件的属性
	// 数据定位信息
}RSP_CREATE_PCKT, *PRSP_CREATE_PCKT;

//
// readdir
//
typedef struct _REQ_READDIR_PCKT
{
	BOOLEAN bWithAttribute;		// 0：只返回文件名；1：一并返回文件属性
	BOOLEAN bSingleFile;			// 0：查询整个目录 1：查询单个文件
	u32		uBufLen;			// 服务器返回数据使用buf的允许的最大长度
	u32    uLastNameLen;
	char   LastName[1];			// a.查询这整个目录时候表示多轮查询的起始位置；b.查询单个文件时候表示查询的文件名
}REQ_READDIR_PCKT, *PREQ_READDIR_PCKT;
//
typedef struct _RSP_READDIR_PCKT
{
	 u32    uBufLen;
	char	DirInfo[1];
}RSP_READDIR_PCKT, *PRSP_READDIR_PCKT;

//
// readdir
//
typedef struct _REQ_DELETE_PCKT
{
	u32 dumy;
}REQ_DELETE_PCKT, *PREQ_DELETE_PCKT;
//
typedef struct _RSP_DELETE_PCKT
{
	u32 dumy;
}RSP_DELETE_PCKT, *PRSP_DELETE_PCKT;

//
// modify
//
typedef struct _REQ_MODIFY_PCKT
{
	basic_attr Attribute;
}REQ_MODIFY_PCKT, *PREQ_MODIFY_PCKT;
//
typedef struct _RSP_MODIFY_PCKT
{
	u32 Attribute;
}RSP_MODIFY_PCKT, *PRSP_MODIFY_PCKT;

//
// rename
//
typedef struct _REQ_RENAME_PCKT
{
	basic_attr Attribute;
}REQ_RENAME_PCKT, *PREQ_RENAME_PCKT;
//
typedef struct _RSP_RENAME_PCKT
{
	basic_attr Attribute;
}RSP_RENAME_PCKT, *PRSP_RENAME_PCKT;

//
// link
//  @link元命令_META_REQ_PCKT请求包中szPathName数据说明：
//    @stage1由客户端填入link的target群路径（link req中填的路径是link文件自身的路径名）
//    @stage2由Mds把link req中的link文件全路径拷贝到其中
//
typedef struct _REQ_LINK_PCKT
{
	char		stage;				// 1：寻找target；2：创建link文件
	ULONG		uPathNameLen;		// link文件全路径的长度（stage2中使用）
	char		szPathName[1024];	// link文件全路径字符串（stage2中使用）
	attr_data	Attribute;				// link的target文件元数据位置（stage2中使用）

}REQ_LINK_PCKT, *PREQ_LINK_PCKT;
//
typedef struct _RSP_LINK_PCKT
{
	attr_data Attribute;
}RSP_LINK_PCKT, *PRSP_LINK_PCKT;
//
// 元命令请求的汇总包
//
typedef struct _META_REQ_PCKT
{
	permis		Pid;			// 父目录的磁盘定位信息
	ULONG		uNamePos;		// 解析的文件名起始位置
	ULONG		uPathNameLen;	// 全路径的长度
	char		szPathName[1024];	// 全路径字符串（utf-8）
	union
	{
		REQ_CREATE_PCKT Create;
		REQ_READDIR_PCKT ReadDir;
		REQ_DELETE_PCKT Delete;
		REQ_MODIFY_PCKT Modify;
		REQ_RENAME_PCKT Rename;
		REQ_GFSINFO_PCKT FsInfo;
		REQ_LINK_PCKT Link;
	}Meta;
}META_REQ_PCKT, *PMETA_REQ_PCKT;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

typedef struct _QRY_SCM2SSM_FILE_READ_SECUREINFO
{
   u32				uSsmFileId;
   u32				uScmFileId;
	ULONG       uPartitionSerial;
	ULONG       FileType;   //0:dir 1:file
	__int64     uPhyFileId;

	u32			mode;
	u32			uid;
	u32			gid;
}QRY_SCM2SSM_FILE_READ_SECUREINFO, *PQRY_SCM2SSM_FILE_READ_SECUREINFO;

typedef QRY_SCM2SSM_FILE_READ_SECUREINFO RESP_QRY_SCM2SSM_FILE_READ_SECUREINFO;

typedef struct _REQ_SCM2SSM_SWITCH
{
    ULONG    uParSeriesNo;
}REQ_SCM2SSM_SWITCH, *PREQ_SCM2SSM_SWITCH;

typedef struct _RESP_SCM2SSM_SWITCH
{
    ULONG    uParSeriesNo;
    BOOL     bSwitch;
}RESP_SCM2SSM_SWITCH, *PRESP_SCM2SSM_SWITCH;


typedef struct _REQ_SSM2FSD_ACTIVE
{
    ULONG    uParSeriesNo;
}REQ_SSM2FSD_ACTIVE, *PREQ_SSM2FSD_ACTIVE;

typedef struct _RESP_SSM2FSD_ACTIVE
{
    ULONG           dummy;
}RESP_SSM2FSD_ACTIVE, *PRESP_SSM2FSD_ACTIVE;

typedef struct _REQ_SSM2FSD_DEACTIVE
{
    ULONG    uParSeriesNo;
}REQ_SSM2FSD_DEACTIVE, *PREQ_SSM2FSD_DEACTIVE;


typedef struct _RESP_SSM2FSD_DEACTIVE
{
    ULONG           dummy;
}RESP_SSM2FSD_DEACTIVE, *PRESP_SSM2FSD_DEACTIVE;

typedef struct _REQ_SSM2FSD_PREDEACTIVE
{
    ULONG    uParSeriesNo;
}REQ_SSM2FSD_PREDEACTIVE, *PREQ_SSM2FSD_PREDEACTIVE;


typedef struct _RESP_SSM2FSD_PREDEACTIVE
{
    ULONG           dummy;
}RESP_SSM2FSD_PREDEACTIVE, *PRESP_SSM2FSD_PREDEACTIVE;

typedef struct _REQ_FSD2FSLD_ACTIVE
{
    ULONG uParSeriesNo;
}REQ_FSD2FSLD_ACTIVE, *PREQ_FSD2FSLD_ACTIVE;

typedef struct _RESP_FSD2FSLD_ACTIVE
{
    ULONG           dummy;
}RESP_FSD2FSLD_ACTIVE, *PRESP_FSD2FSLD_ACTIVE;

typedef struct _REQ_FSD2FSLD_DEACTIVE
{
    ULONG uParSeriesNo;
}REQ_FSD2FSLD_DEACTIVE, *PREQ_FSD2FSLD_DEACTIVE;

typedef struct _RESP_FSD2FSLD_DEACTIVE
{
    ULONG           dummy;
}RESP_FSD2FSLD_DEACTIVE, *PRESP_FSD2FSLD_DEACTIVE;

typedef struct _REQ_SCM2SCMD_GETMETACMD
{
    ULONG    uParSeriesNo;
}REQ_SCM2SCMD_GETMETACMD, *PREQ_SCM2SCMD_GETMETACMD;

typedef struct _REQ_SCM2SCMD_REGISTER_VOLUME
{
    ULONG    uParSeriesNo;
    WCHAR*   VolumeName; 
    WCHAR*   EventName; 
}REQ_SCM2SCMD_REGISTER_VOLUME, *PREQ_SCM2SCMD_REGISTER_VOLUME;

typedef struct _REQ_SCM2SCMD_UNREGISTER_VOLUME
{
    ULONG   uParSeriesNo;
}REQ_SCM2SCMD_UNREGISTER_VOLUME, *PREQ_SCM2SCMD_UNREGISTER_VOLUME;

typedef struct _REQ_SCM2FSD_UPDATASSMID
{
    u32   uSsmId;
}REQ_SCM2FSD_UPDATASSMID, *PREQ_SCM2FSD_UPDATASSMID;
/*
typedef struct _INFO_VOLUME
{
    ULONG    uParSeriesNo;
    //ULONG   SsmVolumId;//����ȫ��ID
    __int64  uRootDirPhyFileId;
}INFO_VOLUME, *PINFO_VOLUME;
*/

typedef struct _INFO_SCM2SSM_VOLUME
{
    ULONG    uParSeriesNo;
    __int64  uRootDirPhyFileId;
}INFO_SCM2SSM_VOLUME, *PINFO_SCM2SSM_VOLUME;

typedef struct _CREATE_REQ
{
    ULONG                               uDifsOplockFlags;      // Create bit set:
                                                    //0x02 - Request an difs oplock
                                                    //0x04 - Request a difs batch oplock
    ULONG                               uFileType;//0: dir 1: file , 2 : partiton, -1: unknown, adjusted by server.
	u8									uClientType; // 0:windows 1: linux
	u64                                 uPhyFileId;
//	#pragma pack(push,8)
    ULONG                               uFcb;
//	#pragma pack(pop)
    u64                                 uTimeStamp;
    ULONG                               uPartitionSerial ;//used when open partition 
    BOOLEAN                             bIsCreateNewDir ;// to create a new dir
    u8                                  uAccessMode;
    __int64                             uParentDirPhyFid;//�����ڸ�Ŀ¼�ľ���·������ID
    BOOLEAN                             bParentDirIsHideOpen;//相对于根目录的绝对路径在该客户端是否是隐式打开过
    DiFS_ACCESS_MASK                    DesiredAccess; //Access desired (See Section 3.8 for an explanation of this field)
    __int64                             AllocationSize; //Initial allocation size
    __int64                             FileSize;  
    __int64                             ValidDataSize;  

 	u32									SecureId;			//The security id add by hc
	u32									Uid;				//The user id  add by hc 02/22/2011
	u32									Gid;				//The group id  add by hc


    DiFS_FILE_ATTRIBUTE                 FileAttributeAndFlags ;
    ULONG                               FileAttributeAndFlags2 ;
    DiFS_SHARE_MODE                     ShareAccess; //Type of share access
    DiFS_CREATE_DISPOSITION             CreateDisposition; //Action if file does/does not exist
    DiFS_CREATE_OPTION                  CreateOptions; //Options to use if creating a file
    ULONG                               uFilePathLen ;//file path length in bytes
    PWCHAR_DEF(pPath); //File to open or create
    ULONG                               uFileNameLen;
    PWCHAR_DEF(pName); //File to open or create
    
}CREATE_REQ, * PCREATE_REQ;

typedef struct _CREATE_RESP
{
    ULONG                   CreateAction;   //The action taken
    s64                     CreationTime;   //The time the file was created
    s64                     LastAccessTime;     //The time the file was accessed
    s64                     LastWriteTime;      //The time the file was last written
    s64                     ChangeTime;         //The time the file was last changed
    __int64                 AllocationSize; //The number of byes allocated
    __int64                 FileSize;  
    __int64                 ValidDataSize;  

	u32						SecureId;			//The security id add by hc
	u32						Uid;				//The user id  add by hc 02/22/2011
	u32						Gid;				//The group id  add by hc

    ULONG                   FileType;
    BOOLEAN                 Directory; //TRUE if this is a directory
    __int64                 uParentDirPhyFid;
    __int64                 uPhyFileId;
    u32                     uSsmFileId; 
    u32                     uScmFileId; //
    u64                     uTimeStamp;
    FILE_ATTR_FLAGS         file_attributes;  
    BOOLEAN                 bDeleteOnClose;
    BOOLEAN                 bDesireDelete;
    BOOLEAN                 bQueryFiOnReachEnd;
    u8                      uCacheMode;
    USHORT                  uDataVolId ;//if is the user file , the DataVol id the file is in
    USHORT                  RunListElmentNum ;//number of runlistelement
    USHORT                  DataLength ; //for read and little file , the buffer contain file data
    ULONG                   ParentDirPhyIDNum;
    PRun_List_Element_DEF(  RunListDataBuf);
    PCHAR_DEF(DataBuffer);      
    PU64_DEF(ParentDirPhyIDList);
    
} CREATE_RESP, * PCREATE_RESP ;


//*********************************************�����л�ʱ�ͻ��˺ͷ���������ݽṹ*******************************************//


typedef struct _INFO_SCM2SSM_CREATEFILE_SCMFILE
{
    u32                              uScmFileId;//�ļ�(Ŀ¼)��SCM ID
    u8                                  uAccessMode;
    ULONG                               uShareMode;//�ļ�����ģʽ
    //BOOLEAN                             bIsCleanup;//�Ƿ���cleanup(�ļ���Ŀ¼)
}INFO_SCM2SSM_CREATEFILE_SCMFILE, * PINFO_SCM2SSM_CREATEFILE_SCMFILE;


typedef struct _INFO_SCM2SSM_CREATEFILE_PHYFILE
{
    ULONG                               uFileType;		//0: dir 1: file 
    u8                                  uFileFlag;		//如果是目录,标识打开方式 0: 显示打开 1: 隐式打开 2: 显示打开，且隐式打开
    ULONG                               uSubOpenCount;	//如果是目录,该目录下打开的目录和文件个数（计数）
    u8                                  uCacheMode;		//���ļ��Ļ���ģʽ
    u64                                 uPhyFileId;		//�ļ�������ID
    u32                                 uSsmFileId;		//�ļ���SSMID
    ULONG                               uFcb;
    char                                bDeleteOnClose;	//*
    char                                bDesireDelete;	//*
    char                                bDeleteSetFlag;	//*�Ƿ��Ѿ�����ɾ������
	char                                bRenameSetFlag;	//*�Ƿ��Ѿ���������������

    ULONG                               uFilePathLen;	//�ļ���·������
    PUCHAR_DEF                           (pPath);			//�ļ���·��,��Ҫ���л�
    ULONG                               uFileNameLen;	//�ļ����ĳ���
    PUCHAR_DEF                           (pName);			//�ļ���,��Ҫ���л�
    ULONG                               uScmFileNum;	//SCM����
    PUCHAR_DEF                           (pScmFile);		//SCM�ļ�,��Ҫ���л�
    //���������pPath��pName��pScmFile(INFO_SCM2SSM_CREATEFILE_SCMFILE)���ڴ����л�
    //...
    
}INFO_SCM2SSM_CREATEFILE_PHYFILE, * PINFO_SCM2SSM_CREATEFILE_PHYFILE;


typedef struct _INFO_SCM2SSM_ALLCREATEFILE
{
    ULONG                               FileTotalNum;//客户端打开的文件总数(包括隐式打开的目录)
    ULONG                               FileNum;//��ǰ�����ļ�����(һ��������1M�������)
    PUCHAR_DEF                         (pAllFileInfo); //�ļ���Ϣ
    //�����������ļ���Ϣ���ڴ����л�
    //...
    
}INFO_SCM2SSM_ALLCREATEFILE, * PINFO_SCM2SSM_ALLCREATEFILE;

//*********************************************�����л�ʱ�ͻ��˺ͷ���������ݽṹ*******************************************//

//��������
typedef struct _REQ_SSM2SCM_SUBSSMSTART
{
    ULONG   ulSerNo;//vol serial no
    DWORD   dwSSMIP;// backup ssm ip
}REQ_SSM2SCM_SUBSSMSTART, * PREQ_SSM2SCM_SUBSSMSTART;

typedef struct _RESP_SSM2SCM_SUBSSMSTART
{
    ULONG dummy;
}RESP_SSM2SCM_SUBSSMSTART, * PRESP_SSM2SCM_SUBSSMSTART;


//���Ļ�����
typedef struct _REQ_SSM2SCM_FS_OPLOCK
{
    ULONG       SsmFileId;  //The file ID allcate by ssm
    ULONG       OplockType; //0:NO_OPLOCK
                            //1:EXCLUSIVE_OPLOCK
                            //2:BATCH_OPLOCK
                            //3:LEVEL2_OPLOCK
}REQ_SSM2SCM_FS_OPLOCK, * PREQ_REQ_SSM2SCM_FS_OPLOCK;

typedef struct _RESP_SSM2SCM_FS_OPLOCK
{
    ULONG       SsmFileId;  //The file ID allcate by ssm
    ULONG       OplockType; //0:NO_OPLOCK
                            //1:EXCLUSIVE_OPLOCK
                            //2:BATCH_OPLOCK
                            //3:LEVEL2_OPLOCK
}RESP_SSM2SCM_FS_OPLOCK, * PRESP_REQ_SSM2SCM_FS_OPLOCK;



//all struct must 4 byte align
//--------------MOUNT---------------------

typedef struct _REQ_SSM2MBD_FS_MOUNT 
{
    ULONG   uParSeriesNo ;
    ULONG   uNameLength ;
    WCHAR   VolumName[DIFS_FILENAME_MAXLENGTH];            //  volum name of this fsd
    
    byte    bIsBack;//是否备份
    DWORD   BackMode;//0: ͬ������ 1: �첽����
    u32     uBackupfilesize;
    u32     uBackupNumb;
    
    u64     difs_formattime;
    u32     bpb_bytes_per_filerecord;
    u16     bpb_bytes_per_sector;
    u16     bpb_sectors_per_cluster;

    u8      BackupPathLen[MBD_BACKUP_MAX_NUMB];
    
}REQ_SSM2MBD_FS_MOUNT,  * PREQ_REQ_SSM2MBD_FS_MOUNT;

typedef struct _REQ_SSM2FSD_FS_MOUNT 
{
    ULONG   uParSeriesNo ;
    ULONG   uNameLength ;
    WCHAR   VolumName[DIFS_FILENAME_MAXLENGTH];            //  volum name of this fsd

    byte      bIsBack;//是否备份
    DWORD     BackMode;//0: ͬ������ 1: �첽����
    u32     uBackupfilesize;
    u32     uBackupNumb;
    //WCHAR   *BackupPath[MBD_BACKUP_MAX_NUMB];
    u8      BackupPathLen[MBD_BACKUP_MAX_NUMB];
    
}REQ_SSM2FSD_FS_MOUNT,  * PREQ_REQ_SSM2FSD_FS_MOUNT;

typedef struct _RESP_SSM2FSD_FS_MOUNT 
{
    ULONG   Status ;                 // 0 for refused, 1 for connect accept
    u64     u64RootDirPhyID;
    USHORT                              DvcbNumber;
    DIFS_FILE_FS_VOLUME_INFORMATION     fs_volume_info ;
    DIFS_FILE_FS_CONTROL_INFORMATION    fs_ctrl_info ;
    DIFS_FILE_FS_DEVICE_INFORMATION     fs_device_info ;
    DIFS_FILE_FS_ATTRIBUTE_INFORMATION  fs_attribute_info ;
}RESP_SSM2FSD_FS_MOUNT, * PRESP_SSM2FSD_FS_MOUNT;


typedef struct _REQ_SCM2FSD_FS_MOUNT 
{
    ULONG   uParSeriesNo ;
    //ULONGLONG     difs_formattime;
    USHORT   DvcbNumber;
    
    USHORT  uNameLength ;
    WCHAR   VolumName[DIFS_FILENAME_MAXLENGTH];            //  volum name of this fsd

    USHORT  uEventNameLength ;
    WCHAR   EventName[100]; //event name (scm2scmd)

    __int64  uRootDirPhyFileId;
    u32      uSsmid;        

    ///info from SSM
    DIFS_FILE_FS_VOLUME_INFORMATION     fs_volume_info ;
    DIFS_FILE_FS_CONTROL_INFORMATION    fs_ctrl_info ;
    DIFS_FILE_FS_DEVICE_INFORMATION     fs_device_info ;
    DIFS_FILE_FS_ATTRIBUTE_INFORMATION  fs_attribute_info ;


}REQ_SCM2FSD_FS_MOUNT,  * PREQ_SCM2FSD_FS_MOUNT;

typedef struct _RESP_SCM2FSD_FS_MOUNT 
{
    ULONG   Status ;                 // 0 for refused, 1 for connect accept
}RESP_SCM2FSD_FS_MOUNT, * PRESP_SCM2FSD_FS_MOUNT;




typedef struct _REQ_SCM2SSM_FS_MOUNT 
{
    ULONG   uParSeriesNo ;
	ULONG   uFlag;                         // ���� 0��������ʽMount 1�������ڴ���Ϣ�������
	ULONG	uMetaCacheMode;
	ULONG	Reserved;
}REQ_SCM2SSM_FS_MOUNT, * PREQ_REQ_SCM2SSM_FS_MOUNT;

typedef struct Volno_Formattime
{
	u32 volno;
	u64 format_time;
}VOLNOFORMATTIME, * PVOLNOFORMATTIME;

typedef struct _RESP_SCM2SSM_FS_MOUNT 
{
    ULONG   Status ;                 // 0 for refused, 1 for connect accept
    //ULONG   SsmVolumId ;             //volum id from ssm    
                                    // 
    USHORT   DvcbNumber;

    __int64  uRootDirPhyFileId;
    u32      uSsmid; 
	DWORD    dwDelayTime;           // ���������ʱ��
	DWORD    dwScmRenewTime;		// SSM�Ͽ�SCM���ӳ�ʱ��
                                     
    DIFS_FILE_FS_VOLUME_INFORMATION     fs_volume_info ;
    DIFS_FILE_FS_CONTROL_INFORMATION    fs_ctrl_info ;
    DIFS_FILE_FS_DEVICE_INFORMATION     fs_device_info ;
    DIFS_FILE_FS_ATTRIBUTE_INFORMATION  fs_attribute_info ;
	ULONGLONG	difsformat_time;
}RESP_SCM2SSM_FS_MOUNT, * PRESP_SCM2SSM_FS_MOUNT;

typedef struct _REQ_FSD2SCMD_FS_REGISTER
{
    ULONG   uParSeriesNo ;
    ULONG   pFsdDevObject;

    WCHAR*  EventName; //event name (scm2scmd)
}REQ_FSD2SCMD_FS_REGISTER, *PREQ_FSD2SCMD_FS_REGISTER;

//----------UNMOUNT----------------------
// 
#define SvrType_SSM     1
#define SvrType_SCM     2

typedef struct _NTF_SVR2FSD_REGISTER
{
    ULONG uSvrType;
    ULONG uProcessId;
}NTF_SVR2FSD_REGISTER, * PNTF_SVR2FSD_REGISTER;

typedef struct _NTF_FSD2SCM_FS_STATUS 
{
    ULONG   ParObject ; //partition object
}NTF_FSD2SCM_FS_STATUS, * PNTF_FSD2SCM_FS_STATUS;

typedef struct _NTF_FSD2SSM_FS_STATUS 
{
    ULONG   ParObject ; //partition object
}NTF_FSD2SSM_FS_STATUS, * PNTF_FSD2SSM_FS_STATUS;

typedef struct _QRY_SERIAL_NUMBER
{
    ULONG   tag;  //must be 'DIFS'
}QRY_SERIAL_NUMBER, *PQRY_SERIAL_NUMBER;

typedef struct _RESP_Qry_SERIAL_NUMBER
{
    ULONG   uVolSerialNo;
}RESP_Qry_SERIAL_NUMBER, *PRESP_Qry_SERIAL_NUMBER;


typedef struct _NTF_SCM2SSM_FS_UNMOUNT 
{
    ULONG   SsmVolumId ;             //volum id from ssm
    ULONG   uParSeriesNo ;
}NTF_SCM2SSM_FS_UNMOUNT, * PNTF_SCM2SSM_FS_UNMOUNT;

typedef struct _REQ_SSM2FSD_FS_UNMOUNT 
{
    ULONG   uParSeriesNo ;
}REQ_SSM2FSD_FS_UNMOUNT, * PREQ_SSM2FSD_FS_UNMOUNT;

typedef struct _RESP_SSM2FSD_FS_UNMOUNT 
{
    ULONG   uParSeriesNo ;
}RESP_SSM2FSD_FS_UNMOUNT, * PRESP_SSM2FSD_FS_UNMOUNT;


typedef struct _REQ_SCM2FSD_FS_UNMOUNT 
{
    ULONG   uParSeriesNo ;
}REQ_SCM2FSD_FS_UNMOUNT, * PREQ_SCM2FSD_FS_UNMOUNT;

typedef struct _RESP_SCM2FSD_FS_UNMOUNT 
{
    ULONG   uParSeriesNo ;           
}RESP_SCM2FSD_FS_UNMOUNT, * PRESP_SCM2FSD_FS_UNMOUNT;

typedef struct _REQ_SSM2SCM_FS_UNMOUNT 
{
    ULONG   uParSeriesNo ;
}REQ_SSM2SCM_FS_UNMOUNT, * PREQ_SSM2SCM_FS_UNMOUNT;

typedef struct _RESP_SSM2SCM_FS_UNMOUNT 
{
    ULONG   uParSeriesNo ;           
}RESP_SSM2SCM_FS_UNMOUNT, * PRESP_SSM2SCM_FS_UNMOUNT;


//-----------��ѯ�ļ�ϵͳ������Ϣ-------------
typedef struct _QRY_FSD2SCM_FS_CTRLINFO 
{
    ULONG   ParObject ; //partition object
    
}QRY_FSD2SCM_FS_CTRLINFO, * PQRY_FSD2SCM_FS_CTRLINFO;

typedef struct _RESP_QRY_FSD2SCM_FS_CTRLINFO
{
    DIFS_FILE_FS_CONTROL_INFORMATION  CtrlInfo ;
}RESP_QRY_FSD2SCM_FS_CTRLINFO, * PRESP_QRY_FSD2SCM_FS_CTRLINFO;

typedef struct _QRY_SCM2SSM_FS_CTRLINFO 
{
    ULONG   SsmVolumId ;             //volum id from ssm
    
}QRY_SCM2SSM_FS_CTRLINFO, * PQRY_SCM2SSM_FS_CTRLINFO;

typedef struct _RESP_QRY_SCM2SSM_FS_CTRLINFO
{
    DIFS_FILE_FS_CONTROL_INFORMATION  CtrlInfo ;
}RESP_QRY_SCM2SSM_FS_CTRLINFO, * PRESP_QRY_SCM2SSM_FS_CTRLINFO;

typedef struct _QRY_SSM2FSD_FS_CTRLINFO 
{
    ULONG   ParObject ; //partition object
    
} QRY_SSM2FSD_FS_CTRLINFO,* PQRY_SSM2FSD_FS_CTRLINFO;

typedef struct _RESP_QRY_SSM2FSD_FS_CTRLINFO
{
    DIFS_FILE_FS_CONTROL_INFORMATION  CtrlInfo ;
}RESP_QRY_SSM2FSD_FS_CTRLINFO, * PRESP_QRY_SSM2FSD_FS_CTRLINFO;

//--------------��ѯ�ļ�ϵͳ��ϸ��Ϣ----------------
typedef struct _QRY_FSD2SCM_FS_FULLSIZEINFO
{
    ULONG   ParObject ; //partition object
    
}QRY_FSD2SCM_FS_FULLSIZEINFO, * PQRY_FSD2SCM_FS_FULLSIZEINFO;

typedef struct _RESP_QRY_FSD2SCM_FS_FULLSIZEINFO
{
    DIFS_FILE_FS_FULL_SIZE_INFORMATION  FullInfo ;
} RESP_QRY_FSD2SCM_FS_FULLSIZEINFO, *PRESP_QRY_FSD2SCM_FS_FULLSIZEINFO ;

typedef struct _QRY_SCM2SSM_FS_FULLSIZEINFO
{
    ULONG   SsmVolumId ;             //volum id from ssm
    
}QRY_SCM2SSM_FS_FULLSIZEINFO,* PQRY_SCM2SSM_FS_FULLSIZEINFO;

typedef struct _RESP_QRY_SCM2SSM_FS_FULLSIZEINFO
{
    DIFS_FILE_FS_FULL_SIZE_INFORMATION  FullInfo ;
}RESP_QRY_SCM2SSM_FS_FULLSIZEINFO, * PRESP_QRY_SCM2SSM_FS_FULLSIZEINFO ;

typedef struct _QRY_SSM2FSD_FS_FULLSIZEINFO
{
    ULONG   ParObject ; //partition object
    
}QRY_SSM2FSD_FS_FULLSIZEINFO,* PQRY_SSM2FSD_FS_FULLSIZEINFO;

typedef struct _RESP_QRY_SSM2FSD_FS_FULLSIZEINFO
{
    DIFS_FILE_FS_FULL_SIZE_INFORMATION  FullInfo ;
}RESP_QRY_SSM2FSD_FS_FULLSIZEINFO, * PRESP_QRY_SSM2FSD_FS_FULLSIZEINFO ;

//----------------��ѯ�ļ�ϵͳ��Ϣ-------------------
typedef struct _QRY_FSD2SCM_FS_SIZEINFO
{
    ULONG   ParObject ; //partition object
    
}QRY_FSD2SCM_FS_SIZEINFO,* PQRY_FSD2SCM_FS_SIZEINFO;

typedef struct _RESP_QRY_FSD2SCM_FS_SIZEINFO
{
    DIFS_FILE_FS_SIZE_INFORMATION   SizeInfo ;
}RESP_QRY_FSD2SCM_FS_SIZEINFO,* PRESP_QRY_FSD2SCM_FS_SIZEINFO;

typedef struct _QRY_SCM2SSM_FS_SIZEINFO
{
    ULONG   SsmVolumId ;             //volum id from ssm
    
}QRY_SCM2SSM_FS_SIZEINFO,* PQRY_SCM2SSM_FS_SIZEINFO;

typedef struct _RESP_QRY_SCM2SSM_FS_SIZEINFO
{
    DIFS_FILE_FS_SIZE_INFORMATION   SizeInfo ;
}RESP_QRY_SCM2SSM_FS_SIZEINFO,* PRESP_QRY_SCM2SSM_FS_SIZEINFO;

typedef struct _QRY_SSM2FSD_FS_SIZEINFO
{
    ULONG   ParObject ; //partition object
    
}QRY_SSM2FSD_FS_SIZEINFO,* PQRY_SSM2FSD_FS_SIZEINFO;

typedef struct _RESP_QRY_SSM2FSD_FS_SIZEINFO
{
    DIFS_FILE_FS_SIZE_INFORMATION   SizeInfo ;
}RESP_QRY_SSM2FSD_FS_SIZEINFO,* PRESP_QRY_SSM2FSD_FS_SIZEINFO;

////////////////CREATE PROCESS ////////////////////////////////////

typedef struct _REQ_FSD2SCM_CREATE 
{
//     ULONG   uVolSerialNo;
//     USHORT  uPartitionSsmId ; //partition id from SSM
//     USHORT  uPartitionScmId ; //partition id from SSM for this SCM
//     ULONG   uFileObject ;//allocate by os io manager

    CREATE_REQ  CreateReq ;
}REQ_FSD2SCM_CREATE,* PREQ_FSD2SCM_CREATE;

typedef struct _RESP_FSD2SCM_CREATE
{
    CREATE_RESP  CreateResp ;
}RESP_FSD2SCM_CREATE,* PRESP_FSD2SCM_CREATE;

typedef struct _REQ_SCM2SSM_CREATE 
{
//     ULONG    uVolSerialNo;
//     USHORT   uPartitionSsmId ; //partition id from SSM
//     USHORT   uPartitionScmId ; //partition id from SSM for this SCM

    CREATE_REQ  CreateReq ;
}REQ_SCM2SSM_CREATE,* PREQ_SCM2SSM_CREATE;


typedef struct _RESP_SCM2SSM_CREATE
{
    UCHAR                   OplockLevel; // The oplock level granted:
                                        //0 - No oplock granted
                                        //1 - Exclusive oplock granted
                                        //2 - Batch oplock granted
                                        //3 - Level II oplock granted

    //ssm , scm, 
//    ULONG                   SsmFileId;  //The file ID allcate by ssm
    
    CREATE_RESP  CreateResp ;
}RESP_SCM2SSM_CREATE,* PRESP_SCM2SSM_CREATE;

//--------------SSM->FSD---------------
// typedef struct _REQ_SSM2FSD_CREATE
// {
//     ULONG   ParObject ; //partition object on the SSM side
//
//     CREATE_REQ  CreateReq ;
// }REQ_SSM2FSD_CREATE, * PREQ_SSM2FSD_CREATE;
//
// typedef struct _RESP_SSM2FSD_CREATE
// {
//     CREATE_RESP  CreateResp ;
// }RESP_SSM2FSD_CREATE, * PRESP_SSM2FSD_CREATE;

typedef struct _REQ_FSD2SCM_CLOSE
{
    //fsd,fsfd 
    ULONG                   uVolSerialNo;
    ULONG                   uFileType;//0: dir 1: file , 2 : partiton
    __int64                 uPhyFileId;
    u32                     uSsmFileId; 
    u32                     uScmFileId; 
    s64                     LastWriteTime;  //Time of last write
    BOOLEAN                 Delete;
    ULONG                   HideParentDirPhyIDNum;
    PU64_DEF(               HideParentDirPhyIDList); 
}REQ_FSD2SCM_CLOSE,* PREQ_FSD2SCM_CLOSE ;

typedef struct _REQ_FSD2SCM_CLEANUP
{
    //fsd,fsfd 
    ULONG                   uVolSerialNo;
    ULONG                   uFileType;//0: dir 1: file , 2 : partiton
    __int64                 uPhyFileId;
    u32                     uSsmFileId; 
    u32                     uScmFileId; 
    s64                     LastWriteTime;  //Time of last write

    __int64                 AllocationSize; //The number of byes allocated
    __int64                 FileSize;  
    __int64                 ValidDataSize; 
}REQ_FSD2SCM_CLEANUP,* PREQ_FSD2SCM_CLEANUP ;

typedef struct _REQ_SCM2SSM_CLEANUP
{
    //fsd,fsfd 
    ULONG                   uVolSerialNo;
    ULONG                   uFileType;//0: dir 1: file , 2 : partiton
    __int64                 uPhyFileId;
    u32                     uSsmFileId; 
    u32                     uScmFileId; 
    s64                     LastWriteTime;  //Time of last write

    __int64                 AllocationSize; //The number of byes allocated
    __int64                 FileSize;  
    __int64                 ValidDataSize; 
}REQ_SCM2SSM_CLEANUP,* PREQ_SCM2SSM_CLEANUP ;

typedef struct _RESP_FSD2SCM_CLOSE
{
    ULONG                   uSsmFileId; 
    u32                     uScmFileId; 
    BOOLEAN                 Deleted ;  //is file deleted
    
}RESP_FSD2SCM_CLOSE,* PRESP_FSD2SCM_CLOSE ;

typedef struct _RESP_FSD2SCM_CLEANUP
{
    ULONG                   uSsmFileId; 
    u32                     uScmFileId; 
    BOOLEAN                 Deleted ;  //is file deleted
    
    u8                      uCacheMode;  //return cachemode (0: û�ı�)
}RESP_FSD2SCM_CLEANUP, *PRESP_FSD2SCM_CLEANUP ;

typedef struct _RESP_SCM2SSM_CLEANUP
{
    ULONG                   uSsmFileId; 
    u32                     uScmFileId; 
    BOOLEAN                 Deleted ;  //is file deleted
    
    u8                      uCacheMode;  //return cachemode (0: û�ı�)
}RESP_SCM2SSM_CLEANUP, *PRESP_SCM2SSM_CLEANUP ;

typedef struct _REQ_SCM2SSM_CLOSE
{
    ULONG                   VolSerialNo;
    ULONG                   FileType;//0: dir 1: file 2:partition
    __int64                 uPhyFileId;
    u32                     SsmFileId;  //The file ID allcate by ssm
    u32                     ScmFileId; 
    s64                     LastWriteTime;  //Time of last write
    BOOLEAN                 Delete;
    ULONG                   HideParentDirPhyIDNum;
    u64 *                   HideParentDirPhyIDList; 
}REQ_SCM2SSM_CLOSE,* PREQ_SCM2SSM_CLOSE ;


typedef struct _RESP_SCM2SSM_CLOSE
{
    u32                      SsmFileId;  //The file ID allcate by ssm
    u32                      ScmFileId; 
    BOOLEAN                  Deleted ;  //is file deleted
    
}RESP_SCM2SSM_CLOSE,* PRESP_SCM2SSM_CLOSE ;


//------------------QUERY FILE/DIR INFO

////////BASIC INFO////////////////////


typedef struct _QRY_FSD2SCM_BASICINFO
{
    ULONG                   FileObject ;//allocate by os io manager
    
}QRY_FSD2SCM_BASICINFO,* PQRY_FSD2SCM_BASICINFO ;

typedef struct _RESP_QRY_FSD2SCM_BASICINFO
{
//  ULONG               FileObject ;//allocate by os io manager
    DIFS_FILE_BASIC_INFORMATION   BasicInfo ;
} RESP_QRY_FSD2SCM_BASICINFO, * PRESP_QRY_FSD2SCM_BASICINFO ;

typedef struct _QRY_SCM2SSM_BASICINFO
{
    ULONG                   FileType;   //0:dir 1:file
    ULONG                   SsmFileId;  //The file ID allcate by ssm
    
}QRY_SCM2SSM_BASICINFO,* PQRY_SCM2SSM_BASICINFO ;

typedef struct _RESP_QRY_SCM2SSM_BASICINFO
{
//  ULONG                   SsmFileId;  //The file ID allcate by ssm
    DIFS_FILE_BASIC_INFORMATION     BasicInfo ;
}RESP_QRY_SCM2SSM_BASICINFO,* PRESP_QRY_SCM2SSM_BASICINFO ;

typedef struct _QRY_SSM2FSD_BASICINFO
{
    ULONG                   FileObject ;//allocate by os io manager
    
}QRY_SSM2FSD_BASICINFO,* PQRY_SSM2FSD_BASICINFO ;

typedef struct _RESP_QRY_SSM2FSD_BASICINFO
{
//  ULONG                   FileObject ;//allocate by os io manager
    DIFS_FILE_BASIC_INFORMATION     BasicInfo ;
}RESP_QRY_SSM2FSD_BASICINFO,* PRESP_QRY_SSM2FSD_BASICINFO ;

//-----------STANDARD INFO-----------------------

typedef struct _QRY_FSD2SCM_STDINFO
{
    ULONG                   FileObject ;//allocate by os io manager

}QRY_FSD2SCM_STDINFO,* PQRY_FSD2SCM_STDINFO;

typedef struct _RESP_QRY_FSD2SCM_STDINFO
{
    DIFS_FILE_STANDARD_INFORMATION  StdInfo ;
}RESP_QRY_FSD2SCM_STDINFO,* PRESP_QRY_FSD2SCM_STDINFO;


typedef struct _QRY_SCM2SSM_STDINFO
{
    ULONG                   FileType;   //0:dir 1:file
    ULONG                   SsmFileId;  //The file ID allcate by ssm
}QRY_SCM2SSM_STDINFO,* PQRY_SCM2SSM_STDINFO ;

typedef struct _RESP_QRY_SCM2SSM_STDINFO
{
    DIFS_FILE_STANDARD_INFORMATION  StdInfo ;
}RESP_QRY_SCM2SSM_STDINFO,* PRESP_QRY_SCM2SSM_STDINFO;

typedef struct _QRY_SSM2FSD_STDINFO
{
    ULONG                   FileObject ;//allocate by os io manager
}QRY_SSM2FSD_STDINFO,* PQRY_SCM2FSD_STDINFO ;

typedef struct _RESP_QRY_SSM2FSD_STDINFO
{
    DIFS_FILE_STANDARD_INFORMATION  StdInfo ;
}RESP_QRY_SSM2FSD_STDINFO,* PRESP_QRY_SSM2FSD_STDINFO;


//-------------SET BASIC INFO------------
typedef struct _REQ_FSD2SCM_SET_BASIC_INFO
{
    ULONG       FileObject ;
    DIFS_FILE_BASIC_INFORMATION     BasicInfo ;
    
}REQ_FSD2SCM_SET_BASIC_INFO,* PREQ_FSD2SCM_SET_BASIC_INFO;

typedef struct _RESP_FSD2SCM_SET_BASIC_INFO
{
    ULONG       Status ;
}RESP_FSD2SCM_SET_BASIC_INFO,* PRESP_FSD2SCM_SET_BASIC_INFO; 

typedef struct _REQ_SCM2SSM_SET_BASIC_INFO
{
    ULONG                   FileType;   //0:dir 1:file
    ULONG       SsmFileId ;
    DIFS_FILE_BASIC_INFORMATION     BasicInfo ;
    
}REQ_SCM2SSM_SET_BASIC_INFO,* PREQ_SCM2SSM_SET_BASIC_INFO;

typedef struct _RESP_SCM2SSM_SET_BASIC_INFO
{
    ULONG       Status ;
}RESP_SCM2SSM_SET_BASIC_INFO,* PRESP_SCM2SSM_SET_BASIC_INFO; 

typedef struct _REQ_SSM2FSD_SET_BASIC_INFO
{
    ULONG       SsmFileId ;
    DIFS_FILE_BASIC_INFORMATION     BasicInfo ;
    
}REQ_SSM2FSD_SET_BASIC_INFO,* PREQ_SSM2FSD_SET_BASIC_INFO;

typedef struct _RESP_SSM2FSD_SET_BASIC_INFO
{
    ULONG       Status ;
}RESP_SSM2FSD_SET_BASIC_INFO,* PRESP_SSM2FSD_SET_BASIC_INFO; 

//----------set filename --------------

typedef struct _REQ_FSD2SCM_SET_FILENAME
{
    ULONG                           FileObject ;
    DIFS_FILE_NAME_INFORMATION      FileInfo ;
}REQ_FSD2SCM_SET_FILENAME,* PREQ_FSD2SCM_SET_FILENAME;

typedef struct _RESP_FSD2SCM_SET_FILENAME
{
    ULONG       Status ;
}RESP_FSD2SCM_SET_FILENAME,* PRESP_FSD2SCM_SET_FILENAME; 

typedef struct _REQ_SCM2SSM_SET_FILENAME
{
    ULONG                   FileType;   //0:dir 1:file
    ULONG       SsmFileId ;
    DIFS_FILE_NAME_INFORMATION      FileInfo ;
    
}REQ_SCM2SSM_SET_FILENAME, * PREQ_SCM2SSM_SET_FILENAME;

typedef struct _RESP_SCM2SSM_SET_FILENAME
{
    ULONG       Status ;
}RESP_SCM2SSM_SET_FILENAME, * PRESP_SCM2SSM_SET_FILENAME; 

typedef struct _REQ_SSM2FSD_SET_FILENAME
{
    ULONG       SsmFileId ;
    DIFS_FILE_NAME_INFORMATION      FileInfo ;
    
}REQ_SSM2FSD_SET_FILENAME, * PREQ_SSM2FSD_SET_FILENAME;

typedef struct _RESP_SSM2FSD_SET_FILENAME
{
    ULONG       Status ;
}RESP_SSM2FSD_SET_FILENAME, * PRESP_SSM2FSD_SET_FILENAME; 

/////////////////////----------LOCK FILE ---------------

typedef struct REQ_FSD2SCM_FILE_LOCK
{
    ULONG           FileObject ;
    LockArea        FileLockArea ;
}*PREQ_FSD2SCM_FILE_LOCK;

typedef struct _RESP_FSD2SCM_FILE_LOCK
{
    ULONG       Status ;
}RESP_FSD2SCM_FILE_LOCK, * PRESP_FSD2SCM_FILE_LOCK; 


typedef struct _REQ_SCM2SSM_FILE_LOCK
{
    ULONG           SsmFileId ;
    LockArea        FileLockArea ;
}REQ_SCM2SSM_FILE_LOCK, * PREQ_SCM2SSM_FILE_LOCK ;                     

typedef struct _RESP_SCM2SSM_FILE_LOCK
{
    ULONG       Status ;
}RESP_SCM2SSM_FILE_LOCK, * PRESP_SCM2SSM_FILE_LOCK; 


///////////////////-----------uNlOCK FILE ---------------
typedef struct REQ_FSD2SCM_FILE_UNLOCK
{
    ULONG           FileObject ;
    LockArea        FileLockArea ;
    
}*PREQ_FSD2SCM_FILE_UNLOCK;

typedef struct _RESP_FSD2SCM_FILE_UNLOCK
{
    ULONG       Status ;
}RESP_FSD2SCM_FILE_UNLOCK, * PRESP_FSD2SCM_FILE_UNLOCK; 

typedef struct _REQ_SCM2SSM_FILE_UNLOCK
{
    ULONG           SsmFileId ;
    LockArea        FileLockArea ;
    
}REQ_SCM2SSM_FILE_UNLOCK, * PREQ_SCM2SSM_FILE_UNLOCK;
                        
typedef struct _RESP_SCM2SSM_FILE_UNLOCK
{
    ULONG       Status ;
}RESP_SCM2SSM_FILE_UNLOCK, * PRESP_SCM2SSM_FILE_UNLOCK; 

/////////////////////////---------FLUSH ---------------------
typedef struct REQ_FSD2SCM_FILE_FLUSH
{
    ULONG           FileObject ;
   __int64           ValidDataLength;   
}*PREQ_FSD2SCM_FILE_FLUSH;
/*
typedef struct RESP_FSD2SCM_FILE_FLUSH
{
    ULONG       Status ;
}* PRESP_FSD2SCM_FILE_FLUSH; 
*/
typedef struct _REQ_SCM2SSM_FILE_FLUSH
{
    ULONG           SsmFileId ;
   __int64           ValidDataLength;   
}REQ_SCM2SSM_FILE_FLUSH, * PREQ_SCM2SSM_FILE_FLUSH;
/*
typedef struct RESP_SCM2SSM_FILE_FLUSH
{
    ULONG       Status ;
}* PRESP_SCM2SSM_FILE_FLUSH; 
*/
/////////////////////---------------QUERY FILE ALLOCTION INFO --------
typedef struct _FileAllocInfo
{
    s64               CreationTime;   //The time the file was created
    s64               LastAccessTime;     //The time the file was accessed
    s64               LastWriteTime;      //The time the file was last written
    s64               ChangeTime;         //The time the file was last changed
    __int64                 AllocationSize; //The number of byes allocated
    __int64                 FileSize;  
    __int64                 ValidDataSize;  
    int                      FileType;
    BOOLEAN                 Directory; //TRUE if this is a directory
    USHORT                  uDataVolId ;//if is the user file , the DataVol id the file is in

    ULONG                  nEntryNumber ;//the number of Run_List_Element
    //PRun_List_Element array ,in this array store the alloction info of the file in disk
    PRun_List_Element_DEF( pRun_List_Element);

}FileAllocInfo, * PFileAllocInfo;


typedef struct  _QRY_FSD2SCM_FILE_READ_ALLOCINFO
{
	u32							uSsmFileid;
	u32							uScmFileid;
    ULONG                   uParSeriesNo ;
    __int64                 uPhyFileId;
    
}QRY_FSD2SCM_FILE_READ_ALLOCINFO, * PQRY_FSD2SCM_FILE_READ_ALLOCINFO        ;   

typedef struct _RESP_FSD2SCM_FILE_READ_ALLOCINFO
{
    FILE_ATTR_FLAGS         file_attributes;                                        
    FileAllocInfo   alloc_info;
}RESP_FSD2SCM_FILE_READ_ALLOCINFO, * PRESP_FSD2SCM_FILE_READ_ALLOCINFO; 

typedef struct  _QRY_SCM2SSM_FILE_READ_ALLOCINFO
{
    u32							uSsmFileid;
	u32							uScmFileid;
    ULONG                   uParSeriesNo ;
    __int64                 uPhyFileId;
    
}QRY_SCM2SSM_FILE_READ_ALLOCINFO, * PQRY_SCM2SSM_FILE_READ_ALLOCINFO        ;   

typedef struct _RESP_SCM2SSM_FILE_READ_ALLOCINFO
{
    FILE_ATTR_FLAGS         file_attributes;                                        
    FileAllocInfo   alloc_info;
}RESP_SCM2SSM_FILE_READ_ALLOCINFO, * PRESP_SCM2SSM_FILE_READ_ALLOCINFO; 

typedef struct  _QRY_SSM2FSD_FILE_READ_ALLOCINFO
{
    HANDLE_DEF_UK(          hFile);
    ULONG                   ParentDirPhyIDNum;
}QRY_SSM2FSD_FILE_READ_ALLOCINFO, * PQRY_SSM2FSD_FILE_READ_ALLOCINFO        ;   

typedef struct _RESP_SSM2FSD_FILE_READ_ALLOCINFO
{
    FILE_ATTR_FLAGS         file_attributes;                                        
    FileAllocInfo           alloc_info;

    __int64                 uPhyFileId;
    u64                     uTimeStamp;
    ULONG                   ParentDirPhyIDNum;
    PU64_DEF(               ParentDirPhyIDList);
}RESP_SSM2FSD_FILE_READ_ALLOCINFO, * PRESP_SSM2FSD_FILE_READ_ALLOCINFO; 

/////////-----SET FILE EOF---------------
typedef struct REQ_FSD2SCM_SET_FILE_EOF
{
    ULONG           FileObject ;
    __int64         EndOfFile    ;
}*PREQ_FSD2SCM_SET_FILE_EOF;                

typedef struct _RESP_FSD2SCM_SET_FILE_EOF
{
    ULONG       Status ;
}RESP_FSD2SCM_SET_FILE_EOF, * PRESP_FSD2SCM_SET_FILE_EOF; 


typedef struct REQ_SCM2SSM_SET_FILE_EOF
{
    ULONG           SsmFileId ;
    __int64         EndOfFile    ;
}*PREQ_SCM2SSM_SET_FILE_EOF ;           

typedef struct _RESP_SCM2SSM_SET_FILE_EOF
{
    ULONG       Status ;
}RESP_SCM2SSM_SET_FILE_EOF, * PRESP_SCM2SSM_SET_FILE_EOF; 

typedef struct REQ_SSM2FSD_SET_FILE_EOF
{
    ULONG           FileObject ;
    __int64         EndOfFile    ;
}*PREQ_SSM2FSD_SET_FILE_EOF;            

typedef struct _RESP_SSM2FSD_SET_FILE_EOF
{
    ULONG       Status ;
}RESP_SSM2FSD_SET_FILE_EOF, * PRESP_SSM2FSD_SET_FILE_EOF; 

//////////-------------SET VALID LENGTH -------------
typedef struct _REQ_FSD2SCM_SET_FILE_VALID_LEN           
{
    ULONG           FileObject ;
    __int64          ValidDataLength;
}REQ_FSD2SCM_SET_FILE_VALID_LEN, * PREQ_FSD2SCM_SET_FILE_VALID_LEN      ;   

typedef struct _RESP_FSD2SCM_SET_FILE_VALID_LEN
{
    ULONG       Status ;
}RESP_FSD2SCM_SET_FILE_VALID_LEN, * PRESP_FSD2SCM_SET_FILE_VALID_LEN; 


typedef struct REQ_SCM2SSM_SET_FILE_VALID_LEN
{
    ULONG           SsmFileId ;
    __int64          ValidDataLength;
}*PREQ_SCM2SSM_SET_FILE_VALID_LEN   ;

typedef struct _RESP_SCM2SSM_SET_FILE_VALID_LEN
{
    ULONG       Status ;
}RESP_SCM2SSM_SET_FILE_VALID_LEN, * PRESP_SCM2SSM_SET_FILE_VALID_LEN; 


typedef struct REQ_SSM2FSD_SET_FILE_VALID_LEN
{
    ULONG           FileObject ;
    __int64          ValidDataLength;
}*PREQ_SSM2FSD_SET_FILE_VALID_LEN;              

typedef struct _RESP_SSM2FSD_SET_FILE_VALID_LEN
{
    ULONG       Status ;
}RESP_SSM2FSD_SET_FILE_VALID_LEN, * PRESP_SSM2FSD_SET_FILE_VALID_LEN; 


///////////---------SET FILE ALLOCATION ---------
typedef struct _REQ_FSD2SCM_SET_FILE_ALLOCSIZE
{
    u32      uSsmFileId;
    u32      uScmFileId;
    ULONG       uPartitionSerial;
    __int64     uPhyFileId;
    __int64     AllocationSize;
    __int64     FileSize;  
    __int64     ValidDataSize;  
    
}REQ_FSD2SCM_SET_FILE_ALLOCSIZE, *PREQ_FSD2SCM_SET_FILE_ALLOCSIZE;      

typedef struct _RESP_FSD2SCM_SET_FILE_ALLOCSIZE
{
    ULONG       Status ; //DISTATUS
    __int64     RealAllocationSize;
    __int64     ValidDataSize;
    __int64     FileSize;  
    int         nEntryNumber ;//the number of Run_List_Element

    //PRun_List_Element array ,in this array store the alloction info of the file in disk
    PRun_List_Element_DEF( run_List_Element);
}RESP_FSD2SCM_SET_FILE_ALLOCSIZE, * PRESP_FSD2SCM_SET_FILE_ALLOCSIZE; 

typedef struct _REQ_SCM2SSM_SET_FILE_ALLOCSIZE
{
    u32      uSsmFileId;
    u32      uScmFileId;
    ULONG       uPartitionSerial;
    __int64     uPhyFileId;

    __int64     AllocationSize;
    __int64     FileSize;  
    __int64     ValidDataSize;  
}REQ_SCM2SSM_SET_FILE_ALLOCSIZE, *PREQ_SCM2SSM_SET_FILE_ALLOCSIZE           ;

typedef struct _RESP_SCM2SSM_SET_FILE_ALLOCSIZE
{
    ULONG       Status ;
    __int64     _i64RealAllocationSize;
    __int64     ValidDataSize;
    __int64     FileSize;  
    int         nEntryNumber ;//the number of Run_List_Element

    //PRun_List_Element array ,in this array store the alloction info of the file in disk
    PRun_List_Element_DEF( run_List_Element);
}RESP_SCM2SSM_SET_FILE_ALLOCSIZE, * PRESP_SCM2SSM_SET_FILE_ALLOCSIZE; 

typedef struct _REQ_SSM2FSD_SET_FILE_ALLOCSIZE
{
    HANDLE_DEF_UK( hFile);
    u32      uSsmFileId;
    u32      uScmFileId;
    __int64     uPhyFileId;
    __int64     AllocationSize;
    __int64     FileSize;
    __int64     ValidDataSize;
}REQ_SSM2FSD_SET_FILE_ALLOCSIZE, *PREQ_SSM2FSD_SET_FILE_ALLOCSIZE        ;

typedef struct _REQ_SSM2FSD_MARK_FILE_ALLOCSIZE
{
    HANDLE_DEF_UK( hFile);
    __int64     uPhyFileId;
    __int64     FileSize;

    int         nEntryNumber ;
    PRun_List_Element_DEF( run_List_Element);
}REQ_SSM2FSD_MARK_FILE_ALLOCSIZE, *PREQ_SSM2FSD_MARK_FILE_ALLOCSIZE ;

typedef struct _RESP_SSM2FSD_SET_FILE_ALLOCSIZE
{
    ULONG       Status ;
    __int64     RealAllocationSize;
    int         nEntryNumber ;//the number of Run_List_Element

    //PRun_List_Element array ,in this array store the alloction info of the file in disk
    PRun_List_Element_DEF( run_List_Element);
}RESP_SSM2FSD_SET_FILE_ALLOCSIZE, * PRESP_SSM2FSD_SET_FILE_ALLOCSIZE; 


typedef struct _REQ_SSM2FSD_SET_FILE_SIZEINFO
{
    HANDLE_DEF_UK( hFile);
    u64     uPhyFileId;
    u64     AllocationSize;
    u64     FileSize;
    u64     ValidDataSize;

}REQ_SSM2FSD_SET_FILE_SIZEINFO, *PREQ_SSM2FSD_SET_FILE_SIZEINFO;

typedef struct _RESP_SSM2FSD_SET_FILE_SIZEINFO
{
    u64     ValidDataSize;  
}RESP_SSM2FSD_SET_FILE_SIZEINFO, *PRESP_SSM2FSD_SET_FILE_SIZEINFO;

/////////////////////QUERY DIR BOTH INFO /////////////
typedef struct QRY_FSD2SCM_DIR_INFO
{
    ULONG           VolSerialNo;

    __int64         uPhyFileId;
    u32             uSsmId;
    u32             uScmId;
	u32				uQryId;
    BOOLEAN         FirstQuery;
    int             FileInformationClass;  //FILE_INFORMATION_CLASS
    BOOLEAN         IndexSpecified ;
    BOOLEAN         RestartScan;
    BOOLEAN         ReturnSingleEntry;
    ULONG           FileIndex;
    u64             TimeStamp;

    int             nBufferLen ;
    int             fileNameLen;
    PUCHAR_DEF(     Buffer);    
    PWCHAR_DEF(     fileName);

}QRY_FSD2SCM_DIR_INFO, *PQRY_FSD2SCM_DIR_INFO    ;

typedef struct _RESP_QRY_FSD2SCM_DIR_INFO
{
    u64             TimeStamp;
    u32             uScmId;
	u32				uQryId;
    ULONG           UsedLength;
    int             nBufferLen ;
    PUCHAR_DEF(     Buffer) ;  
}RESP_QRY_FSD2SCM_DIR_INFO, * PRESP_QRY_FSD2SCM_DIR_INFO; 

typedef struct QRY_SCM2SSM_DIR_INFO
{
    ULONG           VolSerialNo;

    __int64         uPhyFileId;
    u32             uSsmId;
    u32             uScmId;
	u32				uQryId;
    BOOLEAN         FirstQuery;
    int             FileInformationClass;  //FILE_INFORMATION_CLASS
    BOOLEAN         IndexSpecified ;
    BOOLEAN         RestartScan;
    BOOLEAN         ReturnSingleEntry;
    ULONG           FileIndex;
    u64             TimeStamp;

    int             nBufferLen ;
    int             fileNameLen;
    PUCHAR_DEF(     Buffer) ;   
    PWCHAR_DEF(     fileName);
    
}QRY_SCM2SSM_DIR_INFO, *PQRY_SCM2SSM_DIR_INFO ;                  

typedef struct _RESP_QRY_SCM2SSM_DIR_INFO
{
    u64         TimeStamp;
    u32         uScmId;
	u32				uQryId;
    ULONG       UsedLength;
    int         nBufferLen ;
    PUCHAR_DEF( Buffer) ;  
}RESP_QRY_SCM2SSM_DIR_INFO, * PRESP_QRY_SCM2SSM_DIR_INFO; 

typedef struct QRY_SSM2FSD_DIR_INFO
{
    ULONG           VolSerialNo;

    HANDLE_DEF_UK(  hHandleDir);
    __int64         uPhyFileId;
    BOOLEAN         FirstQuery;
    int             FileInformationClass;  //FILE_INFORMATION_CLASS
    BOOLEAN         IndexSpecified ;
    BOOLEAN         RestartScan;
    BOOLEAN         ReturnSingleEntry;
    ULONG           FileIndex;

    int             nBufferLen ;
    int             fileNameLen;
    PUCHAR_DEF(     Buffer) ;   
    PWCHAR_DEF(     fileName);
}QRY_SSM2FSD_DIR_INFO, *PQRY_SSM2FSD_DIR_INFO ;                  

typedef struct _RESP_QRY_SSM2FSD_DIR_INFO
{
    ULONG       UsedLength;
    int         nBufferLen ;
    PUCHAR_DEF( Buffer) ;
}RESP_QRY_SSM2FSD_DIR_INFO, * PRESP_QRY_SSM2FSD_DIR_INFO; 



//QRY_VOLUME_INFORMATION
typedef struct QRY_FSD2SCM_VOLUME_INFORMATION
{
    ULONG           VolSerialNo;
    
    int             FileInformationClass;
    
    int             nBufferLen ;
    PUCHAR_DEF(     Buffer);   
}QRY_FSD2SCM_VOLUME_INFORMATION, *PQRY_FSD2SCM_VOLUME_INFORMATION    ;

typedef struct _RESP_QRY_FSD2SCM_VOLUME_INFORMATION
{
    int             nBufferLen ;
    PUCHAR_DEF(     Buffer) ;  
}RESP_QRY_FSD2SCM_VOLUME_INFORMATION, * PRESP_QRY_FSD2SCM_VOLUME_INFORMATION; 

typedef struct QRY_SCM2SSM_VOLUME_INFORMATION
{
    ULONG           VolSerialNo;
    
    int             FileInformationClass;
    
    int             nBufferLen ;
    PUCHAR_DEF(     Buffer) ;   
}QRY_SCM2SSM_VOLUME_INFORMATION, *PQRY_SCM2SSM_VOLUME_INFORMATION    ;

typedef struct _RESP_QRY_SCM2SSM_VOLUME_INFORMATION
{
    int             nBufferLen ;
    PUCHAR_DEF(     Buffer) ;  
}RESP_QRY_SCM2SSM_VOLUME_INFORMATION, * PRESP_QRY_SCM2SSM_VOLUME_INFORMATION; 

typedef struct QRY_SSM2FSD_VOLUME_INFORMATION
{
    ULONG           VolSerialNo;
    
    int             FileInformationClass;
    
    int             nBufferLen ;
    PUCHAR_DEF(     Buffer) ;   
}QRY_SSM2FSD_VOLUME_INFORMATION, *PQRY_SSM2FSD_VOLUME_INFORMATION    ;

typedef struct _RESP_QRY_SSM2FSD_VOLUME_INFORMATION
{
    int             nBufferLen ;
    PUCHAR_DEF(     Buffer);  
}RESP_QRY_SSM2FSD_VOLUME_INFORMATION, * PRESP_QRY_SSM2FSD_VOLUME_INFORMATION; 


//SET_VOLUME_INFORMATION
typedef struct SET_FSD2SCM_VOLUME_INFORMATION
{
    ULONG           VolSerialNo;
    
    int             FileInformationClass;
    int             nBufferLen ;
    PUCHAR_DEF(     Buffer) ;  
    
}SET_FSD2SCM_VOLUME_INFORMATION, *PSET_FSD2SCM_VOLUME_INFORMATION    ;

typedef struct _RESP_SET_FSD2SCM_VOLUME_INFORMATION
{
    u32 dummy;
}RESP_SET_FSD2SCM_VOLUME_INFORMATION, * PRESP_SET_FSD2SCM_VOLUME_INFORMATION; 

typedef struct SET_SCM2SSM_VOLUME_INFORMATION
{
    ULONG           VolSerialNo;
    
    int             FileInformationClass;
    int             nBufferLen ;
    PUCHAR_DEF(     Buffer) ;  
    
}SET_SCM2SSM_VOLUME_INFORMATION, *PSET_SCM2SSM_VOLUME_INFORMATION    ;

typedef struct _RESP_SET_SCM2SSM_VOLUME_INFORMATION
{
    u32 dummy;
}RESP_SET_SCM2SSM_VOLUME_INFORMATION, * PRESP_SET_SCM2SSM_VOLUME_INFORMATION; 

typedef struct SET_SSM2FSD_VOLUME_INFORMATION
{
    ULONG           VolSerialNo;
    
    int             FileInformationClass;
    int             nBufferLen ;
    PUCHAR_DEF(     Buffer) ;  
    
}SET_SSM2FSD_VOLUME_INFORMATION, *PSET_SSM2FSD_VOLUME_INFORMATION    ;

typedef struct _RESP_SET_SSM2FSD_VOLUME_INFORMATION
{
    u32 dummy;
}RESP_SET_SSM2FSD_VOLUME_INFORMATION, * PRESP_SET_SSM2FSD_VOLUME_INFORMATION; 


typedef struct REQ_SSM2SCM_FILEDELETE
{
    ULONG           uVolSerialNo;
    ULONG           PhyIDNum;
    PU64_DEF(       PhyIDList);
    
}REQ_SSM2SCM_FILEDELETE, *PREQ_SSM2SCM_FILEDELETE    ;

typedef struct REQ_SCM2FSD_FILEDELETE
{
    ULONG           uVolSerialNo;
    ULONG           PhyIDNum;
    PU64_DEF(       PhyIDList);
    
}REQ_SCM2FSD_FILEDELETE, *PREQ_SCM2FSD_FILEDELETE    ;


//set information
typedef struct SET_FSD2SCM_INFORMATION
{
    ULONG           VolSerialNo;
    
    __int64         uPhyFileId;
    u32             uSsmFileId; 
    u32             uScmFileId; 
    ULONG           FileType;   //0:dir 1:file
    int             FileInformationClass;
    BOOLEAN         AdvanceOnly;
    BOOLEAN         ReplaceIfExists;
    int             nBufferLen ;
    __int64         ValidDataSize;  
    PUCHAR_DEF(     Buffer) ;  
    
}SET_FSD2SCM_INFORMATION, *PSET_FSD2SCM_INFORMATION    ;

typedef struct _RESP_SET_FSD2SCM_INFORMATION
{
    __int64         AllocationSize; //The number of byes allocated
    __int64         FileSize;  
    __int64         ValidDataSize;  
    ULONG                  RunListElmentNum ;//number of runlistelement
    PRun_List_Element_DEF( RunListDataBuf);
}RESP_SET_FSD2SCM_INFORMATION, * PRESP_SET_FSD2SCM_INFORMATION; 

typedef struct SET_SCM2SSM_INFORMATION
{
    ULONG           VolSerialNo;
    
    __int64         uPhyFileId;
    u32             uSsmFileId; 
    u32             uScmFileId; 
    ULONG           FileType;   //0:dir 1:file
    int             FileInformationClass;
    BOOLEAN         AdvanceOnly;
    BOOLEAN         ReplaceIfExists;
    int             nBufferLen ;
    __int64         ValidDataSize;  
    PUCHAR_DEF(     Buffer) ;  
    
}SET_SCM2SSM_INFORMATION, *PSET_SCM2SSM_INFORMATION    ;

typedef struct _RESP_SET_SCM2SSM_INFORMATION
{
    __int64         AllocationSize; //The number of byes allocated
    __int64         FileSize;  
    __int64         ValidDataSize; 
    BOOL            DeleteFile;
    ULONG           pan;
    ULONG                  RunListElmentNum ;//number of runlistelement
    PRun_List_Element_DEF( RunListDataBuf) ;
}RESP_SET_SCM2SSM_INFORMATION, * PRESP_SET_SCM2SSM_INFORMATION; 

typedef struct SET_SSM2FSD_INFORMATION
{
    ULONG           VolSerialNo;
    
    HANDLE_DEF_UK(  hHandleFile);
    __int64         uPhyFileId;
    int             FileInformationClass;
    BOOLEAN         AdvanceOnly;
    BOOLEAN         ReplaceIfExists;
    int             nBufferLen ;
    __int64         ValidDataSize;  
    PUCHAR_DEF(     Buffer) ;  
    
}SET_SSM2FSD_INFORMATION, *PSET_SSM2FSD_INFORMATION    ;

typedef struct FILE_SECURE_INFOMATION 
{
	u32 uid;
	u32 gid;
	u32 mode;
	u32 opUid;  /* current*/
	u32 opGid;  /* current*/
}FILE_SECURE_INFOMATION,*PFILE_SECURE_INFOMATION;

typedef struct _RESP_SET_SSM2FSD_INFORMATION
{
    __int64         AllocationSize; //The number of byes allocated
    __int64         FileSize;  
    __int64         ValidDataSize;  
    ULONG           RunListElmentNum ;//number of runlistelement
    PRun_List_Element_DEF( RunListDataBuf);
}RESP_SET_SSM2FSD_INFORMATION, * PRESP_SET_SSM2FSD_INFORMATION; 


typedef struct REQ_SSM2FSD_CHECKVOLUME
{
    ULONG           VolSerialNo;
}REQ_SSM2FSD_CHECKVOLUME, *PREQ_SSM2FSD_CHECKVOLUME    ;

typedef struct _RESP_SSM2FSD_CHECKVOLUME
{
    BOOLEAN         bIsValid;
}RESP_SSM2FSD_CHECKVOLUME, *PRESP_SSM2FSD_CHECKVOLUME    ;


typedef struct _REQ_SSM2FSD_PRECLOSE_HANDLE
{
    HANDLE_DEF_UK( hFile);
}REQ_SSM2FSD_PRECLOSE_HANDLE, *PREQ_SSM2FSD_PRECLOSE_HANDLE;

typedef struct _RESP_SSM2FSD_PRECLOSE_HANDLE
{
    ULONG           dummy; 
}RESP_SSM2FSD_PRECLOSE_HANDLE, *PRESP_SSM2FSD_PRECLOSE_HANDLE;

typedef struct _REQ_SCM2FSD_DISCONNECT_SSM
{
    ULONG           VolSerialNo;
}REQ_SCM2FSD_DISCONNECT_SSM, *PREQ_SCM2FSD_DISCONNECT_SSM;

typedef struct _RESP_SCM2FSD_DISCONNECT_SSM
{
    ULONG           dummy;
}RESP_SCM2FSD_DISCONNECT_SSM, *PRESP_SCM2FSD_DISCONNECT_SSM;

typedef struct _REQ_SCM2FSD_CONNECT_SSM
{
    ULONG           VolSerialNo;
    ULONG           uChgCntAdd;
}REQ_SCM2FSD_CONNECT_SSM, *PREQ_SCM2FSD_CONNECT_SSM;

typedef struct _RESP_SCM2FSD_CONNECT_SSM
{
    ULONG           dummy;
}RESP_SCM2FSD_CONNECT_SSM, *PRESP_SCM2FSD_CONNECT_SSM;

///////dir modify notify /////////////////////
typedef struct QRY_FSD2SCM_DIR_NOTIFY_CHANGE
{
    ULONG           FileObject ;
    
}*PQRY_FSD2SCM_DIR_NOTIFY_CHANGE     ;


typedef struct _RESP_QRY_FSD2SCM_DIR_NOTIFY_CHANGE
{
    ULONG       Status ;
}RESP_QRY_FSD2SCM_DIR_NOTIFY_CHANGE, *PRESP_QRY_FSD2SCM_DIR_NOTIFY_CHANGE        ;

typedef struct QRY_SCM2SSM_DIR_NOTIFY_CHANGE
{
    ULONG           SsmFileId ;
    
} *PQRY_SCM2SSM_DIR_NOTIFY_CHANGE;          

typedef struct RESP_QRY_SCM2SSM_DIR_NOTIFY_CHANGE
{
    ULONG       Status ;
}*PRESP_QRY_SCM2SSM_DIR_NOTIFY_CHANGE        ;


typedef struct QRY_SSM2FSD_DIR_NOTIFY_CHANGE
{
    ULONG           FileObject ;
    
}*PQRY_SSM2FSD_DIR_NOTIFY_CHANGE    ;

typedef struct RESP_QRY_SSM2FSD_DIR_NOTIFY_CHANGE
{
    ULONG       Status ;
}*PRESP_QRY_SSM2FSD_DIR_NOTIFY_CHANGE        ;

typedef struct REQ_FSD2SCM_SET_FILE_DELETE
{
    DIFS_FILE_DISPOSITION_INFORMATION   delete_info;
}*PREQ_FSD2SCM_SET_FILE_DELETE;

typedef struct RESP_FSD2SCM_SET_FILE_DELETE
{
    ULONG       Status ;
}*PRESP_FSD2SCM_SET_FILE_DELETE;

typedef struct REQ_SCM2SSM_SET_FILE_DELETE
{
    DIFS_FILE_DISPOSITION_INFORMATION   delete_info;
}*PREQ_SCM2SSM_SET_FILE_DELETE;

typedef struct RESP_SCM2SSM_SET_FILE_DELETE
{
    ULONG       Status ;
}*PRESP_SCM2SSM_SET_FILE_DELETE;

typedef struct REQ_SSM2FSD_SET_FILE_DELETE
{
    DIFS_FILE_DISPOSITION_INFORMATION   delete_info;
}*PREQ_SSM2FSD_SET_FILE_DELETE;

typedef struct RESP_SSM2FSD_SET_FILE_DELETE
{
    ULONG       Status ;
}*PRESP_SSM2FSD_SET_FILE_DELETE;

typedef struct _REQ_FSD2SCM_RENAME_FILENAME
{
    ULONG               uVolSerialNo; 
    ULONG               FileType;   //0:dir 1:file
    //Դ�ļ�����
    u64                 uSrcPathPhyId;
    u64                 uSrcFilePhyId;
    u32                 ssmid;
    u32                 scmid;
    //Ŀ���ļ�����
    BOOLEAN             bReplaceIfExists ;
    u64                 uTargetFileRootPathPhyId; //������������ʱΪ0�� ������Ϊ0 
    u16                 uTargetRelativePathLen ;
    u16                 uTargetFilenameLen ;
    PWCHAR_DEF(         pTargetRelativePath); //当简单重命名时为NULL ，否则不为NULL
    PWCHAR_DEF(         pTargetFilename);//file name
}REQ_FSD2SCM_RENAME_FILENAME, * PREQ_FSD2SCM_RENAME_FILENAME;

typedef struct _RESP_FSD2SCM_RENAME_FILENAME
{
    ULONG           dummy;
}RESP_FSD2SCM_RENAME_FILENAME, * PRSP_FSD2SCM_RENAME_FILENAME;

typedef struct _REQ_SCM2SSM_RENAME_FILENAME
{
    ULONG               uVolSerialNo; 
    ULONG               FileType;   //0:dir 1:file
    //Դ�ļ�����
    u64                 uSrcPathPhyId;
    u64                 uSrcFilePhyId;
    u32                 ssmid;
    u32                 scmid;
    //Ŀ���ļ�����
    BOOLEAN             bReplaceIfExists ;
    u64                 uTargetFileRootPathPhyId; //������������ʱΪ0�� ������Ϊ0 
    u16                 uTargetRelativePathLen ;
    u16                 uTargetFilenameLen ;
    PWCHAR_DEF(         pTargetRelativePath); //当简单重命名时为NULL ，否则不为NULL
    PWCHAR_DEF(         pTargetFilename);//file name
}REQ_SCM2SSM_RENAME_FILENAME, * PREQ_SCM2SSM_RENAME_FILENAME;

typedef struct _RESP_SCM2SSM_RENAME_FILENAME
{
    ULONG               dummy;
}RESP_SCM2SSM_RENAME_FILENAME, * PRESP_SCM2SSM_RENAME_FILENAME;

typedef struct _REQ_SSM2FSD_RENAME_FILENAME
{
    HANDLE_DEF_UK(      hSrcFile);
    HANDLE_DEF_UK(      hDesPath);
    BOOLEAN             bReplaceIfExists ;
    u16                 uDesFilenameLen ;
    PWCHAR_DEF(         pTargetFilename);//file name
}REQ_SSM2FSD_RENAME_FILENAME, * PREQ_SSM2FSD_RENAME_FILENAME;

typedef struct _RESP_SSM2FSD_RENAME_FILENAME
{
    ULONG               dummy;
}RESP_SSM2FSD_RENAME_FILENAME, * PRESP_SSM2FSD_RENAME_FILENAME;

//fsd and Mbd's communication structures
typedef struct _REQ_FSD2MBD_REGISTER
{
    BOOL bIsregistered;
    DWORD AsnyMode;
    u32 bytespermft;
    u32 bytespercluster;
    u32 uVolSerialNumb;
    u32 uVolPartsNumb;
    u32 backupfilesize;
    u32 mbNumb;
    u64 uFormatTime;
    WCHAR *partnames[MBD_BACKUP_MAX_NUMB];
}REQ_FSD2MBD_REGISTER, *PREQ_FSD2MBD_REGISTER;

typedef struct _REQ_FSD2MBD_UNREGISTER
{
    u32 uVolSerialNumb;
}REQ_FSD2MBD_UNREGISTER, *PREQ_FSD2MBD_UNREGISTER;

// fsd and fsld's communication structures
typedef struct _REQ_FSD2FSLD_REGISTER   
{
    //u32             m_VolumeID;                 // ����ÿһ������ID
    u32                     m_partionSerialNum;
    u32                     m_logListLen;
    PDIFS_LOGLIST           m_ppLogList[DIFS_MAX_DATAVOL_NUM + DIFS_MAX_SYSVOL_NUM];
    u64/*PDEVICE_OBJECT*/   m_pVolumeObject;     // 卷设备对象指针
    u64/*PDEVICE_OBJECT*/   m_pDiskObject;        // 磁盘名称
    u32                     m_SectorByteSize;

    PLIST_ENTRY             m_pReadTaskHead;
    PKSPIN_LOCK             m_pReadTaskHeadLock;
    VOID*                 m_pReadRequstEvent;
}REQ_FSD2FSLD_REGISTER ,    *PREQ_FSD2FSLD_REGISTER;

typedef struct _REQ_FSD2FSLD_UNREGISTER   
{
    u32             m_SerialNumber;               
}REQ_FSD2FSLD_UNREGISTER , * PREQ_FSD2FSLD_UNREGISTER;

typedef struct _REQ_FSD2FSLD_SENDDATA
{
    VOID*  m_pLogInfo;//when use , cast into PLogInfoHead
    PRECORDSTRUCT m_pRecordStruct;
}REQ_FSD2FSLD_SENDDATA, *PREQ_FSD2FSLD_SENDDATA;

typedef struct _REQ_FSD2MBD_SENDDATA
{
    VOID*  m_pLogInfo;//when use , cast into PLogInfoHead
    u32 SerialNumb;
    u32 OpType;           //0:redo       1:undo
}REQ_FSD2MBD_SENDDATA, *PREQ_FSD2MBD_SENDDATA;

typedef struct _RESP_FSD2FSLD_SENDDATA
{
    LSN    TopOpLsn ;
}RESP_FSD2FSLD_SENDDATA, *PRESP_FSD2FSLD_SENDDATA;

typedef struct _REQ_MFT_AREA
{
    u64 mftstart;
    u64 mftend;
}REQ_MFT_AREA, *PREG_MFT_AREA;

typedef struct _REQ_MFT_VOLS
{
    u32 partvolnumb;
    REQ_MFT_AREA mftarea[2];                   //SysVol and one DataVol
}REQ_MFT_VOLS, *PREQ_MFT_VOLS;

typedef struct _REQ_FSD2DISKPERF_REGISTER
{
    PVOID VolDeviceObject;
    PVOID DiskDeviceObject;
    u32   SectorSize;
    REQ_MFT_VOLS mftvols;
}REQ_FSD2DISKPERF_REGISTER, *PREQ_FSD2DISKPERF_REGISTER;

typedef struct _REQ_FSD2SCMD_FS_UNREGISTER   
{
    ULONG             uParSeriesNo;               
}REQ_FSD2SCMD_FS_UNREGISTER , *PREQ_FSD2SCMD_FS_UNREGISTER;

//change access mode

typedef struct _REQ_APP2FSD_ACCESSMODE_CHANGE
{
    HANDLE_DEF_UK(    hFile);
    u8                uAccessMode;
}REQ_APP2FSD_ACCESSMODE_CHANGE, *PREQ_APP2FSD_ACCESSMODE_CHANGE;
typedef struct _RESP_APP2FSD_CACHEMODE_CHANGE 
{
    ULONG             dummy;
}RESP_APP2FSD_ACCESSMODE_CHANGE, *PRESP_APP2FSD_ACCESSMODE_CHANGE;

typedef struct _REQ_FSD2SCM_ACCESSMODE_CHANGE 
{
    ULONG             uVolSerialNo; 
    __int64           uPhyFileId;
    u32               uSsmFileId; 
    u32               uScmFileId; 
    u8                uAccessMode;
}REQ_FSD2SCM_ACCESSMODE_CHANGE, *PREQ_FSD2SCM_ACCESSMODE_CHANGE;
typedef struct _RESP_FSD2SCM_CACHEMODE_CHANGE 
{
    u8              uCacheMode;
    BOOLEAN         bQueryFiOnReachEnd;
    __int64         AllocationSize; //The number of byes allocated
    __int64         FileSize;  
    __int64         ValidDataSize;  
    int             nEntryNumber ;//the number of Run_List_Element
    PRun_List_Element_DEF( run_List_Element);
}RESP_FSD2SCM_ACCESSMODE_CHANGE, *PRESP_FSD2SCM_ACCESSMODE_CHANGE;

typedef struct _REQ_SCM2SSM_ACCESSMODE_CHANGE
{
    ULONG             uVolSerialNo; 
    __int64           uPhyFileId;
    u32               uSsmFileId; 
    u32               uScmFileId; 
    u8                uAccessMode;
}REQ_SCM2SSM_ACCESSMODE_CHANGE, *PREQ_SCM2SSM_ACCESSMODE_CHANGE;
typedef struct _RESP_SCM2SSM_ACCESSMODE_CHANGE
{
    u8              uCacheMode;
    BOOLEAN         bQueryFiOnReachEnd;
    __int64         AllocationSize; //The number of byes allocated
    __int64         FileSize;  
    __int64         ValidDataSize;  
    int             nEntryNumber ;//the number of Run_List_Element
    PRun_List_Element_DEF( run_List_Element);
}RESP_SCM2SSM_ACCESSMODE_CHANGE, *PRESP_SCM2SSM_ACCESSMODE_CHANGE;


//change cache mode
typedef struct _REQ_SSM2SCM_CACHEMODE_CHANGE
{
    ULONG           uVolSerialNo;
    ULONG           uFcb;
    ULONG           uBlank;
    u8              uCacheMode;
    BOOLEAN         bQueryFiOnReachEnd; 
    __int64         AllocationSize; //The number of byes allocated
    __int64         FileSize;  
    __int64         ValidDataSize; 
	BOOLEAN         bSimpleRename;
	DWORD			dwFullNameSize;
    ULONG           PhyIDNum;
    int             nEntryNumber ;//the number of Run_List_Element
    PU64_DEF(       PhyIDList);
    PRun_List_Element_DEF( run_List_Element);
}REQ_SSM2SCM_CACHEMODE_CHANGE, *PREQ_SSM2SCM_CACHEMODE_CHANGE;
typedef struct _RESP_SSM2SCM_CACHEMODE_CHANGE
{
    ULONG           dummy;
}RESP_SSM2SCM_CACHEMODE_CHANGE, *PRESP_SSM2SCM_CACHEMODE_CHANGE;

//change cache mode
typedef struct _REQ_SSM2SCM_DIRCACHE_CHANGE
{
	ULONG           uVolSerialNo;
	ULONG           PhyID;
	u8              uCacheMode;
	DWORD			dwFullNameSize;

	LARGE_INTEGER  CreationTime;
	LARGE_INTEGER  LastAccessTime;
	LARGE_INTEGER  LastWriteTime;
	LARGE_INTEGER  ChangeTime;
	LARGE_INTEGER  EndOfFile;
	LARGE_INTEGER  AllocationSize;
	ULONG  FileAttributes;
	ULONG  FileNameLength;

//	FILE_DIRECTORY_INFORMATION fChildDir;
}REQ_SSM2SCM_DIRCACHE_CHANGE, *PREQ_SSM2SCM_DIRCACHE_CHANGE;
typedef struct _RESP_SSM2SCM_DIRCACHE_CHANGE
{
	ULONG           dummy;
}RESP_SSM2SCM_DIRCACHE_CHANGE, *PRESP_SSM2SCM_DIRCACHE_CHANGE;

typedef struct _REQ_SCM2FSD_CACHEMODE_CHANGE 
{
    ULONG           uVolSerialNo;
    ULONG           uFcb;
    ULONG           uBlank;
    u8              uCacheMode;
    BOOLEAN         bQueryFiOnReachEnd;
    __int64         AllocationSize; //The number of byes allocated
    __int64         FileSize;  
    __int64         ValidDataSize; 
	BOOLEAN         bSimpleRename;
	DWORD			dwFullNameSize;
    ULONG           PhyIDNum;
    int             nEntryNumber ;//the number of Run_List_Element
    PU64_DEF(       PhyIDList);
    PRun_List_Element_DEF( run_List_Element);
}REQ_SCM2FSD_CACHEMODE_CHANGE, *PREQ_SCM2FSD_CACHEMODE_CHANGE;
typedef struct _RESP_SCM2FSD_CACHEMODE_CHANGE 
{
    ULONG           dummy;
}RESP_SCM2FSD_CACHEMODE_CHANGE, *PRESP_SCM2FSD_CACHEMODE_CHANGE;


typedef struct _SET_SSM2FSD_FILEDELETEONCLOSE
{
    HANDLE_DEF_UK( hFile);
}SET_SSM2FSD_FILEDELETEONCLOSE, *PSET_SSM2FSD_FILEDELETEONCLOSE;

typedef struct _RESP_SSM2FSD_FILEDELETEONCLOSE
{
    ULONG           dummy;
}RESP_SSM2FSD_FILEDELETEONCLOSE, *PRESP_SSM2FSD_FILEDELETEONCLOSE;

typedef struct _REQ_SSM2SCM_VOTESWITCH
{
    ULONG           nameSize;
	TCHAR            ipName[100];
}REQ_SSM2SCM_VOTESWITCH, *PREQ_SSM2SCM_VOTESWITCH;

typedef struct _RESP_SSM2SCM_VOTESWITCH
{
    BOOLEAN         bSwitch;
}RESP_SSM2SCM_VOTESWITCH, *PRESP_SSM2SCM_VOTESWITCH;

//typedef struct _FILE_FS_SIZE_INFORMATION {
//    LARGE_INTEGER   TotalAllocationUnits;
//    LARGE_INTEGER   AvailableAllocationUnits;
//    ULONG           SectorsPerAllocationUnit;
//    ULONG           BytesPerSector;
//} FILE_FS_SIZE_INFORMATION, *PFILE_FS_SIZE_INFORMATION;

typedef struct _REQ_FSD2FSLD_READ_DATA
{
    IN LARGE_INTEGER   l64Offset;
    IN PVOID           pBuffer;
    IN ULONG           Length;

    OUT ULONG       statusRet;

    LIST_ENTRY      item;

#if (!defined(DIFS_USERMODE_COMPILE) && !defined(FI_LINUX) && !defined(FI_MAC))
	KEVENT          completeEvent;
#else
	HANDLE_DEF_UK(  completeEvent);
#endif
    
}REQ_FSD2FSLD_READ_DATA, *PREQ_FSD2FSLD_READ_DATA;


typedef struct _RESP_FSD2FSLD_READ_DATA
{
    ULONG       uNumberOfBytesRead;
}RESP_FSD2FSLD_READ_DATA, *PRESP_FSD2FSLD_READ_DATA;

#pragma pack(pop)
#endif

