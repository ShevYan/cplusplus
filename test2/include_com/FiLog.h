/************************************************************************/
/*File: FiLog.h                                                         */
/*By:   WangLei                                                         */
/*Date: 2012.12.12                                                      */
/************************************************************************/

#ifndef _Fi_LOG_H_
#define _Fi_LOG_H_
//
#if ! defined(FILOG_WINDOWS)
#if defined(_WIN32) || defined(WIN32)
#define FILOG_WINDOWS
#endif // WIN32 or _WIN32
#endif // not FILOG_WINDOWS
//
//

#include "FiContext.h"
#include "OSMutex.h"
#include "OSSemaphore.h"

#include <map>
#include <list>

#if defined(FILOG_WINDOWS)
#include <process.h>
#else
#include <pthread.h>
#endif

using namespace std;

#define _LOG_BUF_SIZE_			1024	/*Byte*/
#define _LOG_BUF_NUM_			1024
#define LOG_FILE_SIZE_LIMIT		(50*1024*1024ll)


class FiLogerWorkQueue
{
public:
	//
	typedef void(*_WORK_FUNC_TYPE_)(MCD_CHAR*, int);

	typedef struct _WorkItem
	{
		int nRegId;
		int nBufLen;
		MCD_CHAR pTaskBuf[0];
	}WorkItem, *PWorkItem;
	//
	static FiLogerWorkQueue* GetInstance();
	static int RegisterWorker(_WORK_FUNC_TYPE_ lpWorkerFun);// return value is register_id
	static bool PutTask(int nRegId=-1, MCD_CHAR* pTaskBuf=NULL, int nLength=0, bool bWaitIdle=false);
private:
	FiLogerWorkQueue();
	~FiLogerWorkQueue();
private:
#if defined(FILOG_WINDOWS)
	static void _WorkThread(void* lpParam);	// thread
#else
	static void* _WorkThread(void* lpParam);	// thread
#endif
	void StartWorkThread();

	// 
	MCD_CHAR* GetIdleBuf();
	bool PutIdleBuf(MCD_CHAR* lpBuf);
	MCD_CHAR* GetBusyBuf();
	bool PutBusyBuf(MCD_CHAR* lpBuf);
	bool InitBufList();
	bool ClearBufList();
	// 
	_WORK_FUNC_TYPE_ GetWorkFunc(int nRegId);
private:
	int m_nRegId;
	//
	CCritSec m_mtxWorkFuncLock;
	map<int, _WORK_FUNC_TYPE_> m_mapWorkFunc;
	//
	CCritSec m_mtxBufListLock;
	list<MCD_CHAR*> m_logBusyList;	// work items to handle
	list<MCD_CHAR*> m_logIdleList;	// idle work items
	MCD_CHAR* m_pLogBuf;	// buffer. it is splited into <_LOG_BUF_NUM_> blocks that are managed with bufferlist
	COSSemaphore m_semTask;	// indicate there are work-items in busylist 
	//
#if defined(FILOG_WINDOWS)
	HANDLE m_ThreadId;
#else
	pthread_t m_ThreadId;
#endif
	bool m_bExitWorkThd;
};

class CFiLogerBase
{
public:
	CFiLogerBase();
	virtual ~CFiLogerBase();
	virtual void DoLog(const char* lpFileName, int nLine, const MCD_CHAR* lpszFormat, va_list argptr);
	// synchronic
	bool WriteLog(MCD_CHAR* lpStr, unsigned int dwCount);
	bool FileLog(MCD_CHAR* lpStr, unsigned int dwCount);
	bool ScreenLog(MCD_CHAR* lpStr, unsigned int dwCount);
	// asynchronism
	virtual void AsyncWriteLog(MCD_CHAR* lpStr, unsigned int dwCount) = 0;
	//
	void InitLogFile(MCD_STR strName);

public:
	virtual int GenLogHeader(MCD_CHAR* strLog ) = 0;
	virtual long GetLogItemSeq() = 0;
	MCD_STR GetTimeString();
	MCD_STR GetExeName();
	MCD_STR GetExePath();
	bool IsFileExist(MCD_STR strFullFileName);
	long long GetFileSize(MCD_STR strFullFileName);

private:
	bool OpenLogFile(MCD_STR strFullFileName);
	void CloseLogFile();
	bool SwitchLogFile();

	bool _FileLog( MCD_CHAR* lpStr, unsigned int dwCount );
	bool WriteStartMark();
	bool WriteEndMark();
	bool WriteContinueMark(MCD_STR strFileOldName);

	MCD_STR GetValidFileName(const MCD_STR& strLogPath);
	MCD_STR GetNextFileName(const MCD_STR& strLogPath);

