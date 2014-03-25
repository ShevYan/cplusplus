/************************************************************************/
/*File: FiLog.cpp                                                       */
/*By:   WangLei                                                         */
/*Date: 2012.12.12                                                      */
/************************************************************************/

#include "FiLog.h"
#include <stdio.h>
#include <assert.h>

#if defined(FILOG_WINDOWS)

#include <direct.h>
#else // linux or mac
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#endif // FILOG_WINDOWS

//
/************************************************************************/
/**	CFiLoger															*/
/************************************************************************/

CFiLogerBase::CFiLogerBase()
{
}

CFiLogerBase::~CFiLogerBase()
{
	if (false == WriteEndMark())
	{
		MCD_PRINTF(MCD_T("[FiLoger] write log file (%s) failed\n"), MCD_2PCSZ(m_strFileName));
	}

	CloseLogFile();
}

void CFiLogerBase::InitLogFile( MCD_STR strName )
{
	MCD_STR strExeName = GetExeName();
	MCD_STR strFileNamePrefix = strExeName + MCD_T("_") + strName;

	m_dwCurLogFileSeq = 0;
	m_i64CurFileSize = 0;

	MCD_STR strLogFolder = MCD_T("log");
	MCD_STR strMainPath = GetExePath();
	strMainPath = strMainPath + strLogFolder;

	m_strFilePath = strMainPath;
	m_strNamePrefix = strFileNamePrefix;

	MCD_STR strFileName = GetValidFileName(m_strFilePath);

#if defined(FILOG_WINDOWS)
	if(0 != _tmkdir(MCD_2PCSZ(strMainPath)) && EEXIST != errno)
#else
	if(0 != mkdir(MCD_2PCSZ(strMainPath), 0777) && EEXIST != errno)
#endif
	{
		MCD_PRINTF(MCD_T("[FiLoger] Create log folder (%s) failed <%d>\n"), MCD_2PCSZ(strMainPath), errno);
		goto __EXIT__;
	}
#if !defined(FILOG_WINDOWS)
	if(0 != chmod(MCD_2PCSZ(strMainPath), 0777))
	{
		MCD_PRINTF(MCD_T("[FiLoger] chmod log folder (%s) failed <%d>\n"), MCD_2PCSZ(strMainPath), errno);
	}
#endif
	if (false == OpenLogFile(m_strFilePath + MCD_T("/") +strFileName))
	{
		MCD_PRINTF(MCD_T("[FiLoger] Open log file (%s) failed\n"), MCD_2PCSZ(m_strFilePath + MCD_T("/") +strFileName));
		goto __EXIT__;
	}

	if (false == WriteStartMark())
	{
		MCD_PRINTF(MCD_T("[FiLoger] Write log file (%s) failed\n"), MCD_2PCSZ(m_strFilePath + MCD_T("/") +strFileName));
		goto __EXIT__;
	}

__EXIT__:

	return;
}

bool CFiLogerBase::OpenLogFile(MCD_STR strFullFileName)
{
	bool bRet = false;
	m_i64CurFileSize = 0;
	MCD_FOPEN(m_pFile, MCD_2PCSZ(strFullFileName), MCD_T("a+"));

	if(m_pFile)
	{
#if ! defined(FILOG_WINDOWS)
		chmod(MCD_2PCSZ(strFullFileName), 0666);
#endif
		MCD_FSEEK(m_pFile, 0L, SEEK_END);
		m_i64CurFileSize = MCD_FTELL(m_pFile);
		bRet = true;
	}

	return bRet;
}

bool CFiLogerBase::WriteStartMark()
{
	MCD_STR strCurTime = GetTimeString();
	MCD_STR strPreName = m_strNamePrefix;
	MCD_CHAR szSeperatorMsg[_LOG_BUF_SIZE_] = {0};
	int nStrLen = 0;
	bool bRet = false;

	MCD_SNPRINTF(szSeperatorMsg, sizeof(szSeperatorMsg), MCD_T("\n\n-------------------------Start(%s) At %s---------------------\n\n"), MCD_2PCSZ(strPreName), MCD_2PCSZ(strCurTime));
	nStrLen = MCD_PSZLEN(szSeperatorMsg);

	bRet = _FileLog(szSeperatorMsg, nStrLen);

	return bRet;
}

bool CFiLogerBase::WriteEndMark()
{
	MCD_STR strCurTime = GetTimeString();
	MCD_STR strPreName = m_strNamePrefix;
	MCD_CHAR szSeperatorMsg[_LOG_BUF_SIZE_] = {0};
	int nStrLen = 0;
	bool bRet = false;

	MCD_SNPRINTF(szSeperatorMsg, sizeof(szSeperatorMsg), MCD_T("\n\n-------------------------Finish(%s) At %s---------------------\n\n"), MCD_2PCSZ(strPreName), MCD_2PCSZ(strCurTime));
	nStrLen = MCD_PSZLEN(szSeperatorMsg);

	bRet = _FileLog(szSeperatorMsg, nStrLen);

	return bRet;
}

