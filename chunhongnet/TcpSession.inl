#ifndef _TCPSESSION_INL_
#define _TCPSESSION_INL_


inline SOCKET TcpSession::GetSocket(void) const
{ 
  return m_iSoket; 
}

inline void TcpSession::SetSocket(SOCKET sock)
{
  assert(sock != INVALID_SOCKET);
  m_iSoket = sock;
}

inline circular_buffer* TcpSession::GetRecvBuf(void)
{
  return pRecvBuf; 
}

inline circular_buffer* TcpSession::GetSendBuf(void) 
{
  return pSendBuf; 
}


#endif // _TCPSESSION_INL_