
#include "OSUtility.h"

#if defined(_WIN32) || defined(_WIN64)

#pragma comment (lib,"Ws2_32.lib")
#else
	#include <unistd.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <net/if.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

	#include <sys/types.h> 
	#include <sys/param.h> 

	#include <net/if_arp.h> 

	#ifdef _AK_FOR_SOLARIS_
		#include <sys/sockio.h> 
	#endif

	#ifdef FI_LINUX
		#include <sys/time.h>
	#else
		#include <time.h>
		#include <mach-o/dyld.h>
	#endif

	//
	#include <sys/stat.h>
	#include <memory.h>
	#include <dirent.h>
	#include <netdb.h>
#endif

#ifndef MAX_PATH
    #define MAX_PATH 255
#endif

CStdString GetErrMsg(int nerr )
{
#if defined(_WIN32) || defined(_WIN64)
	CStdString str;
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);

	str = CStdString((LPCTSTR) lpMsgBuf);
	// Free the buffer.
	LocalFree( lpMsgBuf );
	return str;
#else
    return strerror(nerr);
#endif
}


void OSZeroMemory(void * pMem , int nsize)
{
    memset(pMem , 0 ,nsize);
}

#if !(defined(_WIN32) || defined(_WIN64))
long long GetMicroSecTime()
{
    struct timeval tpstart;
    gettimeofday(&tpstart,0);

    return(tpstart.tv_sec * 1000 * 1000 + tpstart.tv_usec);
}

long long GetMilliSecTime()
{

	struct timeval tpstart;
	gettimeofday(&tpstart,0);

	return(tpstart.tv_sec * 1000  + (tpstart.tv_usec)/1000);
}

int 	GetLastError()
{
    return errno;
}

DWORD GetTickCount()
{
		DWORD dwTickCount = 0 ;
	float fTickCount = 0.0 ;
	struct timeval now_time;

	if(-1 == gettimeofday(&now_time, 0))
	{
		printf("gettimeofday return False, errcode \n");
		ASSERT(0);
		return 0;
	}

	fTickCount = (float)now_time.tv_usec / 1000.0;

	dwTickCount = now_time.tv_sec * 1000 + (DWORD)fTickCount;
	
	return dwTickCount ;
}

bool QueryPerformanceCounter(LARGE_INTEGER *tp)
{
	tp->QuadPart = GetMicroSecTime();

	return true;
}
#endif

int FiGetPid()
{
#if defined(_WIN32) || defined(_WIN64)
	return GetCurrentProcessId();
#else
	return getpid();
#endif
}

OSTHREAD_ID GetThreadId()
{
#if defined(_WIN32) || defined(_WIN64)
	return GetCurrentThreadId();
#else
	return pthread_self();
#endif
}


ULONGLONG GetTickCounts()
{
	ULONGLONG ulTickCount = 0 ;
#if defined(_WIN32) || defined(_WIN64)
	ulTickCount = GetTickCount64();
#else
  
    double fTickCount = 0.0 ;
    struct timeval now_time;

    if (-1 == gettimeofday(&now_time, 0))
    {
        printf("gettimeofday return False, errcode \n");
        ASSERT(0);
        return 0;
    }

    fTickCount = (double)now_time.tv_usec / 1000.0;

    ulTickCount = now_time.tv_sec * 1000 + (ULONGLONG)fTickCount;
#endif
    return ulTickCount ;
}


