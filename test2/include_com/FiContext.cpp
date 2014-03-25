/************************************************************************/
/*File: FiContext.cpp                                                   */
/*By:   WangLei                                                         */
/*Date: 2012.12.12                                                      */
/************************************************************************/

#include "FiContext.h"
#include <stdio.h>
#include <assert.h>

#if defined(FICTX_WINDOWS)
//
#else
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#endif
//////////////////////////////////////////////////////////////////////////
CMarkupEx::CMarkupEx()
{
}

CMarkupEx::~CMarkupEx()
{
}

void CMarkupEx::GetLock()
{
	m_lock.Lock();
}

void CMarkupEx::ReleaseLock()
{
	m_lock.Unlock();
}

//////////////////////////////////////////////////////////////////////////

CContextPath::CContextPath()
{

}

CContextPath::CContextPath(MCD_STR strPath)
{
	ParsePath(strPath);
}


CContextPath::~CContextPath()
{

}

void CContextPath::Set(MCD_STR strPath)
{
	Clear();
	ParsePath(strPath);
}

void CContextPath::Append(MCD_STR strAppend)
{
	if (IsValidPath(strAppend))
	{
		m_vPathNodes.push_back(strAppend);
	}
}

void CContextPath::Clear()
{
	m_vPathNodes.clear();
}

bool CContextPath::IsValidPath(MCD_STR strPath)
{
	bool bRet = true;

	if (MCD_STRNPOS != MCD_STRFIND(strPath,MCD_T("//"),0) ||		//  one slash can not be after another
		strPath == MCD_T("/") ||							//  a single slash is not a valid path
		strPath == MCD_T("")
		)
	{
		bRet = false;
	}

	return bRet;
}

void CContextPath::ParsePath(MCD_STR strPath)
{
	size_t nEnd = 0;
	size_t nStart = nEnd;
	if (IsValidPath(strPath))
	{
		while (nStart != MCD_STRNPOS)
		{
			nEnd = MCD_STRFIND(strPath, MCD_T('/'), nStart + 1);
			if (MCD_STRNPOS == nEnd)
			{
				m_vPathNodes.push_back(MCD_STRMID2(strPath,(nStart + 1)));
			}
			else
			{
				m_vPathNodes.push_back(MCD_STRMID(strPath,(nStart + 1), (nEnd - nStart - 1)));
			}

			nStart = nEnd;
		}
	}
}

void CContextPath::RemoveBack()
{
    m_vPathNodes.pop_back();
}

MCD_STR CContextPath::GetPath()
{
	MCD_STR strRet = MCD_T("");
	for (unsigned int i = 0; i < m_vPathNodes.size(); i++)
	{
		if (m_vPathNodes[i][0] != MCD_T('@')) // if first letter is MCD_T('@'), it is a Data, not path
		{
			strRet += MCD_T("/");
			strRet += m_vPathNodes[i];
		}
		else
		{
			assert(MCD_T("@ must appear in the last item!") && (i == m_vPathNodes.size() - 1));
		}
	}

	return strRet;
}

MCD_STR CContextPath::GetData()
{
	MCD_STR strRet = m_vPathNodes.back();

	if (MCD_T('@') == strRet[0])
	{
		return MCD_STR(strRet, 1);
	}
	else
	{
		return MCD_STR(MCD_T(""));
	}
}


//////////////////////////////////////////////////////////////////////////

CFiContext *CFiContext::m_pInstance = NULL;
//  lock for single instance
CCritSec CFiContext::m_SingleInstanceLock;

CFiContext::CFiContext()
{
	m_nUpdateTime = 0;
	m_ThreadId = 0;
#if !defined(FICTX_WINDOWS)
	m_TimerId = NULL;
#endif
	m_bTimerBusyFlag = 0;
}

CFiContext::~CFiContext()
{
	// release contexts
	CMarkupEx * pContext = NULL;
	LockContexts();
	map<MCD_STR, CMarkupEx *>::iterator it = m_contexts.begin();
	while(it != m_contexts.end())
	{
		pContext = it->second;
		delete pContext;
		it++;
	}
	m_contexts.clear();
	UnlockContexts();
#if !defined(FICTX_WINDOWS)
	if(m_TimerId)
	{
		timer_delete(m_TimerId);
	}
#endif
}

