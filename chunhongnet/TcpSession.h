#ifndef _TCPSESSION_H_
#define _TCPSESSION_H_

namespace NetWork
{
  // Base class Deal all tcp session
  class LIBNET_EXPORT TcpSession
  {
  public:
    enum ESessionStatus
    {
      ESESSION_STATUS_NONE,
      ESESSION_STATUS_START,
      ESESSION_STATUS_CLOSE,
    };

  public:
    TcpSession(int irecvbufsize, int isendbufsize);
    ~TcpSession();

  public:
    virtual void InitUse(NetBase* pNet, SOCKET iSock, const sockaddr_in& Addr);

    inline SOCKET GetSocket(void) const;

    void ClearBuf();
    inline circular_buffer* GetRecvBuf(void);
    inline circular_buffer* GetSendBuf(void);

  public:
    BOOL IsClosed(void);
  protected:
    void ChgSessionState(ESessionStatus estate);
    ESessionStatus GetSessionState(void);

    virtual void FinishSession();

  public:
    virtual void SendAllKey();
    virtual bool ReadAllKey();

  private:
    inline void SetSocket(SOCKET sock);

  private:
    SOCKET m_iSoket;
    IPAddress Address;

    CPLCriticalSection mutexStatus;
    ESessionStatus m_Status;

  private:
    NetBase* pTcpServer;

  private:
    circular_buffer* pRecvBuf;
    circular_buffer* pSendBuf;

    friend class NetBase;
    friend class TcpServer;
    friend class TcpClient;
  };


  class LIBNET_EXPORT TcpSessionFactory
  {
  public:
    TcpSessionFactory()
    {

    }

    virtual ~TcpSessionFactory()
    {

    }


  public:
    virtual void Init(unsigned int iClientNum, const char* Name) = 0;
    virtual TcpSession* CreateATcpSession(const char* IP, int iPort) = 0;  // Get A Free Tcp Session from memory pool
    //virtual void FreeATcpSession(TcpSession* pSession) = 0; // put back A Tcp Session to memory pool
  };

#include "TcpSession.inl"
}

#endif // !_TCPSESSION_H_
