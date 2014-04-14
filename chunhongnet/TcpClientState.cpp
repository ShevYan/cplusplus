#include "NetPrivate.h"

namespace NetWork
{
  // Connect Server 
  void TcpClientConnect::Enter(TcpClient* pEnt)
  {
    if(pEnt->Connect())
    {
      pEnt->ChangeState(ETCP_STATE_START);
    }
  }

  void TcpClientConnect::Execute(TcpClient* pEnt)
  {
  }

  void TcpClientConnect::Exit(TcpClient* pEnt)
  {
  }

  // Tcp Net Start Work
  void TcpClientRun::Enter(TcpClient* pEnt)
  {
  }

  void TcpClientRun::Execute(TcpClient* pEnt)
  {
    if(!pEnt->NetEvent())
    {
      pEnt->ChangeState(ETCP_STATE_DISCONNECT);
    }
  }

  void TcpClientRun::Exit(TcpClient* pEnt)
  {
  }

  // Diconnect
  void TcpClientDisconnect::Enter(TcpClient* pEnt)
  {
    //pEnt->ChangeState(ETCP_STATE_CONNECT);
  }

  void TcpClientDisconnect::Execute(TcpClient* pEnt)
  {
  }

  void TcpClientDisconnect::Exit(TcpClient* pEnt)
  {
  }

  // Clost Net Work
  void TcpClientClose::Enter(TcpClient* pEnt)
  {
    pEnt->Disconnect();
    pEnt->ChangeState(ETCP_STATE_NONE);
  }

  void TcpClientClose::Execute(TcpClient* pEnt)
  {
  }

  void TcpClientClose::Exit(TcpClient* pEnt)
  {
  }
}