void CFiContext::Navigate(CMarkupEx& xml, CContextPath& ctxPathStack, unsigned int nIndex)
{
	MCD_STR strTemp = MCD_T("");
	MCD_STR strPrev = MCD_T("");
	unsigned int nCount = 0;
	MCD_CHAR szIndex[32] = { 0 };

	if (xml.FindChildElem())
	{  // not reach leaf node
		// Note: find_child would set current position to child
		xml.ResetChildPos();       // back to parent
		strTemp = xml.GetTagName();
		xml.IntoElem();

		if (nIndex != 0) // more than one node has identical tag.
		{
			MCD_SPRINTF(szIndex, MCD_T("[%d]"), nIndex + 1); // index for CMarkup is 1-based
			strTemp = strTemp + szIndex; // append index to tag
		}
		ctxPathStack.Append(strTemp);

		while (xml.FindElem())
		{
			strTemp = xml.GetTagName();
			if (strTemp == strPrev)
			{ // if all tags in one grade are the same, index of the tag should be calculated.
				nCount++;
			}

			Navigate(xml, ctxPathStack, nCount);

			strPrev = strTemp;
		}

		ctxPathStack.RemoveBack();
		xml.OutOfElem();
	}
	else
	{	// reach leaf node
		strTemp = xml.GetTagName();
		if (nIndex != 0) // more than one node has identical tag.
		{
			MCD_SPRINTF(szIndex, MCD_T("[%d]"), nIndex + 1); // index for CMarkup is 1-based
			strTemp = strTemp + szIndex; // append index to tag
		}
		ctxPathStack.Append(strTemp);

		strTemp = xml.GetData(); // datum
		//  set
		Set(ctxPathStack.GetPath(), strTemp);

		ctxPathStack.RemoveBack();
		return;
	}
}

void CFiContext::UpdateContext()
{
	CMarkupEx xml;
	MCD_STR strRoot = MCD_T("");
	vector<MCD_STR> vFileNames = ListContextFiles();
	CContextPath ctxPathStack; // stack for traversing the xml
	CMarkupEx *pTarContext = NULL;

	for (unsigned int i = 0; i < vFileNames.size(); i++)
	{
		ctxPathStack.Clear();

		if (xml.Load(vFileNames[i]))
		{
			xml.FindChildElem();
			strRoot = xml.GetTagName();
			pTarContext = GetContext(strRoot);

			if (pTarContext)
			{
				xml.ResetPos();
				Navigate(xml, ctxPathStack);  // start to traverse
			}
			else
			{
				AddContext(vFileNames[i]);
			}
		}
	}

}

vector<MCD_STR> CFiContext::ListContextFiles()
{
	vector<MCD_STR> vFileNames;
	size_t nIdx = MCD_STRNPOS;
	MCD_STR strContextFolder = m_strFolder; // modify the context folder name if necessary
	MCD_STR strPath = GetExePath();// path is ended with MCD_T('/')
	MCD_STR strXmlFile = MCD_T("");
	strPath = strPath + strContextFolder;

#if defined(FICTX_WINDOWS)

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	// Find the first file in the directory.
	hFind = FindFirstFile(strPath + MCD_T("\\*"), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE) 
	{
		strXmlFile = FindFileData.cFileName;
		nIdx = MCD_STRFIND(strXmlFile, MCD_T(".xml"), 0);
		if (nIdx != MCD_STRNPOS &&			// can be found
			nIdx == MCD_STRLENGTH(strXmlFile)-4) 	// test the last 4 letters
		{
			vFileNames.push_back(strPath + strXmlFile);
		}
		// List all the other files in the directory.
		while (FindNextFile(hFind, &FindFileData) != 0) 
		{
			strXmlFile = FindFileData.cFileName;
			nIdx = MCD_STRFIND(strXmlFile, MCD_T(".xml"), 0);
			if (nIdx != MCD_STRNPOS &&			// can be found
				nIdx == MCD_STRLENGTH(strXmlFile)-4) 	// test the last 4 letters
			{
				vFileNames.push_back(strPath + strXmlFile);
			}
		}

		FindClose(hFind);
	}


#else
	DIR *dirInfo = opendir(MCD_2PCSZ(strPath));
	struct dirent *dirEntry;

	if (dirInfo)
	{
		while ((dirEntry = readdir(dirInfo)) != NULL)
		{
			strXmlFile = dirEntry->d_name;
			nIdx = MCD_STRFIND(strXmlFile, MCD_T(".xml"), 0);
			if (nIdx != MCD_STRNPOS &&			// can be found
				nIdx == MCD_STRLENGTH(strXmlFile)-4) 	// test the last 4 letters
			{
				vFileNames.push_back(strPath + strXmlFile);
			}
		}

		closedir(dirInfo);
    }
#endif

	return vFileNames;
}