	unsigned int GetNextLogFileSeq();
	long long GetLogFileSize();
	bool	IsExceedLengthLimit();

private:
	FILE*	m_pFile;

	unsigned int	m_dwCurLogFileSeq;		// sequence number for current log item
	long long	m_i64CurFileSize;	// log file size at this moment
	MCD_STR m_strFilePath;		// path of current log file(with out file name)
	MCD_STR m_strFileName;		// current log file name(including "_ing")
	MCD_STR m_strNamePrefix;		// log file name(without seq and "_ing")
};

class CFiDebugLoger: public CFiLogerBase
{
private:
	CFiDebugLoger(MCD_STR strName);
public:
	virtual ~CFiDebugLoger();
public:
	static CFiDebugLoger* GetInstance();
	virtual void AsyncWriteLog(MCD_CHAR* lpStr, unsigned int dwCount);
	static void AsyncWriteWorkFun(MCD_CHAR* lpStr, int nLength);
	virtual long GetLogItemSeq();
	virtual int GenLogHeader(MCD_CHAR* strLog );
private:
	bool RegisterAsyncWriteWorkFun();
private:
	volatile long m_nLogItemSeq;
	int m_nRegId;
#if defined(FILOG_WINDOWS)
	static volatile long m_lSingleGuard;
#endif
};

class CFiUserLoger: public CFiLogerBase
{
private:
	CFiUserLoger(MCD_STR strName);
public:
	virtual ~CFiUserLoger();
public:
	static CFiUserLoger* GetInstance();
	virtual void AsyncWriteLog(MCD_CHAR* lpStr, unsigned int dwCount);
	static void AsyncWriteWorkFun(MCD_CHAR* lpStr, int nLength);
	virtual long GetLogItemSeq();
	virtual int GenLogHeader(MCD_CHAR* strLog );
private:
	bool RegisterAsyncWriteWorkFun();
private:
	volatile long m_nLogItemSeq;
	int m_nRegId;
#if defined(FILOG_WINDOWS)
	static volatile long m_lSingleGuard;
#endif
};

class CVitalLoger: public CFiLogerBase
{
private:
	CVitalLoger(MCD_STR strName);
public:
	virtual ~CVitalLoger();
public:
	static CVitalLoger* GetInstance();
	virtual void AsyncWriteLog(MCD_CHAR* lpStr, unsigned int dwCount);
	static void AsyncWriteWorkFun(MCD_CHAR* lpStr, int nLength);
	virtual long GetLogItemSeq();
	virtual int GenLogHeader(MCD_CHAR* strLog );
private:
	bool RegisterAsyncWriteWorkFun();
private:
	volatile long m_nLogItemSeq;
	int m_nRegId;
#if defined(FILOG_WINDOWS)
	static volatile long m_lSingleGuard;
#endif
};

//
class IFiLog
{
private:
	IFiLog();
public:
	~IFiLog();
	//
	static IFiLog* GetInstance();
	//
	void FiTrace(const char* lpFileName, int nLine, const MCD_CHAR* lpszFormat, ...);
	void FiUserTrace(const char* lpFileName, int nLine, const MCD_CHAR* lpszFormat, ...);
	void FiVitalTrace(const char* lpFileName, int nLine, const MCD_CHAR* lpszFormat, ...);
private:
	void SetDefaltCfg();
private:
	//
	// ???log???????????
	// ?????????????????
	// ??????????????????????
	// ????????????????
	//
	CCritSec* m_pTraceLock;
	//
	// windows????????????vc????????
	// ???????????
	// ???????????????????
	// ??????????????????????
	// ???????????????????????????
	// ???????????????????????
	//
#if defined(FILOG_WINDOWS)
	static long m_lSingleGuard;
#endif
};

//////////////////////////////////////////////////////////////////////////

#define STRACE(...) \
	IFiLog::GetInstance()->FiTrace(__FILE__, __LINE__, ##__VA_ARGS__)

#define UTRACE(...) \
	IFiLog::GetInstance()->FiUserTrace(__FILE__, __LINE__, ##__VA_ARGS__)

#define VTRACE(...)	\
	IFiLog::GetInstance()->FiVitalTrace(__FILE__, __LINE__, ##__VA_ARGS__)

#define NTRACE(...)\
	IFiLog::GetInstance()->FiVitalTrace(__FILE__, __LINE__, ##__VA_ARGS__)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DoTraceMessage(...)\
   IFiLog::GetInstance()->FiVitalTrace(__FILE__, __LINE__, ##__VA_ARGS__)
#endif //_Fi_LOG_H_
