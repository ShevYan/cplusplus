/*++

Copyright (c) 2006  sobey Corporation

Module Name:

    kernel_user.h

Abstract:

    In charge of convert struct types in kernel mode and user mode

Author:

    difs_sobey   2006-8-29  

Revision History:

--*/

#ifndef _KERNEL_USER_H_
#define _KERNEL_USER_H_

/* DEFINITIONS ***********************************************************/
	#if defined (FI_LINUX)
		#include <asm/types.h>
	#elif defined (FI_MAC)
		#include <sys/types.h>
	#endif


	#ifndef _AMD64_              //32 bit driver and application
		#define     PVOID_DEF_UK(pVar)  union{PVOID              pVar; ULONGLONG pVar##_placeholder;}
		#define     PU64_DEF(pVar)      union{unsigned long long * pVar; ULONGLONG pVar##_placeholder;}
		#define     PCHAR_DEF(pVar)     union{char             * pVar; ULONGLONG pVar##_placeholder;}
		#define     PWCHAR_DEF(pVar)    union{short          * pVar; ULONGLONG pVar##_placeholder;}
		#define     PUCHAR_DEF(pVar)    union{unsigned char    * pVar; ULONGLONG pVar##_placeholder;}
		#define     HANDLE_DEF_UK(Var)  union{void*              Var; ULONGLONG  Var##_placeholder;}
        #define     MASEVENTHANDLE_DEF(Var)  union{MASEVENTHANDLE     Var; ULONGLONG  Var##_placeholder;}
        #define     DSIKPOOL_DEF(pVar)  union{CDiskPool*     pVar; ULONGLONG  pVar##_placeholder;}
	#else                           //64 bit driver and application
		#define     PVOID_DEF_UK(pVar)       PVOID              pVar
		#define     PU64_DEF(pVar)           unsigned long long * pVar
		#define     PCHAR_DEF(pVar)          char             * pVar
		#define     PWCHAR_DEF(pVar)         short          * pVar
		#define     PUCHAR_DEF(pVar)         unsigned char    * pVar
		#define     HANDLE_DEF_UK(Var)       void*             Var
        #define     MASEVENTHANDLE_DEF(Var)        MASEVENTHANDLE     Var
        #define     DSIKPOOL_DEF(pVar)        CDiskPool*     pVar
		
	#endif
#endif


