#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_

#include <string>
using namespace std;

namespace NetWork
{
  const int iRecvBufferSize = 1024 * 64;
  const int iSendBufferSize = 1024 * 64;

  enum ETcpClientState
  {
    ETCP_STATE_NONE,       // none
    ETCP_STATE_CONNECT,    // connecting
    ETCP_STATE_START,      // already
    ETCP_STATE_DISCONNECT, // disconnecting
    ETCP_STATE_CLOSE,      // Close Socket
    ETCP_STATE_MAX
  };

#if defined(__LINUX__)
#define LOG_TCPCLIENT_ERROR()\
  do\
  {\
    LogOutErrorString(_("%s %d %s ERROR: %s"), __FILE__, __LINE__, __FUNCTION__, strerror(errno));\
  }while (FALSE);
#else
#define LOG_TCPCLIENT_ERROR()\
  do\
  {\
   LogOutErrorString(_("%s %d %s ERROR: %d"), __FILE__, __LINE__, __FUNCTION__, ::WSAGetLastError());\
  } while (FALSE);
#endif // __LINUX__



  class TcpClient
  {
  public:
    TcpClient();
    ~TcpClient();
    DISALLOW_COPY_AND_ASSIGN(TcpClient);

  public:
    bool InitNet(void);
    void NetPoll(void);

    void Stop();

    void SetIPAddres(const char* szIP, U16 iPort);

    inline const char* GetServerIP(void) const { return Address.IP().c_str(); }
    inline U16 GetServerPort(void) const { return Address.Port(); }

    inline TcpSession* GetTcpSession() { return pSession; }
    void SetTcpSession(TcpSession* pTcp);

    void ChangeState(ETcpClientState eState);

  private:
    inline void SetState(ETcpClientState eState) { TcpState = eState; }

  public:
    bool Connect(const char* pIP, int iPort);
    bool Connect(void);

    bool NetEvent(void);
    bool Disconnect();

  public:
    void RegisterEvent(ETcpClientState eState, FastDelegate0<> Handler);
    void UnregisterEvent(ETcpClientState eState, FastDelegate0<> Handler);

  private:
    void NoBlockSocket(SOCKET Sock);
    void CloseSocket();
    void CloseATcpSession(TcpSession* pSession);

    bool Reconnect(const char* pIP, int iPort);
    bool Reconnect(void);

  private:
    SOCKET Socket; // SOCKET ÃèÊö·û

    ETcpClientState TcpState;

    IPAddress Address;  // Server IP Address

    // Protocol
    TcpSession* pSession;

    // Transport Buf
    char RecvBuf[MAX_RECV_BUF_LEN];
    char SendBuf[MAX_SEND_BUF_LEN];

    // FSM
    FSM<TcpClient>* pFsmBrain;

  public:
    FastDelegate0<> TcpClientEventHandler[ETCP_STATE_MAX];
  };
}

#endif // _TCPCLIENT_H_