CStdString GetCurExeMainPath()
{
    CStdString strPath = _T("");
    TCHAR *chPath = new TCHAR[1024];
    u32 size = 1024*sizeof(TCHAR);

    OSZeroMemory(chPath,size);
#if defined(_WIN32) || defined(_WIN64)
	TCHAR *szMainPath = NULL;

	if (0 == GetModuleFileName(NULL,szMainPath,1024 - 1)  ) 
	{
	    
	}else
	{
		strPath = szMainPath ;
	}
#else

	#ifdef __macosx__
		_NSGetExecutablePath(chPath,&size);
		strPath = chPath;
	#else
		int count = readlink("/proc/self/exe", chPath, size);

		if (count < 0 || count >= size)
		{
			printf("Current System Not Surport Proc.\n");

			strPath = "";
		}
		else
		{
			int nIndex = count - 1;

			while (nIndex >= 0 && chPath[nIndex] != '/')
			{
				chPath[nIndex] = 0;

				nIndex--;
			}

			strPath = chPath;
		}
	#endif	
		delete[] chPath;
		chPath = NULL;
#endif
    return strPath ;
}

CStdString GetExeDir()
{
    int pos = -1;
    CStdString strPDir(_T(""));

    CStdString strP = GetCurExeMainPath();
    pos = strP.ReverseFind(SLASHCH);

    if (-1 != pos && 0 != pos)
    {
        strPDir = strP.substr(0, (pos+1));
    }

    return strPDir;
}

int MyGetComputerName(CStdString & cpname)
{
    DWORD cb = 0 ;

    cb =MAX_PATH +1;
    char chHost[MAX_PATH+1];
    OSZeroMemory(chHost,MAX_PATH+1);
    int nLen = MAX_PATH+1;

    int nRet = gethostname(chHost,nLen);
    if (nRet)
        return nRet;

    cpname.Format(_T("%s"),chHost);
    return nRet;
}

int GetRandNumber(int cy)
{
    int number;
    time_t t;
    srand((unsigned) time(&t));
    for (int i = 0;i < cy;i ++)
    {
        number = rand() % 9;
    }
    return number;
}


CStdString ConvertIPtoString(const DWORD &dwIP)
{
    CStdString strIP = _T("");

    unsigned char *pIP = (unsigned char*)&dwIP;
    strIP.Format(_T("%u.%u.%u.%u"), *(pIP), *(pIP+1), *(pIP+2), *(pIP+3));

    return strIP;
}

DWORD ConvertIPtoDWORD(CStdString &strIp)
{
#if defined(_WIN32) || defined(_WIN64)
	
	char ansi[64];
	memset(ansi, 0, 64);

	int len = UTF16ConvertANSI((WCHAR*)(LPCTSTR)strIp, strIp.GetLength(), ansi, 64);
	ASSERT(len);

	return inet_addr(ansi);	
#else
    return inet_addr(strIp.c_str());
#endif
}

bool InvertStringToGuid(const TCHAR *guidStr, GUID *pGuid)
{
    unsigned long Data4H = 0;
    unsigned long Data4L = 0;
#if defined(_WIN32) || defined(_WIN64)

    _stscanf(guidStr, _T("%08x%04x%04x%08x%08x"), (unsigned int*)&(pGuid->Data1), (unsigned int*)&(pGuid->Data2), 
           (unsigned int*)&(pGuid->Data3), (unsigned int*)&Data4H, (unsigned int*)&Data4L);
#else
	sscanf(guidStr, _T("%08x%04x%04x%08x%08x"), (unsigned int*)&(pGuid->Data1), (unsigned int*)&(pGuid->Data2), 
		(unsigned int*)&(pGuid->Data3), (unsigned int*)&Data4H, (unsigned int*)&Data4L);
#endif
    *((long *)pGuid->Data4) = Data4H;
    *((long *)pGuid->Data4+1) = Data4L;

    return true;
}

CStdString InvertGUIDtoString(GUID &guid)
{
    CStdString guidStr = _T("");

    guidStr.Format(_T("%x-%x-%x-%llx"), guid.Data1, guid.Data2,
            guid.Data3, *(u64*)guid.Data4);

    return guidStr;
}