CMarkupEx* CFiContext::GetContext(MCD_STR strRoot)
{
	CMarkupEx *pContext = NULL;

	LockContexts();

	map<MCD_STR, CMarkupEx *>::iterator it = m_contexts.find(strRoot);

	if (m_contexts.end() != it)
	{
		pContext =  it->second;
	}

	UnlockContexts();

	return pContext;
}

CMarkupEx* CFiContext::AddContext(MCD_STR strFileName)
{
	MCD_STR strRoot = MCD_T("");
	CMarkupEx *pXml = new CMarkupEx;
	assert(MCD_T("[AddContext] failed to new item!") && pXml);

	if (pXml->Load(strFileName))
	{
		pXml->FindChildElem();
		strRoot = pXml->GetTagName();
		pXml->ResetPos();

		LockContexts();
		m_contexts[strRoot] = pXml;
		UnlockContexts();
	}
	else
	{
		delete pXml;
		pXml = NULL;
	}

	return pXml;
}

void CFiContext::Initialize(MCD_STR strFolder, unsigned int uTime, bool bAutoUpdate)
{
	vector<MCD_STR> vFileNames = ListContextFiles();

	for (unsigned int i = 0; i < vFileNames.size(); i++)
	{
		if (GetContext(vFileNames[i]) == NULL)
		{
			AddContext(vFileNames[i]);
		}
	}

	// create auto update thread
	m_nUpdateTime = uTime;
	m_strFolder = strFolder;
	if (bAutoUpdate)
	{
		StartAutoUpdate();
	}
}

bool CFiContext::Add(MCD_STR strPath, MCD_STR strData)
{
	CContextPath ctxPath(strPath);
	bool bRet = false;
	MCD_STR strRoot = ctxPath.m_vPathNodes[0];
	MCD_STR strTemp = MCD_T("");

	CMarkupEx *pContext = GetContext(strRoot);
	if (pContext)
	{
		pContext->GetLock();
		pContext->ResetPos();

		unsigned int i = 0;
		for (i = 0;  i < ctxPath.m_vPathNodes.size(); i++)
		{
			if (pContext->FindElem(ctxPath.m_vPathNodes[i]))
			{
				if (!pContext->IntoElem()) break;
			}
			else
			{	// not find, add and into it
				strTemp = ctxPath.m_vPathNodes[i];
				// firstly, erase index [xx] at the end of the tag
				unsigned int nBegin = MCD_STRFIND2(strTemp, (MCD_T('[')));
				unsigned int nEnd = MCD_STRFIND2(strTemp, (MCD_T(']')));
				if (nEnd > nBegin && nEnd == MCD_STRLENGTH(strTemp) - 1)
				{
					MCD_STRDEL(strTemp, nBegin, (nEnd - nBegin + 1));
				}

				if (!pContext->AddElem(strTemp)) break;
				if (!pContext->IntoElem()) break;
			}
		}

		if (i == ctxPath.m_vPathNodes.size())
		{
			pContext->OutOfElem();
			bRet = pContext->SetData(strData);
		}

		pContext->ReleaseLock();
	}

	return bRet;
}

bool CFiContext::Set(MCD_STR strPath, MCD_STR strData)
{
	CContextPath ctxPath(strPath);
	bool bRet = false;
	MCD_STR strRoot = ctxPath.m_vPathNodes[0];

	CMarkupEx *pContext = GetContext(strRoot);
	if (pContext)
	{
		pContext->GetLock();
		pContext->ResetPos();

		if (pContext->FindElem(strPath))
		{
			bRet = pContext->SetData(strData);
		}
		else
		{ // add path and set data
			bRet = Add(strPath, strData);
		}

		pContext->ReleaseLock();
	}
	else
	{
	pContext = new  CMarkupEx;
	LockContexts();
	m_contexts[strRoot] = pContext;
	UnlockContexts();

	bRet = Add(strPath, strData);
	}

    return bRet;
}

