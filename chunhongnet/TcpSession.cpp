#include "NetPrivate.h"

namespace NetWork
{
  TcpSession::TcpSession(int irecvbufsize, int isendbufsize)
    :m_iSoket(INVALID_SOCKET)
    ,m_Status(ESESSION_STATUS_NONE)
    ,pTcpServer(NULL)
  {
    pRecvBuf = new circular_buffer(irecvbufsize);
    pSendBuf = new circular_buffer(isendbufsize);
  }

  TcpSession::~TcpSession()
  {
    delete pRecvBuf;
    delete pSendBuf;
  }

  void TcpSession::SendAllKey()
  {
    LogOutErrorString(_("%s %d %s Not Instance"), __FILE__, __LINE__, __FUNCTION__);
  }

  bool TcpSession::ReadAllKey()
  {
    LogOutErrorString(_("%s %d %s Not Instance"), __FILE__, __LINE__, __FUNCTION__);
    return false;
  }

  void TcpSession::ClearBuf()
  {
    pRecvBuf->clear();
    pSendBuf->clear();
  }

  void TcpSession::InitUse(NetBase* pNet, SOCKET iSock, const sockaddr_in& Addr)
  {
    ChgSessionState(ESESSION_STATUS_START);

    Address.SockAddr(Addr);

    SetSocket(iSock);

    pTcpServer = pNet;

    ClearBuf();
  }

  void TcpSession::FinishSession()
  {
    LogOutDebugString(_("Logout A Tcp Session: %d"), GetSocket());

    if(pTcpServer)
    {
      pTcpServer->Finish(GetSocket(), this);
      pTcpServer = NULL;
    }
    else
    {
      LogOutErrorString(_("Repeat Release A Socket %d !"), GetSocket());
    }
  }

  void TcpSession::ChgSessionState(ESessionStatus estate) 
  {
    LockGuard<CPLCriticalSection> Guard(mutexStatus);
    m_Status = estate; 
  }

  TcpSession::ESessionStatus TcpSession::GetSessionState(void) 
  {
    LockGuard<CPLCriticalSection> Guard(mutexStatus);
    return m_Status; 
  }

  BOOL TcpSession::IsClosed()
  {
    LockGuard<CPLCriticalSection> Guard(mutexStatus);
    return m_Status == ESESSION_STATUS_CLOSE;
  }

}