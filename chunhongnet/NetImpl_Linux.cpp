#include "NetPrivate.h"

namespace NetWork
{
  TcpServer::TcpServer(void)
    :m_Listenfd(INVALID_SOCKET)
    ,m_Epollfd(INVALID_SOCKET)
  {
    memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
    memset(m_SendBuf, 0, sizeof(m_SendBuf));
  }

  TcpServer::~TcpServer()
  {
    CloseNetwork();
    CloseSocket(m_Listenfd);
    m_Listenfd = INVALID_SOCKET;

    // Close Session
    ClearTcpSession();
  };

  void TcpServer::CloseSocket(SOCKET Sock)
  {
    ::close(Sock);
  }

  void TcpServer::NoBlockSocket(SOCKET Sock)
  {
    S32 opts = fcntl(Sock, F_GETFD);
    opts = opts | O_NONBLOCK | O_ASYNC;
    fcntl(Sock, F_SETFD, opts);
  }


  BOOL TcpServer::TcpStartup()
  {
    m_Epollfd = epoll_create(EPOLL_SIZE);
    if(SOCKET_ERROR == m_Epollfd)
    {
      LogOutErrorString(_("%s %d %s : %s "), __FILE__, __LINE__, __FUNCTION__, strerror(errno));
      return FALSE;
    }
    return TRUE;
  }

  void TcpServer::CloseNetwork()
  {
    CloseSocket(m_Epollfd);
    m_Epollfd = INVALID_SOCKET;
  }

  BOOL TcpServer::InitNet()
  {
    if(!TcpStartup())
    {
      LogOutErrorString(_("%s %d %s"), __FILE__, __LINE__, __FUNCTION__);
      return FALSE;
    }

    if(!OpenSocket(m_Listenfd, m_ListenPort))
    {
      LogOutErrorString(_("%s %d %s : %s "), __FILE__, __LINE__, __FUNCTION__, strerror(errno));
      return FALSE;
    }

    if(AddEpollIO(m_Listenfd, EPOLLIN | EPOLLOUT) < 0)
    {
      LogOutErrorString(_("%s %d %s : %s "), __FILE__, __LINE__, __FUNCTION__, strerror(errno));
      return FALSE;
    }

    return TRUE;
  }



  int TcpServer::SetEpollIO(int fd, unsigned flag)
  {
    if ( ModEpollIO(fd, flag) < 0 )    
    {
      if ( AddEpollIO(fd, flag) < 0 )
      {
        return -1;
      }
    }

    return 0;
  }

  int TcpServer::AddEpollIO(int fd, unsigned flag)
  {
    epoll_event ev;
    memset((void*)&ev,0,sizeof(epoll_event));
    ev.data.fd = fd;
    ev.events = flag;

    if(epoll_ctl(m_Epollfd, EPOLL_CTL_ADD , fd, &ev) < 0)
    {
      LogOutErrorString(_("%s, %s, %d Error: Add Epoll Failed [Code: %d --> %s]"), __FILE__, __FUNCTION__, __LINE__, errno, strerror(errno));
      return -1;
    }

    //++m_attachedSocketCount;
    return 0;
  }

  int TcpServer::ModEpollIO(int fd, unsigned flag)
  {
    epoll_event ev;
    memset((void*)&ev,0,sizeof(epoll_event));
    ev.data.fd = fd;
    ev.events = flag;

    if (epoll_ctl(m_Epollfd, EPOLL_CTL_MOD , fd, &ev) < 0)    
    {
      return -1;
    }

    return 0;
  }

  int TcpServer::DelEpollIO(int fd)
  {
    epoll_event ev;
    memset((void*)&ev,0,sizeof(epoll_event));
    ev.data.fd = fd;
    ev.events = 0;
    if (epoll_ctl(m_Epollfd, EPOLL_CTL_DEL, fd, &ev) < 0)
    {
      return -1;
    }

    //--m_attachedSocketCount;
    return 0;
  }

  void TcpServer::CloseATcpSession(SOCKET Sock)
  {
    if(DelATcpSession(Sock)) // Del A Session form actice list
    {
      DelEpollIO(Sock); // delete A epoll IO

      CloseSocket(Sock); // close socket
    }
  }


  // 返回读取长度.Error： -1
  S32 TcpServer::ReadSocket(SOCKET Sock, char* pBuf, int iSize)
  {
    S32 iLen = ::read(Sock, pBuf, iSize);
    if(iLen == 0)
      return -1;

    if(iLen < 0)
    {
      S32 ErrorNo = errno;
      if(EAGAIN != ErrorNo)
      {
        LogOutErrorString(_("%s Will Close[Socket: %d] %s[errno: %d]"), __FUNCTION__, Sock, strerror(ErrorNo), ErrorNo);
        return -1;
      }
      return 0;
    }

    return iLen;
  }

  // 返回写入长度。Error： -1
  S32 TcpServer::WriteSocket(SOCKET Sock, char* pBuf, int iLen)
  {
    S32 iSendLen = 0;
    while(iLen > 0)
    {
      S32 ret = ::write(Sock, pBuf, iLen);
      if(ret == 0)
        return -1;

      if(ret < 0)
      {
        S32 ErrorNo = errno;
        if(EAGAIN != ErrorNo)
        {
          LogOutErrorString(_("%s Will Close[Socket: %d] %s[errno: %d]"), __FUNCTION__, Sock, strerror(ErrorNo), ErrorNo);
          return -1;
        }

        return 0;
      }

      iLen -= ret;
      pBuf += ret;
      iSendLen += ret;
    }

    return iSendLen;
  }