void replacesrc(char *source,char *sub,char *rep)
{
	char* pc1, *pc2, *pc3;
	int isource,isub,irep;
	isub=strlen(sub);
	irep=strlen(rep);
	isource=strlen(source);

	ASSERT(isub == irep);

	if(0 == *sub)
	{
		ASSERT(0);
	};

	pc1=source;

	while(*pc1 != 0)
	{
		pc2 = pc1;
		pc3 = sub;
		/******************************************************/
		while(*pc2 == *pc3 && *pc3 != 0 && *pc2 != 0)
			pc2++,pc3++;
		if(0== *pc3)
		{
			pc3 = rep;
		
			while(*pc3 != 0)
			{
				*pc1 = *pc3;
				pc1++;
				pc3++;
			}

			pc2--;
			pc1 = pc2;

		}
		pc1++;
	}
}

void GetLocalIP(vector<DWORD>& vecIpAddr)
{
#if defined(_WIN32) || defined(_WIN64)
	char szHostName[128]; 

	if(gethostname(szHostName, 128) != 0 ) 
	{
		return;
	}

	struct hostent * pHost; 
	pHost = gethostbyname(szHostName); 

	for (int i=0; (pHost != NULL) && (pHost->h_addr_list[i] != NULL); i++) 
	{ 
		vecIpAddr.push_back(((struct in_addr *)*pHost->h_addr_list)->s_addr);
	}
#else

    int s;
    struct ifconf conf;
    struct ifreq *ifr;
    char buff[BUFSIZ];
    int num;
    int i;
       s = socket(PF_INET, SOCK_DGRAM, 0);

    conf.ifc_len = BUFSIZ;
    conf.ifc_buf = buff;

    ioctl(s, SIOCGIFCONF, &conf);
    num = conf.ifc_len / sizeof(struct ifreq);
    ifr = conf.ifc_req;

    for (i = 0; i < num; i++)
    {
		struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

		ioctl(s, SIOCGIFFLAGS, ifr);
		if (((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
		{
			printf("%s (%s)/n",
			   ifr->ifr_name,
			   inet_ntoa(sin->sin_addr));
			if (0 != sin->sin_addr.s_addr)
			{
			vecIpAddr.push_back(sin->sin_addr.s_addr);
			}
		}
		ifr++;
    }

#endif
}

	

int GetIpMacAddr(DWORD ip, unsigned char *buf, int buflen)
{
	int r = -1;
#if defined(_WIN32) || defined(_WIN64)
#else


    int s;
    struct ifconf conf;
    struct ifreq *ifr;
    char buff[BUFSIZ];
    int num;
    int i;

    s = socket(PF_INET, SOCK_DGRAM, 0);
    conf.ifc_len = BUFSIZ;
    conf.ifc_buf = buff;

    ioctl(s, SIOCGIFCONF, &conf);
    num = conf.ifc_len / sizeof(struct ifreq);
    ifr = conf.ifc_req;

    for (i = 0; i < num; i++)
    {
	struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

	ioctl(s, SIOCGIFFLAGS, ifr);
	if (((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
	{
	    if (ip == sin->sin_addr.s_addr)
	    {
		struct ifreq ihd;
		int fd2 = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

		strcpy(ihd.ifr_name, ifr->ifr_ifrn.ifrn_name);
		if (0 == ioctl(fd2, SIOCGIFHWADDR, &ihd))
		{
		    char ret[13];
		    for (i = 0; i < 6; ++i)
		    {
			snprintf(ret+i*2,13-i*2,"%02x",(unsigned char) ihd.ifr_addr.sa_data[i]);
			buf[i] = ihd.ifr_addr.sa_data[i];
		    }
		    printf("Get ip[%s] mac[%s] %s\n", ConvertIPtoString(ip).c_str(), buf, ret);

		    r = 0;
		}
		close(fd2);
	    }
	}
	ifr++;
    }

    close(s);
#endif
    return r;
}

void DumpMemHex(const unsigned char* start, int len)
{
	int row, col, i = 0;

	for (row = 0; (i + 1) < len; row++)
	{
		for (col = 0; col < 16; col++)
		{
			i = row * 16 + col;

			if (col == 8) 
			{
				printf(" ");
			}

			if (i < len) 
			{
				printf("%02X", start[i]);
			}
			else 
			{
				printf("  ");
			}

			printf(" ");
		}

		printf(" ");

		for (col = 0; col < 16; col++)
		{
			i = row * 16 + col;

			if (col == 8){
				printf(" ");
			}

			if (i < len)
			{
				if (start[i] > 0x20 && start[i] < 0x7F) //A-Z
				{
					printf("%c", start[i]);
				}
				else 
				{
					printf(".");
				}
			}
			else 
			{
				printf(" ");
			}
		}
		printf("\n");
	}
}

u64 GetIpMacInt(DWORD ip)
{
    u64 imac = 0;
    ASSERT(ip);

    unsigned char b[6];
    memset(b, 0, 6);

    if (0 == GetIpMacAddr(ip, b, 6))
    {
       imac = u64(b[5]) << 40 | u64(b[4]) << 32 | u64(b[3]) << 24 |u64(b[2]) << 16 | u64(b[1]) << 8 | u64(b[0]); 
    }

    return imac;
}

#if defined(_WIN32) || defined(_WIN64)
// utf8 ->unicode
BOOL UTF8ConvertUnicode(char *pInBuf,DWORD InBufLen, WCHAR* pOutBuf, DWORD dwOutBufLen)  
{
	DWORD dwRet = 0;
	DWORD err;
	ASSERT(pInBuf);
	ASSERT(pOutBuf);
	ASSERT(dwOutBufLen);
	//ASSERT(dwOutBufLen >= InBufLen);

	if(InBufLen	> 0)
	{
		dwRet = MultiByteToWideChar (CP_UTF8, 0, pInBuf, InBufLen, pOutBuf, dwOutBufLen);
		err=GetLastError();
		ASSERT(dwRet);
	}

	return  dwRet == 0 ? FALSE : TRUE;
}

// unicode->utf8
// poutbuf alloced mem beyond this function
// outbuflen > inbuflen, outbuflen = 2 * inbuflen is the best;
BOOL UnicodeConvertUTF8(WCHAR *pInBuf,DWORD InBufLen, char* pOutBuf, DWORD dwOutBufLen) 
{
	DWORD dwRet = 0;
	DWORD err;
	ASSERT(pInBuf);
	ASSERT(pOutBuf);
	ASSERT(dwOutBufLen);
	ASSERT(dwOutBufLen >= InBufLen);

	if(InBufLen > 0)
	{
		dwRet = WideCharToMultiByte (CP_UTF8,NULL,pInBuf,InBufLen,pOutBuf,dwOutBufLen,NULL,FALSE);
		err=GetLastError();
		ASSERT(dwRet);
	}
	return dwRet == 0 ? FALSE : TRUE;
}

// unicode to ansi
DWORD UTF16ConvertANSI(WCHAR *pInBuf, DWORD InBufLen, char* pOutBuf, DWORD dwOutBufLen)
{
	DWORD dwRet = 0;
	//预转换，得到所需空间的大小，这次用的函数和上面名字相反
	DWORD ansiLen = ::WideCharToMultiByte(CP_ACP, NULL, pInBuf, InBufLen, NULL, 0, NULL, NULL);
	
	ASSERT((ansiLen + 1) <= dwOutBufLen);

	//unicode版对应的strlen是wcslen
	dwRet = ::WideCharToMultiByte(CP_ACP, NULL, pInBuf, InBufLen, pOutBuf, ansiLen, NULL, NULL);
	ASSERT(dwRet);

	return ansiLen;
}

int GetCPUNumber()
{
	SYSTEM_INFO siSysInfo; 
	//复制硬件信息到SYSTEM_INFO结构变量
	GetSystemInfo(&siSysInfo); 
	//显示硬件信息  
	return siSysInfo.dwNumberOfProcessors;
}

#endif