bool CFiLogerBase::WriteContinueMark(MCD_STR strOldFileName)
{
	MCD_STR strCurTime = GetTimeString();
	MCD_CHAR szSeperatorMsg[_LOG_BUF_SIZE_] = {0};
	int nStrLen = 0;
	bool bRet = false;

	MCD_STRDEL(strOldFileName,(MCD_STRLENGTH(strOldFileName)-MCD_PSZLEN(MCD_T("_ing.log"))), MCD_PSZLEN(MCD_T("_ing")));

	MCD_SNPRINTF(szSeperatorMsg, sizeof(szSeperatorMsg), MCD_T("\n\n-------------------------Continue Log File(%s>) At %s---------------------\n\n"), MCD_2PCSZ(strOldFileName), MCD_2PCSZ(strCurTime));
	nStrLen = MCD_PSZLEN(szSeperatorMsg);

	bRet = _FileLog(szSeperatorMsg, nStrLen);

	return bRet;
}

MCD_STR CFiLogerBase::GetTimeString()
{
	MCD_STR strCurTime = MCD_T("");

#if defined(FILOG_WINDOWS)
	SYSTEMTIME st; 
	GetLocalTime(&st); 

	strCurTime.Format(MCD_T("%04d-%02d-%02d %02d:%02d:%02d"), 
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
#else
	MCD_CHAR szTime[128] = {0};

	time_t tmGmt = {0};
	struct tm* tmBeijing = NULL;
	time(&tmGmt);
	tmBeijing = localtime(&tmGmt);

	MCD_SNPRINTF(szTime, sizeof(szTime), MCD_T("%04d-%02d-%02d %02d:%02d:%02d"), 
		(1900+tmBeijing->tm_year),(1+tmBeijing->tm_mon),tmBeijing->tm_mday,
		tmBeijing->tm_hour,tmBeijing->tm_min,tmBeijing->tm_sec);

	strCurTime = szTime;
#endif
	return strCurTime;
}

MCD_STR CFiLogerBase::GetValidFileName(const MCD_STR& strLogPath)
{
	MCD_STR strFileName = MCD_T("");
	MCD_STR strSuffix = MCD_T("_0_ing.log"); // 
	MCD_STR strPreName = m_strNamePrefix;
	MCD_STR strTempFileName = MCD_T("");
	//
	int i;
	for (i=0; i<10; i++)
	{
#if defined(FILOG_WINDOWS)
		strSuffix.SetAt(1, MCD_T('0') + i);
#else
		strSuffix[1] = MCD_T('0') + i;
#endif
		strTempFileName = strLogPath + MCD_T("/") + strPreName + strSuffix;
		if (IsFileExist(strTempFileName))
		{
			break;
		}
	}

	if (10 == i)
	{	// 1.no log file at all. 2.error occurred 
		i = 0;
		strTempFileName = strLogPath + MCD_T("/") + strPreName + MCD_T("_0.log");
		MCD_REMOVEFILE(MCD_2PCSZ(strTempFileName));
	}

#if defined(FILOG_WINDOWS)
	strSuffix.SetAt(1, MCD_T('0') + i);
#else
	strSuffix[1] = MCD_T('0') + i;
#endif
	strFileName = strPreName + strSuffix;

	m_dwCurLogFileSeq = i;
	m_strFileName = strFileName;
	return strFileName;
}


void CFiLogerBase::DoLog(const char* lpFileName, int nLine, const MCD_CHAR* lpszFormat, va_list argptr)
{
	CFiContext* pCtxt = CFiContext::GetInstance();

	MCD_CHAR szBuffer[_LOG_BUF_SIZE_] = {0};
	MCD_STR strFileName(lpFileName);
	// fill header
	unsigned int dwSize = GenLogHeader(szBuffer);

	// fill body
	MCD_VSNPRINTF(szBuffer+dwSize, _LOG_BUF_SIZE_-dwSize-1, lpszFormat, argptr);	
	dwSize = MCD_PSZLEN(szBuffer);
	if (MCD_T('\n') == szBuffer[dwSize- 1])
	{
		szBuffer[dwSize - 1] = MCD_T(' ');
	}

	while(dwSize < 80)
	{
		szBuffer[dwSize] = MCD_T(' '); 
		dwSize++;
	}

	MCD_SNPRINTF(szBuffer + dwSize, _LOG_BUF_SIZE_-dwSize-1, MCD_T("--%s(%d)\n"), MCD_2PCSZ(strFileName), nLine);

	dwSize = MCD_PSZLEN(szBuffer);
	// write log 
	if (pCtxt->GetNum(MCD_T("/FiLogCfg/AsyncWriteLog")))
	{
		AsyncWriteLog(szBuffer, dwSize);
	}
	else
	{
		WriteLog(szBuffer, dwSize);
	}
}

bool CFiLogerBase::WriteLog(MCD_CHAR* lpStr, unsigned int dwCount)
{
	CFiContext* pCtxt = CFiContext::GetInstance();
	bool bRet = false;
	// write log to file
	if (pCtxt->GetNum(MCD_T("/FiLogCfg/FileLog")))
	{
		bRet = FileLog(lpStr, dwCount);
	}
	// write log to screen
	if (pCtxt->GetNum(MCD_T("/FiLogCfg/ScreenLog")))
	{
		bRet &= ScreenLog(lpStr, dwCount);
	}

	return bRet;
}

bool CFiLogerBase::FileLog( MCD_CHAR* lpStr, unsigned int dwCount )
{
	bool bRet = false;
	// file size must be smaller than the limit
	if (IsExceedLengthLimit())
	{
		if (false == SwitchLogFile())
		{
			MCD_PRINTF(MCD_T("[FiLoger] Switch Log File Failed \n"));
			bRet = false;
			goto __EXIT__;
		}
	}

	bRet = _FileLog(lpStr, dwCount );

__EXIT__:
	return bRet;
}

bool CFiLogerBase::_FileLog( MCD_CHAR* lpStr, unsigned int dwCount )
{
    	bool bRet = false;

	fseek(m_pFile, 0L, SEEK_END);
	if(1 == fwrite(lpStr, dwCount, 1, m_pFile))
	{ // write file success
		//fflush(m_pFile);
		m_i64CurFileSize += dwCount;
		bRet = true;
	}

	return bRet;
}

bool CFiLogerBase::ScreenLog( MCD_CHAR* lpStr, unsigned int dwCount )
{
	int nRet = MCD_PRINTF(MCD_T("%s"), lpStr);

	return (nRet>0);
}

bool CFiLogerBase::SwitchLogFile()
{
	// rename the full log file, and close it
	MCD_STR strCurTime = GetTimeString();
	MCD_STR strOldFileName = m_strFileName; // with MCD_T("_ing")
	MCD_STR strFileNewName = GetNextFileName(m_strFilePath);	 // with MCD_T("_ing")
	MCD_STR strTemp = MCD_T("");
	MCD_CHAR szSeperatorMsg[_LOG_BUF_SIZE_] = {0};

	// write old file continue mark
	strTemp = strFileNewName;
	MCD_STRDEL(strTemp,(MCD_STRLENGTH(strTemp)-MCD_PSZLEN(MCD_T("_ing.log"))), MCD_PSZLEN(MCD_T("_ing")));
	MCD_SNPRINTF(szSeperatorMsg, sizeof(szSeperatorMsg), MCD_T("\n\n-------------------------Switch Log File To(%s) At %s---------------------\n\n"), MCD_2PCSZ(strTemp), MCD_2PCSZ(strCurTime));
	_FileLog(szSeperatorMsg, MCD_PSZLEN(szSeperatorMsg));

	// rename old file
	strTemp = strOldFileName;
	MCD_STRDEL(strTemp,(MCD_STRLENGTH(strTemp)-MCD_PSZLEN(MCD_T("_ing.log"))), MCD_PSZLEN(MCD_T("_ing")));
	if(0 != MCD_RENAME(MCD_2PCSZ(m_strFilePath+MCD_T("/")+strOldFileName), MCD_2PCSZ(m_strFilePath+MCD_T("/")+strTemp)))
	{
		MCD_PRINTF(MCD_T("[FiLoger] rename log file failed! <%s>\n"), MCD_2PCSZ(strOldFileName));
	}

	// close old file
	CloseLogFile();

	// remove the subsequent log file, if it exists
	// if subsequent file is not existed, remove() will do nothing.
	strTemp = strFileNewName;
	MCD_STRDEL(strTemp, (MCD_STRLENGTH(strTemp)-MCD_PSZLEN(MCD_T("_ing.log"))), MCD_PSZLEN(MCD_T("_ing")));
	if(0 != MCD_REMOVEFILE(MCD_2PCSZ(m_strFilePath+MCD_T("/")+strTemp)))
	{
		//printf(MCD_T("[FiLoger] rename subsequent log file failed! <%s>\n"), MCD_2PCSZ(strTemp));
	}

	// open next log file, if not existed, create one
	if (false == OpenLogFile(m_strFilePath + MCD_T("/") + strFileNewName))
	{
		MCD_PRINTF(MCD_T("[FiLoger] Open new log file failed! <%s>\n"), MCD_2PCSZ(strFileNewName));
		return false;
	}

	if (false == WriteContinueMark(strOldFileName))
	{
		MCD_PRINTF(MCD_T("[FiLoger] write (%s)ContinueMakr failed\n"), MCD_2PCSZ(strFileNewName));
		return false;
	}

	return true;
}

MCD_STR CFiLogerBase::GetNextFileName(const MCD_STR& strLogPath)
{
	// calculate next log sequence number
	MCD_STR strFileName = MCD_T("");
	MCD_STR strPreName = m_strNamePrefix;
	MCD_CHAR szFileName[1024] = {0};
	MCD_SNPRINTF(szFileName, sizeof(szFileName), MCD_T("%s_%d.log"), MCD_2PCSZ(strPreName), GetNextLogFileSeq());
	strFileName = szFileName;

	// if existed, delete it
	if (IsFileExist(strLogPath + strFileName))
	{
		if (0 != MCD_REMOVEFILE(MCD_2PCSZ(strLogPath + strFileName)))
		{
			MCD_PRINTF(MCD_T("[FiLoger] Remove old log file failed!\n"));
		}
	}

	MCD_INSERT(strFileName, (MCD_STRLENGTH(strFileName)-MCD_PSZLEN(MCD_T(".log"))), MCD_T("_ing"));

	m_strFileName = strFileName;
	return strFileName;
}

unsigned int CFiLogerBase::GetNextLogFileSeq()
{
	if (9 == m_dwCurLogFileSeq)
	{
		m_dwCurLogFileSeq = 0;
	}
	else
	{
		m_dwCurLogFileSeq++;
	}

	return m_dwCurLogFileSeq;
}

void CFiLogerBase::CloseLogFile()
{
	if(m_pFile)
	{
		fflush(m_pFile);
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

MCD_STR CFiLogerBase::GetExePath()
{
	MCD_STR strPath = MCD_T("");
	const int size = 1024;
	MCD_CHAR szPath[size] = {0};

#if defined(FILOG_WINDOWS)
	int idx = 0 ;

	DWORD dwSize = GetModuleFileName(NULL,szPath,size - 1) ;
	if (dwSize)
	{
		PathRemoveFileSpec(szPath); //export from shlwapi.lib
		strPath = szPath;
	}
#else
	int count = readlink(MCD_T("/proc/self/exe"), szPath, size);

	if (count < 0 || count >= size)
	{
		MCD_PRINTF(MCD_T("[FiLoger] Current System does Not Support Proc.readlink\n") );
		strPath = MCD_T("");
	}
	else
	{
		int nIndex = count - 1;

		while (nIndex >= 0 && szPath[nIndex] != MCD_T('/'))
		{
			szPath[nIndex] = 0;
			nIndex--;
		}

		strPath = szPath;
	}
#endif
	return strPath;
}

MCD_STR CFiLogerBase::GetExeName()
{
	MCD_STR strName = MCD_T("");
	const int size = 1024;
	MCD_CHAR szPath[size] = {0};

#if defined(FILOG_WINDOWS)
	int idx = 0 ;

	DWORD dwSize = GetModuleFileName(NULL,szPath,size - 1) ;
	if (dwSize)
	{
		strName = szPath;
		strName = strName.Mid(strName.ReverseFind(MCD_T('\\'))+1);
	}
#else
	int count = readlink(MCD_T("/proc/self/exe"), szPath, size);

	if (count < 0 || count >= size)
	{
		MCD_PRINTF(MCD_T("[FiLoger] Current System does Not Support Proc.readlink\n") );
		strName = MCD_T("");
	}
	else
	{
		int nIndex = count - 1;

		while (nIndex >= 0 && szPath[nIndex] != MCD_T('/'))
		{
			nIndex--;
		}

		strName = &szPath[nIndex+1];
	}
#endif
	return strName;
}


bool CFiLogerBase::IsFileExist(MCD_STR strFullFileName)
{
#if defined(FILOG_WINDOWS)
	if (GetFileAttributes(strFullFileName) == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	} 
	else
	{
		return true;
	}
#else
	int hFile = open(MCD_2PCSZ(strFullFileName), O_RDONLY);
	
	if (-1 == hFile) 
	{
		return false;
	}

	close(hFile);
	return true;
#endif
}

long long CFiLogerBase::GetFileSize(MCD_STR strFullFileName)
{
	long long n64FileSize = -1ll;	
#if defined(FILOG_WINDOWS)
	WIN32_FILE_ATTRIBUTE_DATA att = {0};
	LARGE_INTEGER liFileSize = {0};
	if (GetFileAttributesEx(strFullFileName, GetFileExInfoStandard, &att))
	{
		liFileSize.LowPart = att.nFileSizeLow;
		liFileSize.HighPart = att.nFileSizeHigh;
		n64FileSize = liFileSize.QuadPart;
	}
#else
	struct stat StatBuf;
	if(stat(MCD_2PCSZ(strFullFileName), &StatBuf) < 0)
	{
		return n64FileSize;
	}
	else
	{
		n64FileSize = StatBuf.st_size;
	}
#endif
	return n64FileSize;
}

long long CFiLogerBase::GetLogFileSize()
{
	return m_i64CurFileSize;
}

bool CFiLogerBase::IsExceedLengthLimit()
{
	return GetLogFileSize() >= LOG_FILE_SIZE_LIMIT;
}

//////////////////////////////////////////////////////////////////////////

// FiLogerWorkQueue
FiLogerWorkQueue::FiLogerWorkQueue()
{
	m_nRegId = 0;
	// Initialize buffer list
	InitBufList();
	// create work thread
	StartWorkThread();
}

FiLogerWorkQueue::~FiLogerWorkQueue()
{
	// release buffer list
	ClearBufList();
	// safely exit work thread
	m_bExitWorkThd = true;
	m_semTask.Post();
}

FiLogerWorkQueue* FiLogerWorkQueue::GetInstance()
{
	static FiLogerWorkQueue _Instance;

	return &_Instance;
}

int FiLogerWorkQueue::RegisterWorker( _WORK_FUNC_TYPE_ lpWorkerFun )
{
	FiLogerWorkQueue* pInstance = GetInstance();
	assert(MCD_T("Instance cannot be null!") && pInstance);
	int nRegId = -1;

	pInstance->m_mtxWorkFuncLock.Lock();
	nRegId = pInstance->m_nRegId++;
	pInstance->m_mapWorkFunc[nRegId] = lpWorkerFun;
	pInstance->m_mtxWorkFuncLock.Unlock();

	return nRegId;
}

bool FiLogerWorkQueue::PutTask( int nRegId/*=-1*/, MCD_CHAR* pTaskBuf/*=NULL*/, int nLength/*=0*/, bool bWaitIdle/*=false*/ )
{
	bool bRet = false;
	FiLogerWorkQueue* pInstance = GetInstance();
	assert(MCD_T("Instance cannot be null!") && pInstance);
	PWorkItem pWorkItem = (PWorkItem)pInstance->GetIdleBuf();
	COSSemaphore timer; // use sem_timed_wait instead of sleep, because sleep wastes cpu time.

	// If [bWaitIdle==true] the pWorkItem cannot be null.
	// In this case, if idle buffer list is empty, we should wait until get idle successfully.
	while(bWaitIdle)
	{
		if (pWorkItem)
		{
			break;
		}
		else
		{	// try again after 2 millisecond
			timer.TimedWait(2);
			pWorkItem = (PWorkItem)pInstance->GetIdleBuf();
		}
	}
	//
	if (pWorkItem)
	{
		assert(MCD_T("buffer is smaller than data length!") && (sizeof(int)*2+nLength+1 <= _LOG_BUF_SIZE_));
		pWorkItem->nRegId = nRegId;
		pWorkItem->nBufLen = nLength;
		memset(pWorkItem->pTaskBuf, 0, nLength+1);
		memcpy(pWorkItem->pTaskBuf, pTaskBuf, nLength);

		pInstance->PutBusyBuf((MCD_CHAR*)pWorkItem);
		pInstance->m_semTask.Post();

		bRet = true;
	}

	return bRet;
}
#if defined(FILOG_WINDOWS)
void FiLogerWorkQueue::_WorkThread( void* lpParam )
#else
void* FiLogerWorkQueue::_WorkThread( void* lpParam )
#endif
{
	_WORK_FUNC_TYPE_ pWorkFunc = NULL;
	PWorkItem pWorkItem = NULL;
	FiLogerWorkQueue* pInstance = (FiLogerWorkQueue*) lpParam;
	
	while(true)
	{
		pInstance->m_semTask.Wait();
		if (pInstance->m_bExitWorkThd)
		{// safely exit thread
			break;
		}

		pWorkItem = (PWorkItem)pInstance->GetBusyBuf();
		if(pWorkItem)
		{
			pWorkFunc = pInstance->GetWorkFunc(pWorkItem->nRegId);
			if (pWorkFunc)
			{
				pWorkFunc(pWorkItem->pTaskBuf, pWorkItem->nBufLen);
			}

			pInstance->PutIdleBuf((MCD_CHAR*)pWorkItem);
		}
	}

#if !defined(FILOG_WINDOWS)
	return NULL;
#endif
}

void FiLogerWorkQueue::StartWorkThread()
{
#if defined(FILOG_WINDOWS)
	m_ThreadId = (HANDLE)_beginthread(_WorkThread, 0, this);
#else
	int r = 0;
	pthread_attr_t ThreadAttr;
	m_bExitWorkThd = false;

	pthread_attr_init(&ThreadAttr);
	pthread_attr_setdetachstate(&ThreadAttr, PTHREAD_CREATE_DETACHED);

	r = pthread_create(&m_ThreadId, &ThreadAttr, _WorkThread, (void*)this);

	pthread_attr_destroy(&ThreadAttr);
#endif
}

MCD_CHAR* FiLogerWorkQueue::GetIdleBuf()
{
	MCD_CHAR* pBuf = NULL;
	m_mtxBufListLock.Lock();
	if (m_logIdleList.size() > 0)
	{
		pBuf = m_logIdleList.front();
		m_logIdleList.pop_front();
	}
	else
	{
		pBuf = NULL;
	}
	m_mtxBufListLock.Unlock();

	return pBuf;
}

bool FiLogerWorkQueue::PutIdleBuf( MCD_CHAR* lpBuf )
{
	bool bRet = false;
	m_mtxBufListLock.Lock();
	if(lpBuf)
	{
		m_logIdleList.push_back(lpBuf);
		bRet = true;
	}
	m_mtxBufListLock.Unlock();

	return bRet;
}

MCD_CHAR* FiLogerWorkQueue::GetBusyBuf()
{
	MCD_CHAR* pBuf = NULL;
	m_mtxBufListLock.Lock();

	if(m_logBusyList.size() > 0)
	{
		pBuf = m_logBusyList.front();
		m_logBusyList.pop_front();
	}
	else
	{
		pBuf = NULL;
	}

	m_mtxBufListLock.Unlock();

	return pBuf;
}

bool FiLogerWorkQueue::PutBusyBuf( MCD_CHAR* lpBuf )
{
	bool bRet = false;
	m_mtxBufListLock.Lock();
	if(lpBuf)
	{
		m_logBusyList.push_back(lpBuf);
		bRet = true;
	}
	m_mtxBufListLock.Unlock();

	return bRet;
}

bool FiLogerWorkQueue::InitBufList()
{
	bool bRet = false;
	m_pLogBuf = new MCD_CHAR[_LOG_BUF_SIZE_*_LOG_BUF_NUM_]; // alloc buffer for work-list

	if(m_pLogBuf)
	{
		for(int i=0; i<_LOG_BUF_NUM_; i++) // split buffer into smaller ones, add them int idlebufferlist
		{
			PutIdleBuf(m_pLogBuf + i*_LOG_BUF_SIZE_);
		}
		bRet = true;
	}

	return bRet;
}

bool FiLogerWorkQueue::ClearBufList()
{
	bool bRet = false;
	m_mtxBufListLock.Lock();

	m_logBusyList.clear();
	m_logIdleList.clear();

	delete m_pLogBuf;
	m_pLogBuf = NULL;

	bRet = true;

	m_mtxBufListLock.Unlock();

	return bRet;
}

FiLogerWorkQueue::_WORK_FUNC_TYPE_ FiLogerWorkQueue::GetWorkFunc( int nRegId )
{
	_WORK_FUNC_TYPE_ pWorkFunc = NULL;

	m_mtxWorkFuncLock.Lock();
	map<int, _WORK_FUNC_TYPE_>::iterator it = m_mapWorkFunc.find(nRegId);
	if (m_mapWorkFunc.end() != it)
	{
		pWorkFunc = it->second;
	}
	m_mtxWorkFuncLock.Unlock();

	return pWorkFunc;
}

//////////////////////////////////////////////////////////////////////////
// CFiDebugLoger

#if defined(FILOG_WINDOWS)
volatile long CFiDebugLoger::m_lSingleGuard = 0;
#endif

CFiDebugLoger::CFiDebugLoger(MCD_STR strName)
{
#if defined(FILOG_WINDOWS)
	if(1 == InterlockedIncrement(&m_lSingleGuard))
	{
#endif
		CFiLogerBase::InitLogFile(strName);
		m_nLogItemSeq = 0;
		m_nRegId = -1;
		RegisterAsyncWriteWorkFun();
#if defined(FILOG_WINDOWS)
	}
#endif
}

CFiDebugLoger::~CFiDebugLoger()
{

}

CFiDebugLoger* CFiDebugLoger::GetInstance()
{
    static CFiDebugLoger _Instance(MCD_T("DbgLog"));
	return &_Instance;
}

void CFiDebugLoger::AsyncWriteWorkFun( MCD_CHAR* lpStr, int nLength )
{
	CFiDebugLoger* pInstance = CFiDebugLoger::GetInstance();
	assert(MCD_T("CFiUserLoger instance cannot be NULL") && pInstance);

	if(lpStr && (nLength > 0))
	{
		pInstance->WriteLog(lpStr, nLength);
	}
}

bool CFiDebugLoger::RegisterAsyncWriteWorkFun()
{
	m_nRegId = FiLogerWorkQueue::RegisterWorker(AsyncWriteWorkFun);

	return (m_nRegId != -1);
}

void CFiDebugLoger::AsyncWriteLog( MCD_CHAR* lpStr, unsigned int dwCount )
{
	FiLogerWorkQueue::PutTask(m_nRegId, lpStr, dwCount, false);
}

int CFiDebugLoger::GenLogHeader( MCD_CHAR* strLog )
{
#if defined(FILOG_WINDOWS)
	int nThreadId = GetCurrentThreadId();
#else
	int nThreadId = pthread_self();
#endif
	int nProcessId = getpid();
	MCD_STR strExe = GetExeName();
	MCD_STR strTime = GetTimeString();
	long nSeq = GetLogItemSeq();

	MCD_SNPRINTF(strLog, _LOG_BUF_SIZE_-1, MCD_T("[%ld][%s][%s][%d][0x%x]:"), nSeq, MCD_2PCSZ(strTime), MCD_2PCSZ(strExe), nProcessId, nThreadId);

	return MCD_PSZLEN(strLog);
}

long CFiDebugLoger::GetLogItemSeq()
{
#if defined(FILOG_WINDOWS)
	return InterlockedIncrement(&m_nLogItemSeq);
#else
	return __sync_fetch_and_add(&m_nLogItemSeq, 1);
#endif
}
//////////////////////////////////////////////////////////////////////////
// CFiUserLoger

#if defined(FILOG_WINDOWS)
volatile long CFiUserLoger::m_lSingleGuard = 0;
#endif

CFiUserLoger::CFiUserLoger(MCD_STR strName)
{
#if defined(FILOG_WINDOWS)
	if(1 == InterlockedIncrement(&m_lSingleGuard))
	{
#endif
		CFiLogerBase::InitLogFile(strName);
		m_nLogItemSeq = 0;
		m_nRegId = -1;
		RegisterAsyncWriteWorkFun();
#if defined(FILOG_WINDOWS)
	}
#endif
}

CFiUserLoger::~CFiUserLoger()
{

}

CFiUserLoger* CFiUserLoger::GetInstance()
{
	static CFiUserLoger _Instance(MCD_T("UserLog"));
	return &_Instance;
}

void CFiUserLoger::AsyncWriteWorkFun( MCD_CHAR* lpStr, int nLength )
{
	CFiUserLoger* pInstance = CFiUserLoger::GetInstance();
	assert(MCD_T("CFiUserLoger instance cannot be NULL") && pInstance);
	if(lpStr && (nLength > 0))
	{
		pInstance->WriteLog(lpStr, nLength);
	}
}

bool CFiUserLoger::RegisterAsyncWriteWorkFun()
{
	m_nRegId = FiLogerWorkQueue::RegisterWorker(AsyncWriteWorkFun);

	return (m_nRegId != -1);
}

void CFiUserLoger::AsyncWriteLog( MCD_CHAR* lpStr, unsigned int dwCount )
{
	FiLogerWorkQueue::PutTask(m_nRegId, lpStr, dwCount, true);
}

long CFiUserLoger::GetLogItemSeq()
{
#if defined(FILOG_WINDOWS)
	return InterlockedIncrement(&m_nLogItemSeq);
#else
	return __sync_fetch_and_add(&m_nLogItemSeq, 1);
#endif
}


int CFiUserLoger::GenLogHeader( MCD_CHAR* strLog )
{
	MCD_STR strExe = GetExeName();
	MCD_STR strTime = GetTimeString();
	long nSeq = GetLogItemSeq();

	MCD_SNPRINTF(strLog, _LOG_BUF_SIZE_-1, MCD_T("[%ld][%s][%s]:"), nSeq, MCD_2PCSZ(strTime), MCD_2PCSZ(strExe));

	return MCD_PSZLEN(strLog);
}

//////////////////////////////////////////////////////////////////////////
// CVitalLoger
#if defined(FILOG_WINDOWS)
volatile long CVitalLoger::m_lSingleGuard = 0;
#endif

CVitalLoger::CVitalLoger( MCD_STR strName )
{
#if defined(FILOG_WINDOWS)
	if(1 == InterlockedIncrement(&m_lSingleGuard))
	{
#endif
		CFiLogerBase::InitLogFile(strName);
		m_nLogItemSeq = 0;
		m_nRegId = -1;
		RegisterAsyncWriteWorkFun();
#if defined(FILOG_WINDOWS)
	}
#endif
}

CVitalLoger::~CVitalLoger()
{

}

CVitalLoger* CVitalLoger::GetInstance()
{
	static CVitalLoger _Instance(MCD_T("VitalLog"));
	return &_Instance;
}

void CVitalLoger::AsyncWriteWorkFun( MCD_CHAR* lpStr, int nLength )
{
	CVitalLoger* pInstance = CVitalLoger::GetInstance();
	assert(MCD_T("CFiUserLoger instance cannot be NULL") && pInstance);
	if(lpStr && (nLength > 0))
	{
		pInstance->WriteLog(lpStr, nLength);
	}
}

bool CVitalLoger::RegisterAsyncWriteWorkFun()
{
	m_nRegId = FiLogerWorkQueue::RegisterWorker(AsyncWriteWorkFun);

	return (m_nRegId != -1);
}

void CVitalLoger::AsyncWriteLog( MCD_CHAR* lpStr, unsigned int dwCount )
{
	FiLogerWorkQueue::PutTask(m_nRegId, lpStr, dwCount, true);
}

long CVitalLoger::GetLogItemSeq()
{
#if defined(FILOG_WINDOWS)
	return InterlockedIncrement(&m_nLogItemSeq);
#else
	return __sync_fetch_and_add(&m_nLogItemSeq, 1);
#endif
}


int CVitalLoger::GenLogHeader( MCD_CHAR* strLog )
{
#if defined(FILOG_WINDOWS)
	int nThreadId = GetCurrentThreadId();
#else
	int nThreadId = pthread_self();
#endif
	int nProcessId = getpid();
	MCD_STR strExe = GetExeName();
	MCD_STR strTime = GetTimeString();
	long nSeq = GetLogItemSeq();

	MCD_SNPRINTF(strLog, _LOG_BUF_SIZE_-1, MCD_T("[%ld][%s][%s][%d][0x%x]:"), nSeq, MCD_2PCSZ(strTime), MCD_2PCSZ(strExe), nProcessId, nThreadId);

	return MCD_PSZLEN(strLog);
}

//////////////////////////////////////////////////////////////////////////

#if defined(FILOG_WINDOWS)
long IFiLog::m_lSingleGuard = 0;
#endif

IFiLog::IFiLog()
{
#if defined(FILOG_WINDOWS)
	if(1 == InterlockedIncrement(&m_lSingleGuard))
	{
#endif
		m_pTraceLock = new CCritSec;
		assert(m_pTraceLock);
		SetDefaltCfg();
#if defined(FILOG_WINDOWS)
	}
#endif
}

IFiLog::~IFiLog()
{
	// ?? delete m_pTraceLock
}

IFiLog* IFiLog::GetInstance()
{
	static IFiLog siFilog;
	return &siFilog;
}

void IFiLog::FiTrace( const char* lpFileName, int nLine, const MCD_CHAR* lpszFormat, ... )
{
	if(CFiContext::GetNum(MCD_T("/FiLogCfg/DebugLog")))
	{
		va_list args;
		va_start(args, lpszFormat);

		CFiDebugLoger* pDbgLoger = CFiDebugLoger::GetInstance();
		m_pTraceLock->Lock();
		pDbgLoger->DoLog(lpFileName, nLine, lpszFormat, args);
		m_pTraceLock->Unlock();

		va_end(args);
	}
}

void IFiLog::FiUserTrace( const char* lpFileName, int nLine, const MCD_CHAR* lpszFormat, ... )
{
	if(CFiContext::GetNum(MCD_T("/FiLogCfg/UserLog")))
	{
		va_list args;
		va_start(args, lpszFormat);

		CFiUserLoger* pUserLoger = CFiUserLoger::GetInstance();
		// log user information
		m_pTraceLock->Lock();
		pUserLoger->DoLog(lpFileName, nLine, lpszFormat, args);
		m_pTraceLock->Unlock();

		va_end(args);
	}

	// user log should also be logged in debug log
	if(CFiContext::GetNum(MCD_T("/FiLogCfg/DebugLog")))
	{
		va_list args;
		va_start(args, lpszFormat);

		CFiDebugLoger* pDbgLoger = CFiDebugLoger::GetInstance();
		m_pTraceLock->Lock();
		pDbgLoger->DoLog(lpFileName, nLine, lpszFormat, args);
		m_pTraceLock->Unlock();

		va_end(args);
	}
}

void IFiLog::FiVitalTrace( const char* lpFileName, int nLine, const MCD_CHAR* lpszFormat, ... )
{
	va_list args;

	// log vital information
	va_start(args, lpszFormat);
	CVitalLoger* pVitalLoger = CVitalLoger::GetInstance();
	m_pTraceLock->Lock();
	pVitalLoger->DoLog(lpFileName, nLine, lpszFormat, args);
	m_pTraceLock->Unlock();
	va_end(args);
	// vital log should also be logged in debug log
	if(CFiContext::GetNum(MCD_T("/FiLogCfg/DebugLog")))
	{
	    va_start(args, lpszFormat);
	    CFiDebugLoger* pDbgLoger = CFiDebugLoger::GetInstance();
	    m_pTraceLock->Lock();
	    pDbgLoger->DoLog(lpFileName, nLine, lpszFormat, args);
	    m_pTraceLock->Unlock();
	    va_end(args);
	}
}

void IFiLog::SetDefaltCfg()
{
	CFiContext* pCtx = CFiContext::GetInstance();

	pCtx->SetDefault(MCD_T("/FiLogCfg/AsyncWriteLog"), 0);
	pCtx->SetDefault(MCD_T("/FiLogCfg/FileLog"), 0);
	pCtx->SetDefault(MCD_T("/FiLogCfg/ScreenLog"), 1);
	pCtx->SetDefault(MCD_T("/FiLogCfg/UserLog"), 1);
	pCtx->SetDefault(MCD_T("/FiLogCfg/DebugLog"), 1);
}