  bool TcpServer::NetPoll()
  {
    DetachTcpSession();

    return IOEPoll();
  }

  bool TcpServer::IOEPoll(void)
  {
    SOCKET acceptSocket = INVALID_SOCKET;

    sockaddr_in addr;
    S32 len = sizeof(addr);

    S32 nfds = epoll_wait(m_Epollfd, Events, ARRAY_COUNT(Events), 10);
    for(S32 i = 0; i < nfds; ++i)
    {
      if(Events[i].data.fd == m_Listenfd)
      {
        acceptSocket = ::accept(m_Listenfd, (struct sockaddr*)&addr, (socklen_t*)&len);
        if(INVALID_SOCKET == acceptSocket)
        {
          S32 ErrorNo = errno;
          if(EAGAIN != ErrorNo)
          {
            if(AddEpollIO(m_Listenfd, EPOLLIN | EPOLLOUT) < 0)
            {
              continue;
            }
          }

          LogOutErrorString(_("%s, %s, %d,Error: ::accept [Code: %d --> %s]"),__FILE__, __FUNCTION__, __LINE__, ErrorNo, strerror(ErrorNo));
          continue;
        }

        const char* pIP = inet_ntoa(addr.sin_addr);
        U32 dwPort = ntohs(addr.sin_port);

        LogOutDebugString(_("Accept Socket: %d IP: %s Port: %u"), acceptSocket, pIP, dwPort);

        TcpSession* pSession = HandlerAttachATcpSession(pIP, dwPort);
        if(NULL != pSession)
        {
          if(AddEpollIO(acceptSocket, EPOLLIN | EPOLLOUT) < 0)
          {
            CloseSocket(acceptSocket);
          }

          NoBlockSocket(acceptSocket);

          pSession->InitUse(acceptSocket, addr);
          AddATcpSession(acceptSocket, pSession);
        }
        else
        {
          LogOutErrorString(_("%s Can't Create A Tcp Session! close socket: %d"), __FUNCTION__, acceptSocket);

          DelEpollIO(acceptSocket);
          CloseSocket(acceptSocket);
        }
      }
      else
      {
        TcpSession* pSession = FindATcpSession(Events[i].data.fd); // Find A Tcp-Session By SOCKET
        if(NULL == pSession)
        {
          DelEpollIO(Events[i].data.fd);
          CloseSocket(Events[i].data.fd);
          continue;
        }

        if(Events[i].events & EPOLLIN)
        {
          S32 iEmptySize = Min<S32>(sizeof(m_RecvBuf), pSession->GetRecvBuf()->available());

          S32 iLen = TcpServer::ReadSocket(Events[i].data.fd, m_RecvBuf, iEmptySize);
          if(-1 == iLen)
          {
            CloseATcpSession(Events[i].data.fd);
            continue;
          }

          if(iLen > 0)
          {
            assert(iLen < pSession->GetRecvBuf()->available());

            pSession->GetRecvBuf()->__kfifo_put(m_RecvBuf, iLen);
          }
        }

        if(Events[i].events & EPOLLOUT)
        {
          S32 iSendLen = pSession->GetSendBuf()->used();
          if(iSendLen > 0)
          {
            S32 iWriteSize = Min<S32>(sizeof(m_SendBuf), iSendLen);
            pSession->GetSendBuf()->__peek(m_SendBuf, iWriteSize);

            S32 iLen = TcpServer::WriteSocket(Events[i].data.fd, m_SendBuf, iWriteSize);
            if(iLen < 0)
            { 
              CloseATcpSession(Events[i].data.fd);
              continue;
            }

            // Move Read
            pSession->GetSendBuf()->__seekread(iLen);

            // Drop Net Data
            if(iLen < iWriteSize)
            {
              LogOutErrorString( _("%s %d %s Socket Buffer Full. Data Size: %d Send Size: %d"), __FILE__, __LINE__, __FUNCTION__, iWriteSize, iLen);
            }
          }
        }
      }
    }

    return true;
  }

  void TcpServer::DetachTcpSession()
  {
    LockGuard<CPLCriticalSection> Guard(m_mutexRemoveSession);
    if(m_MapRemoveSession.size() <= 0)
      return;

    for(STHashMap<SOCKET, TcpSession*>::iterator it(m_MapRemoveSession); it; ++it)
    {
      CloseATcpSession(it.Key());
    }
    m_MapRemoveSession.clear();
  }

  void TcpServer::Stop()
  {
    CloseSocket(m_Listenfd);
    for(STHashMap<SOCKET,TcpSession*>::iterator it(m_MapTcpSession); it; ++it)
    {
      CloseSocket(it.Key());
    }

    LockGuard<CPLCriticalSection> Guard(m_mutexRemoveSession);
    m_MapRemoveSession.clear();

    CloseNetwork();
  }

} // namespace NetWork