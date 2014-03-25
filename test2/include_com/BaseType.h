#ifndef __BASE_TYPE_H_
#define __BASE_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char			u8 ;
typedef unsigned short			u16 ;
typedef unsigned int			ul32 ;        //64 wei xia long gai wei int
typedef unsigned int       		u32;          //long 

typedef unsigned long long		u64 ;

typedef char					s8;
typedef short					s16 ;
typedef int						s32;
typedef long long				s64;


#if !(defined(_WIN32) || defined(WIN32))

#if defined(__linux__) || defined(__linux)
#if !defined(FI_LINUX)
#define FI_LINUX
#endif // !defined(FI_LINUX)
#else // not __linux__
#if !defined(FI_MAC)
#define FI_MAC
#endif // !defined(FI_MAC)
#endif  // defined(__linux__) || defined(__linux)

#if defined(FI_LINUX) || defined(FI_MAC)
#include <string.h>
#include <assert.h>

#define WINAPI 

// wait event
#define WAIT_FAILED     	-1
#define WAIT_TIMEOUT    	-1
#define WAIT_ABANDONED_0 	-1
#define WAIT_OBJECT_0   	0

#define ASSERT assert
#define IN
#define OUT
#define CONST  const

#define interface struct
#define PURE = 0
//#define TRUE    (1)
//#define FALSE   (0)
#define __try_c
#define __finally finally:
#define __leave goto finally;


typedef short				mtf_wchar;
typedef void				VOID;
typedef unsigned int			HRESULT;
typedef unsigned int			DWORD;
typedef unsigned short			WORD;
typedef unsigned short			USHORT;
typedef int				BOOL;
typedef unsigned char			byte;
typedef byte				BYTE;
typedef BYTE*				PBYTE;
typedef unsigned char*			LPBYTE;
typedef void*				LPVOID;
typedef void*				PVOID;
typedef unsigned int			UINT;
typedef int 				LONG;    //long
typedef DWORD*				LPDWORD;
typedef int 				LRESULT;   //long
typedef void*				HANDLE;
typedef signed long long		LONGLONG;
typedef unsigned long long		ULONGLONG;
typedef unsigned char			UCHAR;
typedef char				CHAR;
typedef u64				DIFS_TIME;
typedef char				CCHAR;
typedef CHAR*				PCHAR;
typedef unsigned long			UINT_PTR;
typedef ULONGLONG			DWORDLONG;
typedef DWORDLONG*			PDWORDLONG;
typedef unsigned int		ULONG;    //long
typedef unsigned int*      ULONG_PTR ;      //long
typedef ULONG         SIZE_T;             
typedef void*				HINSTANCE;
typedef UINT				WPARAM;
typedef LONG				LPARAM;
typedef BYTE				BOOLEAN;
typedef LONG				NTSTATUS;
typedef unsigned long long		DWORD64;
typedef mtf_wchar			WCHAR;
typedef WCHAR*				LPWSTR;
typedef const WCHAR*			LPCWSTR;
typedef PCHAR				LPSTR;
typedef	VOID*				PVOID;
typedef BOOLEAN*			PBOOLEAN;
typedef CHAR*				PCHAR;
typedef	UCHAR*				PUCHAR;
typedef BYTE*				PBYTE;

typedef	WCHAR*				PWCHAR;
typedef USHORT*				PUSHORT;
typedef BOOL*				PBOOL;
typedef DWORD*				PDWORD;
typedef	ULONG*				PULONG;
typedef LONG*				PLONG;
typedef	LONGLONG*			PLONGLONG;
typedef	ULONGLONG*			PULONGLONG;

typedef void*				PFILE_OBJECT    ;  
typedef void*				PBCB;
typedef	void*				HANDLE ;
typedef	PVOID				PKSPIN_LOCK;
typedef long long			__int64;
typedef unsigned long long	__uint64;
typedef int					__int32;
typedef unsigned int		__uint32;
typedef unsigned short 		__int16;
typedef	s16					CSHORT;

typedef struct _GUID
{
	unsigned int   Data1;    //long
	unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
	
	bool operator == (const _GUID &guid)
	{
		return (Data1==guid.Data1 && Data2==guid.Data2 && Data3==guid.Data3 && memcmp(Data4, guid.Data4, sizeof(unsigned char)*8)==0);
	}
	
	bool operator != (const _GUID &guid)
	{
		return (Data1!=guid.Data1 || Data2!=guid.Data2 || Data3!=guid.Data3 || memcmp(Data4, guid.Data4, sizeof(unsigned char)*8)!=0);
	}

} GUID;

