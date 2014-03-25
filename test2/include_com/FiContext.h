/************************************************************************/
/*File: FiContext.h                                                     */
/*By:   WangLei                                                         */
/*Date: 2012.12.12                                                      */
/************************************************************************/

#ifndef _FI_CONTEXT_H_
#define _FI_CONTEXT_H_

#if ! defined(FICTX_WINDOWS)
#if defined(_WIN32) || defined(WIN32)
#define FICTX_WINDOWS
#endif // WIN32 or _WIN32
#endif // not FILOG_WINDOWS
//
//

#include "Markup.h"
#include "OSMutex.h"
#include <vector>
#include <map>

#if defined(FICTX_WINDOWS)
#include <process.h>
#else
#include <pthread.h>
#endif

using namespace std;


class CMarkupEx : public CMarkup
{
public:
		//
		CMarkupEx();
		~CMarkupEx();
		//
		void	GetLock();
		void	ReleaseLock();
private:
		CCritSec m_lock;
};

class CContextPath
{
public:
		CContextPath();
		CContextPath(MCD_STR strPath);
		~CContextPath();
		//
		void				Set(MCD_STR strPath);
		void				Append(MCD_STR strAppend);
		void				Clear();
		void				RemoveBack();
		MCD_STR				GetPath();
		MCD_STR				GetData();
public:
		vector<MCD_STR>		m_vPathNodes;
private:
		bool				IsValidPath(MCD_STR strPath);
		void				ParsePath(MCD_STR strPath);

};

class CFiContext
{
private:
		CFiContext();
		static CFiContext*			m_pInstance;
		static CCritSec		m_SingleInstanceLock;
public:
		//CFiContext();
		~CFiContext();
		//// operations
		static CFiContext*			GetInstance();
		void						UpdateContext();
		// get
		static int					GetNum(MCD_STR strPath);
		static MCD_STR				GetString(MCD_STR strPath);
		static vector<int>			GetNumArr(MCD_STR strPath);
		static vector<MCD_STR>		GetStringArr(MCD_STR strPath);
		static vector<MCD_STR>		GetTagArr(MCD_STR strPath);
		static int					GetTagCount(MCD_STR strPath);
		// set
		static bool					SetDefault(MCD_STR strPath, int nVal);
		static bool					SetDefault(MCD_STR strPath, MCD_STR strVal);
#if defined(FICTX_WINDOWS)
		// thread
		static void				_AutoUpdateThreadFun(void* lpParam);
#else
		static void					_AutoUpdateTimerFun(union sigval sig);
#endif
		int							StartAutoUpdate();
private:
//// members
		map<MCD_STR, CMarkupEx *>	m_contexts; // record context xml files( map<xml_root_name, pointor_to_xml> )
		CCritSec				m_ctxLock;
#if defined(FICTX_WINDOWS)
		HANDLE					m_ThreadId;
#else
		pthread_t					m_ThreadId;
		timer_t						m_TimerId;
#endif
		volatile int				m_bTimerBusyFlag; // if 1, timer function is busy, cannot be called again.
		MCD_STR						m_strFolder;// folder containing configure files
		unsigned int				m_nUpdateTime; // time interval for auto update contexts
		//
		void						Initialize(MCD_STR strFolder=MCD_T(""), unsigned int uTime=20/*seconds*/, bool bAutoUpdate=true);
		//
		void						LockContexts();
		void						UnlockContexts();
		//// operations
		vector<MCD_STR>				ListContextFiles(); // return all xml_files' names
		void						Navigate(CMarkupEx& xml, CContextPath& vPathName, unsigned int nIndex=0);
		bool						Add(MCD_STR strPath, MCD_STR strData);
		bool						Set(CContextPath& ctxPath);
		bool						Set(MCD_STR strPath, MCD_STR strData);
		CMarkupEx*					GetContext(MCD_STR strRoot);
		CMarkupEx*					AddContext(MCD_STR strFileName);
		MCD_STR						GetExePath();
	MCD_STR							_GetString(MCD_STR strPath);
};

#endif
