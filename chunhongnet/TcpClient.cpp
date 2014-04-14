#include "NetPrivate.h"

using namespace fastdelegate;


namespace NetWork
{
  TcpClient::TcpClient()
    : TcpState(ETCP_STATE_NONE)
    , pSession(NULL)
  {

  }

  TcpClient::~TcpClient()
  {
    if(NULL != pFsmBrain)
      delete pFsmBrain;
  }

  void TcpClient::SetIPAddres(const char* szIP, U16 iPort)
  { 
    if(!NetBase::CheckIPAddressIPv4(szIP))
    {
      LogOutErrorString(_("Check IP Address[%s] Failed!"), szIP);
      return;
    }

    std::string IP(szIP);
    Address.Set(IP, iPort);
  }

  void TcpClient::Stop()
  {
    ChangeState(ETCP_STATE_CLOSE);
  }

  bool TcpClient::InitNet(void)
  {
    Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(INVALID_SOCKET == Socket)
    {
      LOG_TCPCLIENT_ERROR();
      return false;
    }

    S32 iFlag = 1;
    if(SOCKET_ERROR == ::setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&iFlag, sizeof(char)*4))
      return false;

    // 禁用nagle算法，使得小数据量包尽快发送
    if(SOCKET_ERROR == ::setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, (char*)&iFlag, sizeof(char)*4))
      return false;

    // new FSM
    pFsmBrain = new FSM<TcpClient>(this);

    // Goto Connect Server!
    ChangeState(ETCP_STATE_CONNECT);

    return true;
  }

  void TcpClient::NoBlockSocket(SOCKET Sock)
  {
#ifndef _WIN32
    S32 opts = fcntl(Sock, F_GETFD);
    opts = opts | O_NONBLOCK | O_ASYNC;
    fcntl(Sock, F_SETFD, opts);
#else
    u_long iMode = 1;  // non-blocking mode is enabled.
    ioctlsocket(Sock, FIONBIO, &iMode); // 设置为非阻塞模式
#endif
  }

  //
  // 摘要：
  //     阻塞式重连
  //
  bool TcpClient::Connect(const char* pIP, int iPort)
  {
    if(!NetBase::CheckIPAddressIPv4(pIP))
      return false;

    sockaddr_in sin;

    memset(&sin, 0, sizeof(sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(pIP);
    sin.sin_port = htons(iPort);

    LogOutDebugString(_("Begin Connect Server: %s : %d"), pIP, iPort);

    S32 Ret = ::connect(Socket, (sockaddr*)&sin, sizeof(sockaddr));
    if(Ret != INVALID_SOCKET)
    {
      NoBlockSocket(Socket);

      LogOutDebugString(_("Connect Server[%s : %d] Success!"), pIP, iPort);
      return true;
    }
    else
    {
      LOG_TCPCLIENT_ERROR();
    }

    return false;
  }


  bool TcpClient::Connect()
  {
    const sockaddr_in& sin = Address.SockAddr();

    LogOutDebugString(_("Begin Connect Server: %s : %d"), Address.IP().c_str(), Address.Port());

    S32 Ret = ::connect(Socket, (sockaddr*)&sin, sizeof(sockaddr));
    if(Ret != INVALID_SOCKET)
    {
      NoBlockSocket(Socket);

      LogOutDebugString(_("Connect Server[%s : %d] Success!"), Address.IP().c_str(), Address.Port());
      return true;
    }
    else
    {
      LOG_TCPCLIENT_ERROR();
    }

    return false;
  }

  void TcpClient::CloseSocket()
  {
#ifndef _WIN32
    ::close(Socket);
#else
    closesocket(Socket);
#endif
  }

  void TcpClient::CloseATcpSession(TcpSession* pSession)
  {
    pSession->ChgSessionState(TcpSession::ESESSION_STATUS_CLOSE);
    CloseSocket();
  } 

  bool TcpClient::Disconnect()
  {
    CloseSocket();

    pSession->GetRecvBuf()->clear();
    pSession->GetSendBuf()->clear();

    return true;
  }

  bool TcpClient::Reconnect(const char* pIP, int iPort)
  {
    Disconnect();

    return Connect(pIP, iPort);
  }

  bool TcpClient::Reconnect(void)
  {
    Disconnect();

    return Connect();
  }

  void TcpClient::NetPoll()
  {
    pFsmBrain->Update();
  }

  void TcpClient::ChangeState(ETcpClientState eState)
  {
    SetState(eState);

    switch(eState)
    {
    case ETCP_STATE_CONNECT:
      pFsmBrain->ChangeState(TcpClientConnect::GetInstance());
      break;

    case ETCP_STATE_START:
      pFsmBrain->ChangeState(TcpClientRun::GetInstance());
      break;

    case ETCP_STATE_DISCONNECT:
      pFsmBrain->ChangeState(TcpClientDisconnect::GetInstance());
      break;

    case ETCP_STATE_CLOSE:
      pFsmBrain->ChangeState(TcpClientClose::GetInstance());
      break;

    default:
      break;
    }
  }

  bool TcpClient::NetEvent(void)
  {
    fd_set wfds;
    fd_set rfds;
    const struct timeval tm = { 0, 10 };

    FD_ZERO(&wfds);
    FD_ZERO(&rfds);
    FD_SET(Socket, &wfds);
    FD_SET(Socket, &rfds);

    S32 ret = ::select(Socket + 1, &rfds, &wfds, NULL, &tm);
    if(ret == SOCKET_ERROR)
    {
      LOG_TCPCLIENT_ERROR();
      return false;
    }

    if(FD_ISSET(Socket, &rfds))
    {
      //调用recv，接收数据。
      S32 iEmptySize = Min<S32>(sizeof(RecvBuf), pSession->GetRecvBuf()->available());
      S32 iLen = TcpServer::ReadSocket(Socket, RecvBuf, iEmptySize);
      if(-1 == iLen)
      {
        CloseATcpSession(pSession);

        return false;
      }

      if(iLen > 0)
      {
        assert(iLen <= pSession->GetRecvBuf()->available());

        pSession->GetRecvBuf()->__kfifo_put(RecvBuf, iLen);
      }
    }

    if(FD_ISSET(Socket, &wfds))
    {
      //调用send，发送数据。
      S32 iSendLen = pSession->GetSendBuf()->used();
      if(iSendLen > 0)
      {
        S32 iWriteSize = Min<S32>(sizeof(SendBuf), iSendLen);
        pSession->GetSendBuf()->__peek(SendBuf, iWriteSize);

        S32 iLen = TcpServer::WriteSocket(Socket, SendBuf, iWriteSize);
        if(iLen == -1)
        {
          CloseATcpSession(pSession);

          return false;
        }

        // Move Read
        pSession->GetSendBuf()->__seekread(iLen);

        // Drop Net Data
        if(iLen < iWriteSize)
        {
          LogOutErrorString( _("%s %d %s Socket Buffer Full. Data Size: %d Send Size: %d"),  __FILE__, __LINE__, __FUNCTION__, iWriteSize, iLen);
        }
      }
    }

    return true;
  }

  void TcpClient::SetTcpSession(TcpSession* psession)
  {
    assert(NULL != psession); 
    pSession = psession;
  }

  void TcpClient::RegisterEvent(ETcpClientState eState, FastDelegate0<> Handler)
  {
    TcpClientEventHandler[eState] = Handler;
  }

  void TcpClient::UnregisterEvent(ETcpClientState eState, FastDelegate0<> Handler)
  {
    TcpClientEventHandler[eState].clear();
  }
}