bool CFiContext::Set(CContextPath& ctxPath)
{
	bool bRet = false;

	MCD_STR strPath = ctxPath.GetPath();
	MCD_STR strData = ctxPath.GetData();
	bRet = Set(strPath, strData);

	return bRet;
}

int CFiContext::GetNum(MCD_STR strPath)
{
	CFiContext*	pCtx = GetInstance();
	MCD_STR strRet = pCtx->GetString(strPath);

	return MCD_TTOI(MCD_2PCSZ(strRet));
}

MCD_STR CFiContext::GetString(MCD_STR strPath)
{
	CFiContext*	pCtx = GetInstance();
	MCD_STR strRet = pCtx->_GetString(strPath);
	assert(MCD_T("Empty value is not allowed!") && MCD_STRLENGTH(strRet));

	return strRet;
}

MCD_STR CFiContext::_GetString(MCD_STR strPath)
{
	CContextPath ctxPath(strPath);
	MCD_STR strRet = MCD_T("");
	MCD_STR strRoot = ctxPath.m_vPathNodes[0];

	CMarkupEx *pContext = GetContext(strRoot);
	if (pContext)
	{
		pContext->GetLock();

		pContext->ResetPos();

		if(pContext->FindElem(strPath))
		{
			strRet = pContext->GetData();
		}

		pContext->ReleaseLock();
	}


	return strRet;
}

vector<int> CFiContext::GetNumArr(MCD_STR strPath)
{
	CFiContext*	pCtx = GetInstance();
	vector<int> vNum;
	CContextPath ctxPath(strPath);
	MCD_STR strRoot = ctxPath.m_vPathNodes[0];
	MCD_STR strRet = MCD_T("");

	CMarkupEx *pContext = pCtx->GetContext(strRoot);
	if (pContext)
	{
		pContext->GetLock();
		pContext->ResetPos();

		if (pContext->FindElem(strPath))
		{
			while (pContext->FindChildElem())
			{
				strRet = pContext->GetChildData();
				vNum.push_back(MCD_TTOI(MCD_2PCSZ(strRet)));
			}
		}

		pContext->ReleaseLock();
	}

    return vNum;
}

vector<MCD_STR> CFiContext::GetStringArr(MCD_STR strPath)
{
	CFiContext*	pCtx = GetInstance();
	vector<MCD_STR> vString;
	CContextPath ctxPath(strPath);
	MCD_STR strRoot = ctxPath.m_vPathNodes[0];
	MCD_STR strRet = MCD_T("");

	CMarkupEx *pContext = pCtx->GetContext(strRoot);
	if (pContext)
	{
		pContext->GetLock();
		pContext->ResetPos();

		if (pContext->FindElem(strPath))
		{
			while (pContext->FindChildElem())
			{
				strRet = pContext->GetChildData();
				vString.push_back(strRet);
			}
		}

		pContext->ReleaseLock();
	}

	return vString;
}

vector<MCD_STR> CFiContext::GetTagArr(MCD_STR strPath)
{
	CFiContext*	pCtx = GetInstance();
	vector<MCD_STR> vString;
	CContextPath ctxPath(strPath);
	MCD_STR strRoot = ctxPath.m_vPathNodes[0];
	MCD_STR strRet = MCD_T("");

	CMarkupEx *pContext = pCtx->GetContext(strRoot);
	if (pContext)
	{
		pContext->GetLock();
		pContext->ResetPos();

		if (pContext->FindElem(strPath))
		{
			while (pContext->FindChildElem())
			{
				strRet = pContext->GetChildTagName();
				vString.push_back(strRet);
			}
		}

		pContext->ReleaseLock();
	}

	return vString;
}

int CFiContext::GetTagCount(MCD_STR strPath)
{
	CFiContext*	pCtx = GetInstance();
	int nCount = 0;
	CContextPath ctxPath(strPath);
	MCD_STR strRoot = ctxPath.m_vPathNodes[0];

	CMarkupEx *pContext = pCtx->GetContext(strRoot);
	if (pContext)
	{
		pContext->GetLock();
		pContext->ResetPos();

		if (pContext->FindElem(strPath))
		{
			while (pContext->FindChildElem())
			{
				nCount++;
			}
		}

		pContext->ReleaseLock();
	}

	return nCount;
}

