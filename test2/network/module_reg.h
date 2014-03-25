#ifndef __MODULE_REG_H__
#define __MODULE_REG_H__


#include "socketpool.h"

class CModuleInfo
{
public:

    CModuleInfo(DWORD module, SOCKETCREATE_FUNC _create, SOCKETMSG_FUNC _msgrecv, SOCKETEXIT_FUNC _exit);
	CModuleInfo(DWORD module, INetReceiver* pReceiver);
    ~CModuleInfo();

public:

    DWORD modID;
    SOCKETCREATE_FUNC modCreate;
    SOCKETMSG_FUNC modRecv;
    SOCKETEXIT_FUNC modExit;
	INetReceiver* m_pReceiver;

public:
    u64 GetModPktSeq();
    CMemItem* GetModMsgReply(const u64 &pktId);
    ReqCxt* GetReqParam(const u64& pkt);

    void AddReqCxt(const ReqCxt &cxt);
    BOOL AddOnceCall(const u64 &pkt);

    BOOL EraseOnceCall(const u64 &pkt);
    void EraseReqCxt(const u64 &pkt);

    BOOL Notify(const u64 &pkt, CMemItem *mem);
    /// when module use tunnel buffer recv msg
    void SetTunnelBufferStatus(const u64& pkt,u8 used);
    BOOL ThrowReplyPkt(const u64 &pkt);

private:

    FiCritSec modReqMsgLock;
    map<u64,ReqCxt> modReqMsg;

    /// record all once call svr only deal with once
    map<u64,u64> modOnceCall;
    FiCritSec modOnceCallLock;

	volatile  s64       modPktSeq;
    
};

#endif
