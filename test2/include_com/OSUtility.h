#ifndef __OSUTILITY_H__
#define __OSUTILITY_H__


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <string>
#include <vector>

#include "BaseType.h"


#include "StdString.h"

using namespace std;

#ifdef __linux__
#include <errno.h>
#endif

#ifndef _SafeDeletePtr
        #define _SafeDeletePtr(ptr)             if((ptr) !=NULL) {delete (ptr); (ptr) = NULL;}
#endif

#undef  _SafeDeletePtrArr
#define _SafeDeletePtrArr(ptr)  if ((ptr) != NULL) {delete [](ptr); (ptr) = NULL;}

CStdString GetCurExeMainPath();
CStdString GetExeDir();

int MyGetComputerName(CStdString & cpname);

CStdString InvertGUIDToString(GUID & Guid);

void OSZeroMemory(void * pMem , int nsize);

#if !(defined(_WIN32) || defined(_WIN64))
        long long GetMicroSecTime();
        long long GetMilliSecTime();
        int     GetLastError();
		DWORD GetTickCount();
		bool QueryPerformanceCounter(LARGE_INTEGER *tp);
#endif

int FiGetPid();
OSTHREAD_ID GetThreadId();

ULONGLONG GetTickCounts();

CStdString GetErrMsg(int nerr = 0);
CStdString ConvertIPtoString(const DWORD &dwIP);
CStdString InvertGUIDtoString(GUID &guid);

DWORD ConvertIPtoDWORD(CStdString &strIp);
bool InvertStringToGuid(const char *guidStr, GUID *pGuid);

void replacesrc(char *source,char *sub,char *rep);
void GetLocalIP(vector<DWORD> &vecIpAddr);

int GetIpMacAddr(DWORD ip, unsigned char* buf, int buflen);
u64 GetIpMacInt(DWORD ip);
void DumpMemHex(const unsigned char* start, int len);

#if defined(_WIN32) || defined(_WIN64)

        BOOL UTF8ConvertUnicode(char *pInBuf,DWORD InBufLen, WCHAR* pOutBuf, DWORD dwOutBufLen);  
        BOOL UnicodeConvertUTF8(WCHAR *pInBuf,DWORD InBufLen, char* pOutBuf, DWORD dwOutBufLen);
        DWORD UTF16ConvertANSI(WCHAR *pInBuf, DWORD InBufLen, char* pOutBuf, DWORD dwOutBufLen);
        
        int GetCPUNumber();
#endif

class FiExcepction  
{  
public:  

        // ????,???????  
        FiExcepction(string str)
        {
                m_err = str;
        }

        FiExcepction(int errorId)  
        {  
                // ???????????  
                m_errorId = errorId;  
        }  

        // ??????  
        FiExcepction( FiExcepction& myExp)  
        {  
                // ?????????????  
                this->m_errorId = myExp.m_errorId;
                this->m_err             = myExp.m_err;
        }  

        ~FiExcepction()  
        {  
                // ???????????  
        }  

        // ?????  
        int getErrorId()  
        {  
                return m_errorId;  
        }  

        string ShowErrMsg()
        {
                return m_err;
        }

private:      
        // ???  
        string m_err;
        int m_errorId;  
};

#define FSIZE_1K (1024)
#define FSIZE_1M (1024*1024)
#define FSIZE_1G (1024*1024*1024)

template <class T>
class FiParamVerify{
public:
        FiParamVerify(){ }

        ~FiParamVerify(){ }

        static T Align(const T &nSrc, T nAlign, bool bUp = true){
                if (bUp) {
                        T nExtra = (0==nSrc%nAlign && 0!=nSrc) ? 0 : 1;
                        return (nSrc / nAlign + nExtra)*nAlign;
                }else {
                        return nSrc / nAlign * nAlign;
                }
        }

        static T Range(const T &nSrc, T nLow = -1, T nUp = -1){
                if (-1 != nLow && -1 != nUp) {
                        assert(nLow <= nUp);
                }

                if (-1 != nLow && nSrc < nLow) {
                        return nLow;
                }

                if (-1 != nUp && nSrc > nUp) {
                        return nUp;
                }

                return nSrc;
        }
};


#endif
