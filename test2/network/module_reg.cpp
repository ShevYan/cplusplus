#include "module_reg.h"

CModuleInfo::CModuleInfo(DWORD module, SOCKETCREATE_FUNC _create, SOCKETMSG_FUNC _msgrecv, SOCKETEXIT_FUNC _exit)
:modID(module),modCreate(_create),modRecv(_msgrecv),modExit(_exit),modPktSeq(0),m_pReceiver(NULL)
{
    ;
}

CModuleInfo::CModuleInfo( DWORD module, INetReceiver* pReceiver )
:modID(module),modCreate(NULL),modRecv(NULL),modExit(NULL),modPktSeq(0),m_pReceiver(pReceiver)
{

}


CModuleInfo::~CModuleInfo()
{

}

u64 CModuleInfo::GetModPktSeq()
{
#if defined(_WIN32) || defined(_WIN64)
	return InterlockedIncrement64(&modPktSeq);
#else
    return __sync_add_and_fetch(&modPktSeq,1);
#endif
}

CMemItem* CModuleInfo::GetModMsgReply(const u64 &pktId)
{
    CMemItem* p = NULL;
    map<u64,ReqCxt>::iterator it;

    modReqMsgLock.Lock();

    if(modReqMsg.end() !=(it = modReqMsg.find(pktId)))
    {
	p = it->second.replymsg;

	//// get msg then erase it in lock for syni muliple msg
	 
	modReqMsg.erase(it);
    }

    modReqMsgLock.Unlock();

    return p;
}

void CModuleInfo::AddReqCxt(const ReqCxt &cxt)
{
    ASSERT(cxt.modid);

    modReqMsgLock.Lock();
    modReqMsg[cxt.pktid] = cxt;
    modReqMsgLock.Unlock();
}

void CModuleInfo::EraseReqCxt(const u64 &pkt)
{
    modReqMsgLock.Lock();
    modReqMsg.erase(pkt);
    modReqMsgLock.Unlock();
}

void CModuleInfo::SetTunnelBufferStatus(const u64& pkt, u8 used)
{
    map<u64,ReqCxt>::iterator it;

    modReqMsgLock.Lock();
    if((it = modReqMsg.find(pkt)) != modReqMsg.end())
    {
	it->second.used = used;
    }
    modReqMsgLock.Unlock();
}

ReqCxt* CModuleInfo::GetReqParam(const u64& pkt)
{
    ReqCxt* cxt = NULL;

    map<u64,ReqCxt>::iterator it;
    modReqMsgLock.Lock();

    if((it = modReqMsg.find(pkt)) != modReqMsg.end())
    {
	cxt = &it->second;
    }

    modReqMsgLock.Unlock();

    return cxt ;

}

BOOL CModuleInfo::Notify(const u64 &pkt, CMemItem *mem)
{
    BOOL ret = FALSE;

    map<u64,ReqCxt>::iterator it;
    modReqMsgLock.Lock();

    if((it = modReqMsg.find(pkt)) != modReqMsg.end())
    {
	if (NULL != it->second.replymsg)
	{
	    _SafeDeletePtr(it->second.replymsg);
	}

	it->second.replymsg = mem;

	FiEvent* pEv = (FiEvent*) it->second.ev;
	pEv->Set();

	ret = TRUE;
    }

    modReqMsgLock.Unlock();

    return ret ;
}

BOOL CModuleInfo::ThrowReplyPkt(const u64 &pkt)
{
    BOOL bThrow = FALSE;
    map<u64,ReqCxt>::iterator it;

    modReqMsgLock.Lock();
    if((it = modReqMsg.find(pkt)) != modReqMsg.end())
    {
	if (it->second.used)
	{
	    bThrow = TRUE;         /// other thread use tunnel buffer ,then thow msg
	}
	else
	{
	    it->second.used = TRUE;/// set tunnel buffer is used other thread can not use;
	}
    }
    else
    {
	bThrow = TRUE;
    }

    modReqMsgLock.Unlock();

	return bThrow;
}


BOOL CModuleInfo::AddOnceCall(const u64 &pkt)
{
    BOOL ret = FALSE;

    modOnceCallLock.Lock();

    if (modOnceCall.end() != modOnceCall.find(pkt))
    {
		ret = FALSE;
    }else
    {
		modOnceCall.insert(make_pair(pkt, pkt));
		ret = TRUE;
    }

    modOnceCallLock.Unlock();

	return ret;
}

BOOL CModuleInfo::EraseOnceCall(const u64 &pkt)
{
    modOnceCallLock.Lock();
    modOnceCall.erase(pkt);
    modOnceCallLock.Unlock();

    return TRUE;
}
