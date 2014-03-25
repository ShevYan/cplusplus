#define _TCPIODLL

#include <fcntl.h>
#include "tcpio.h"
#include "tcpserver.h"

#ifndef FI_LINUX
#include <process.h>
#endif


CTcpIo::CTcpIo()
{
	m_pImpl = new CTcpServer;
}

CTcpIo::~CTcpIo()
{

}

BOOL CTcpIo::Start(USHORT uport)
{
	ASSERT(uport);
	m_pImpl->SetPort(uport);
	m_pImpl->SetAutoConnFlag(TRUE);

	return m_pImpl->Init((SOCKETCREATE_FUNC)NULL, NULL, (SOCKETEXIT_FUNC)NULL, NULL,m_pImpl->_recvcb , m_pImpl, TCP_SVR_CLT);
}

BOOL CTcpIo::StartServer(USHORT uport)
{
    ASSERT(uport);

    m_pImpl->SetPort(uport);
    return m_pImpl->Init((SOCKETCREATE_FUNC)NULL, NULL, (SOCKETEXIT_FUNC)NULL, NULL,m_pImpl->_recvcb , m_pImpl, TCP_SVR);
}

BOOL CTcpIo::StartClient(USHORT svrport)
{
    m_pImpl->SetPort(svrport);
    m_pImpl->SetAutoConnFlag(TRUE);

    return m_pImpl->Init((SOCKETCREATE_FUNC)NULL, NULL, (SOCKETEXIT_FUNC)NULL, NULL, m_pImpl->_recvcb, m_pImpl, TCP_CLT);
}

BOOL CTcpIo::RegisterModule(u32 modID, SOCKETCREATE_FUNC _create, SOCKETMSG_FUNC _msgrecv, SOCKETEXIT_FUNC _exit)
{
	return m_pImpl->RegisterModule(modID, _create, _msgrecv, _exit);
}

BOOL CTcpIo::RegisterModule(u32 modID, INetReceiver* pReceiver)
{
	return m_pImpl->RegisterModule(modID, pReceiver);
}

BOOL CTcpIo::UnRegisterModule(u32 modID)
{
	return m_pImpl->UnRegisterModule(modID);
}

/// call masgess and wait retu
int CTcpIo::Call(call_cxt& cxt)
{
	return m_pImpl->Call(cxt);
}

int CTcpIo::Reply(CMemItem* sheet, char* buf1,int buf1len, char* buf2, int buf2len)
{
	return m_pImpl->Reply(sheet, buf1, buf1len, buf2, buf2len);
}

int CTcpIo::Route(call_cxt& cxt, CMemItem* sheet)
{
	return m_pImpl->Route(cxt, sheet);
}

int CTcpIo::LocalCall(call_cxt& cxt)
{
    return m_pImpl->LocalCall(cxt);
}

int CTcpIo::RouteCall(call_cxt& cxt, CMemItem* sheet)
{
    return m_pImpl->RouteCall(cxt, sheet);
}
