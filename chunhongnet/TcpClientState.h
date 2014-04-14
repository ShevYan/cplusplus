#ifndef _TCPCLIENT_STATE_H_
#define _TCPCLIENT_STATE_H_

namespace NetWork
{
  // Deal TcpClient Connect Event
  class TcpClientConnect : public TPLSingleton<TcpClientConnect>, public IFSMState<TcpClient>
  {
  public:
    TcpClientConnect()
    {
    }

    ~TcpClientConnect()
    {
    }
    DISALLOW_COPY_AND_ASSIGN(TcpClientConnect);

  public:
    void Enter(TcpClient* pEnt);
    void Execute(TcpClient* pEnt);
    void Exit(TcpClient* pEnt);
  };

  // Deal TcpClient Start Event
  class TcpClientRun : public TPLSingleton<TcpClientRun>, public IFSMState<TcpClient>
  {
  public:
    TcpClientRun()
    {
    }

    ~TcpClientRun()
    {
    }
    DISALLOW_COPY_AND_ASSIGN(TcpClientRun);

  public:
    void Enter(TcpClient* pEnt);
    void Execute(TcpClient* pEnt);
    void Exit(TcpClient* pEnt);
  };

  // Deal TcpClient Disconnect Event
  class TcpClientDisconnect : public TPLSingleton<TcpClientDisconnect>, public IFSMState<TcpClient>
  {
  public:
    TcpClientDisconnect()
    {
    }

    ~TcpClientDisconnect()
    {
    }
    DISALLOW_COPY_AND_ASSIGN(TcpClientDisconnect);

  public:
    void Enter(TcpClient* pEnt);
    void Execute(TcpClient* pEnt);
    void Exit(TcpClient* pEnt);
  };

  // Deal TcpClient Close Event
  class TcpClientClose : public TPLSingleton<TcpClientClose>, public IFSMState<TcpClient>
  {
  public:
    TcpClientClose()
    {
    }

    ~TcpClientClose()
    {
    }
    DISALLOW_COPY_AND_ASSIGN(TcpClientClose);

  public:
    void Enter(TcpClient* pEnt);
    void Execute(TcpClient* pEnt);
    void Exit(TcpClient* pEnt);
  };
}

#endif // _TCPCLIENT_STATE_H_