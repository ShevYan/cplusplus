//********************************************************************
//  Created: 2014/01/11
//  File Name: NetImpl_Linux.h
//  Author: Kratons.Ren
//  
//  purpose:
//
//********************************************************************

#ifndef _NETTHREAD_LIUNX_H_
#define _NETTHREAD_LIUNX_H_

namespace NetWork
{
  class TcpServer : public NetBase
  {
  public:
    TcpServer(void);

    virtual ~TcpServer(void);

  public:
    inline void   SetListenPort(S32 port) { m_ListenPort = port; }
    inline S32    GetListenPort(void) const { return m_ListenPort; }

    void          CloseSocket(SOCKET Sock);
    void          NoBlockSocket(SOCKET Sock);

    void          DetachTcpSession(void);

    BOOL          InitNet(void);
    BOOL          TcpStartup();
    void          CloseNetwork();
    
    void          CloseATcpSession(SOCKET Sock);

    void          Stop(void);

    bool          NetPoll(void); // Select model

  public:
    static int    ReadSocket(SOCKET Sock, char* pBuf, int iLen);
    static int    WriteSocket(SOCKET Sock, char* pBuf, int iLen);

  private:
    int           SetEpollIO(int fd,unsigned flag);
    int           AddEpollIO(int fd,unsigned flag);
    int           ModEpollIO(int fd,unsigned flag);
    int           DelEpollIO(int fd);

    bool          IOEPoll(void);

  private:
    SOCKET             m_Listenfd;
    int                m_ListenPort; // 

    int                m_Epollfd;

    struct epoll_event ev;
    struct epoll_event Events[EPOLL_SIZE];

    char               m_RecvBuf[MAX_RECV_BUF_LEN];
    char               m_SendBuf[MAX_SEND_BUF_LEN];
  };
}
#endif // _NETTHREAD_LIUNX_H_