bool CFiContext::SetDefault(MCD_STR strPath, int nVal)
{
	CFiContext*	pCtx = GetInstance();
	bool bRet = false;
	MCD_STR strVal = MCD_T("");
	MCD_CHAR szVal[256] = { 0 };

	MCD_SPRINTF(szVal, MCD_T("%d"), nVal);
	strVal = szVal;
	bRet = pCtx->SetDefault(strPath, strVal);

	return bRet;
}

bool CFiContext::SetDefault(MCD_STR strPath, MCD_STR strVal)
{
	CFiContext*	pCtx = GetInstance();
	bool bRet = false;

	// if value is already existed, the default value will be ignored!
	if (pCtx->_GetString(strPath) == MCD_T(""))
	{
		bRet = pCtx->Set(strPath, strVal);
	}

	return bRet;
}

CFiContext* CFiContext::GetInstance()
{
	m_SingleInstanceLock.Lock();
	static CFiContext _Instance;

	if (NULL == m_pInstance)
	{
		m_pInstance = &_Instance;
		m_pInstance->Initialize();
	}

	m_SingleInstanceLock.Unlock();

	assert(MCD_T("CFiContext instance cannot be NULL") && m_pInstance);

	return m_pInstance;
}

void CFiContext::LockContexts()
{
	m_ctxLock.Lock();
}

void CFiContext::UnlockContexts()
{
	m_ctxLock.Unlock();
}

// path is ended with '/'
MCD_STR CFiContext::GetExePath()
{
	MCD_STR strPath = MCD_T("");
	const int size = 1024;
	MCD_CHAR szPath[size] = {0};

#if defined(FICTX_WINDOWS)
	int idx = 0 ;

	DWORD dwSize = GetModuleFileName(NULL,szPath,size - 1) ;
	if (dwSize)
	{
		PathRemoveFileSpec(szPath); //export from shlwapi.lib
		strPath = szPath;
		strPath += MCD_T("\\");
	}
#else
	int count = readlink(MCD_T("/proc/self/exe"), szPath, size);

	if (count < 0 || count >= size)
	{
		MCD_PRINTF(MCD_T("Current System does Not Support Proc.readlink") );
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

#if defined(FICTX_WINDOWS)
void CFiContext::_AutoUpdateThreadFun( void* lpParam )
{
	CFiContext* pThis = (CFiContext*) lpParam;
	assert(MCD_T("Pointer of instance cannot be null!") && pThis);

	while (true)
	{
		pThis->UpdateContext();
		Sleep(pThis->m_nUpdateTime);
	}

	return ;
}
#else
// NOTE:
// If processing time of timer function is longer than interval,
// this function may be called before previous calling completing.
// error may occur.
void CFiContext::_AutoUpdateTimerFun(union sigval sig)
{
	CFiContext* pThis = (CFiContext*) sig.sival_ptr;
	assert(MCD_T("[CFiContext]Instance pointer cannot be null!") && pThis);

	// Only one update can be called, even if time is up!
	if(__sync_fetch_and_add(&pThis->m_bTimerBusyFlag, 1) == 0)
	{
		pThis->UpdateContext();
	}

	__sync_fetch_and_sub(&pThis->m_bTimerBusyFlag, 1);
}
#endif

int CFiContext::StartAutoUpdate()
{
	int r = 0;
#if defined(FICTX_WINDOWS)
	m_ThreadId = (HANDLE)_beginthread(_AutoUpdateThreadFun, 0, this);
#else
	timer_t timerid = NULL;
	struct sigevent evp;
	struct itimerspec ts;

	memset(&evp, 0, sizeof(evp));
	memset(&ts, 0, sizeof(ts));

	evp.sigev_value.sival_ptr = (void*)this;
	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = _AutoUpdateTimerFun;

	if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1)
	{
		printf(MCD_T("[CFiContext]fail to timer_create <%d>\n"), errno);
		r = 1;
		goto __EXIT__;
	}

	ts.it_interval.tv_sec = m_nUpdateTime;
	ts.it_interval.tv_nsec = 0;
	ts.it_value.tv_sec = 0;
	ts.it_value.tv_nsec = 1; // start time CANNOT be zero. Bug of timer_settime!

	if (timer_settime(timerid, 0, &ts, NULL) == -1)
	{
		printf(MCD_T("[CFiContext]fail to timer_settime <%d>\n"), errno);
		r = 2;
		goto __EXIT__;
	}

__EXIT__:
	if(2 == r)
	{ // delete timer, if timer_settime failed.
		timer_delete(&timerid);
	}
#endif
	return r;
}