typedef union _LARGE_INTEGER 
{
    struct 
	{
        DWORD LowPart;
        LONG HighPart;
    } u;
    
	LONGLONG QuadPart;

}LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER 
{
    struct 
	{
        DWORD LowPart;
        DWORD HighPart;
    } u;
	
	ULONGLONG QuadPart;
}ULARGE_INTEGER;

typedef struct tagRECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
}RECT;

#if defined(_UNICODE) || defined(UNICODE)

	typedef LPWSTR				LPTSTR;
	//#define TRACE				wprintf
	#define _tcscpy				wcscpy
	#define _itot				_itow
	#define _tsprintf			swprintf
	#ifndef _TCHAR_DEFINED
	#define __T(x)				L ## x
	typedef wchar_t				TCHAR;
	#define _TCHAR_DEFINED
	#endif	//_TCHAR_DEFINED
	typedef const wchar_t*			LPCTSTR;
	typedef wchar_t*			LPTSTR;
	typedef wchar_t				_TUCHAR;

#else

	typedef LPSTR				LPTSTR;
	//#define TRACE				printf
	#define _tcscpy				strcpy
	#define _itot				_itoa
	#define _tsprintf			sprintf
	#ifndef _TCHAR_DEFINED
	#define __T(x)				x
	typedef char				TCHAR;
	#define _TCHAR_DEFINED
	#endif	//_TCHAR_DEFINED
	typedef const char*			LPCTSTR;
	typedef char*				LPTSTR;
	typedef char				_TUCHAR;

#endif	//_UNICODE|UNICODE	

	#define String std::string

	#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
		 const GUID name \
						= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

	#ifndef ASSERT
	#define ASSERT(x) 			assert(x)
	#endif

	#define getmin(a,b)			(((a) < (b)) ? (a) : (b))
	#define getmax(a,b)			(((a) > (b)) ? (a) : (b))
	#define _T(x)       		__T(x)
	#define E_ABORT        		((HRESULT)0x80000007L)
	#define E_FAIL            	((HRESULT)0x80000008L)
	#define S_OK         		((HRESULT)0x00000000L)
	#define S_FALSE         	((HRESULT)0x00000001L)
	#define INFINITE			0xFFFFFFFF 

	#ifndef FALSE
	#define FALSE				(0)
	#endif

	#ifndef TRUE
		#define TRUE				(1)
	#endif

	#ifndef SID_IDENTIFIER_AUTHORITY_DEFINED
		#define SID_IDENTIFIER_AUTHORITY_DEFINED
		typedef struct _SID_IDENTIFIER_AUTHORITY {
			BYTE  Value[6];
		} SID_IDENTIFIER_AUTHORITY, *PSID_IDENTIFIER_AUTHORITY;
	#endif

	#define ANYSIZE_ARRAY 1  
	#ifndef SID_DEFINED
		#define SID_DEFINED
		typedef struct _SID {
		   BYTE  Revision;
		   BYTE  SubAuthorityCount;
		   SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
			#ifdef MIDL_PASS
			   [size_is(SubAuthorityCount)] DWORD SubAuthority[*];
			#else // MIDL_PASS
			   DWORD SubAuthority[ANYSIZE_ARRAY];
			#endif // MIDL_PASS
		} SID, *PISID;
	#endif

	typedef struct _LIST_ENTRY {
	   struct _LIST_ENTRY *Flink;
	   struct _LIST_ENTRY *Blink;
	} LIST_ENTRY, *PLIST_ENTRY, *PRLIST_ENTRY;

	typedef	TCHAR*				PTCHAR;
	typedef DIFS_TIME*			PDIFS_TIME;

	#define OSTHREAD_H pthread_t
	#define	OSTHREAD_ID OSTHREAD_H
	#define OSTHD_FUNC_RET void*
	#define SLASH _T("/")
	#define SLASHCH _T('/')
	#define SLASHLIN    _T("/")
	#define SLASHLINCH  _T('/')
	#define DBSLASH _T("//")

	#define CLOSESOCKET(x) close(x)

#endif	//(FI_LINUX) || defined(FI_MAC)
	

#else
	#include <winsock2.h>
	#include <windows.h>
	#include <process.h>

	#define SLASH _T("\\")
	#define SLASHCH _T('\\')
	#define DBSLASH _T("\\\\")
	#define SLASHWIN    _T("\\")
	#define SLASHWINCH  _T('\\')
	#define SLASHLIN    _T("/")
	#define SLASHLINCH  _T('/')

	#define OSTHREAD_H HANDLE
	#define	OSTHREAD_ID DWORD
	#define OSTHD_FUNC_RET DWORD
	#define CLOSESOCKET(x) closesocket(x)

#endif //_win32

#ifdef __cplusplus
}		//C plus plus
#endif

#endif

