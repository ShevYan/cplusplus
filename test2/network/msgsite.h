#ifndef __MSGSITE_H__
#define __MSGSITE_H__
 
#include "socketpool.h"
#include "netmsg.h"
#include "module_reg.h"
#include <map>

using namespace std;

#define MSG_HEAD 	0
#define MSG_USER_BODY	1
#define MSG_TUNNEL_BODY 2
#define MSG_END		3

typedef struct _MSG_CXT
{
    /// pMsgHead =Tsheet header + user header + user extral buffer;
    CMemItem *pMsgHead;
    CMemItem *pUserData;
    CMemItem *pTunnelBuf;
    DWORD	    dwUserDataSize;
    DWORD           dwMsgSize;
    DWORD	    dwSheetSize;
    DWORD 	    dwMsgSect;
    BOOL 	    bTunnelUnused;
    FiCritSec       lk;
    Tsheet*         sh;
    void  * 	    mod;

    _MSG_CXT()
    {
	pMsgHead = NULL;
	pTunnelBuf = NULL;
	pUserData  = NULL;
	dwUserDataSize = 0;
	dwMsgSize = 0;
	dwSheetSize = 0;
	bTunnelUnused = FALSE;
	dwMsgSect = 0;
	sh = NULL;
	mod = NULL;
    }

    void Reset()
    {
	pMsgHead = new CMemItem();
	pUserData = new CMemItem();
	ASSERT(pMsgHead);

	pTunnelBuf->SetCurPos(0);
	pTunnelBuf->SetDataRealSize(0);

	dwUserDataSize = 0;
	dwSheetSize = 0;
	dwMsgSize = 0;
	bTunnelUnused = FALSE;
	dwMsgSect = 0;
	sh = NULL;
	mod =NULL;
    }
}MSG_CXT, *PMSG_CXT;

class CMsgSite
{
public:
    CMsgSite(SOCKETMSG_FUNC func)
    :m_callback(func){}

    virtual ~CMsgSite();
public:
    BOOL GetMsg(u32& identify,CShareLock& modlock,map<DWORD,CModuleInfo*>& modmap,
                CMemItem *pMsgBuf, CTCPSocket *psokt, void *userData, MSG_CXT* pMsgCxt);

    BOOL InsertSock(HANDLE hSock);
    void RemoveSock(HANDLE hSock);
    MSG_CXT* Lock_sock(CTCPSocket *psokt);
    void Unlock_sock(MSG_CXT* pMsgCxt);
public:
    CShareLock m_lk;
    SOCKETMSG_FUNC m_callback;
    map<HANDLE, MSG_CXT *> m_mapSockMsg;
}; 

#endif
