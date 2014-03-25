
#include "tcpsocket.h"

#if (defined(_WIN32) || defined(_WIN64))
#include "ws2tcpip.h"
#endif

struct tcp_keepalive 
{
    u_long  onoff;
    u_long  keepalivetime;
    u_long  keepaliveinterval;
};

#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,4)

CTCPSocket::CTCPSocket()
{
    m_hSocket.zero_socket();
    m_dwPeerPort = 0;
    m_IsRev = 0;       
    m_uRemotePort = 0;
    m_dwRemoteIP = 0;
    m_dwLocalIP = 0;
    m_uHostType = 0;    
	m_bAutoConn = false;
}

CTCPSocket::CTCPSocket(const CTCPSocket&sock)
{
    m_hSocket = sock.m_hSocket;
    m_dwPeerPort = sock.m_dwPeerPort;
    m_IsRev = sock.m_IsRev;       
    m_uRemotePort = sock.m_uRemotePort;
    m_dwRemoteIP = sock.m_dwRemoteIP;
    m_dwLocalIP = sock.m_dwLocalIP;
    m_uHostType = sock.m_uHostType; 
	m_bAutoConn = sock.m_bAutoConn;
}

CTCPSocket::CTCPSocket(HANDLE hSocket)
{
    m_hSocket = (SOCKET)hSocket;
    m_dwPeerPort = 0;
    m_IsRev = 0;       
    m_uRemotePort = 0;
    m_dwRemoteIP = 0;
    m_dwLocalIP = 0;
    m_uHostType = 0;    
	m_bAutoConn = false;
}

CTCPSocket::~CTCPSocket()
{
}

void CTCPSocket::Create()
{
    if (m_hSocket.socket_valid())
    {
        return ;
    }

    if (!m_hSocket.socket_create(AF_INET, SOCK_STREAM, 0))
    {
        return;
    }

    int dwRecvBufSize = SOCKET_RECV_BUFFER_SIZE;
    int dwSendBufSize = SOCKET_SEND_BUFFER_SIZE; 
    BOOL bNoDelay     = TRUE;
    int nREUSEADDR    = 1;

//  int flag = ::fcntl(m_hSocket.get_socket(), F_GETFL, 0);
//  int err = ::fcntl(m_hSocket.get_socket(), F_SETFL, flag | O_NONBLOCK);

    if (!m_hSocket.socket_set_opt(SOL_SOCKET, SO_RCVBUF, (const char *)&dwRecvBufSize, sizeof(int)))
    {
        MASTRACE("ERR: setsockopt--RcvBuf·¢ÉúŽíÎó #%d.\n", m_hSocket.get_last_error());
    }

    if (!m_hSocket.socket_set_opt(SOL_SOCKET, SO_SNDBUF, (const char *)&dwSendBufSize, sizeof(int)))
    {
        MASTRACE("ERR: setsockopt--SndBuf·¢ÉúŽíÎó #%d.\n", m_hSocket.get_last_error());
    }

    if (!m_hSocket.socket_set_opt(SOL_SOCKET, SO_REUSEADDR, (const char*)&nREUSEADDR, sizeof(int)))
    {
        MASTRACE("ERR: setsockopt--SO_REUSEADDRf·¢ÉúŽíÎó #%d.\n", m_hSocket.get_last_error());      
    }

    if (!m_hSocket.socket_set_opt(IPPROTO_TCP, TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay)))
    {
        MASTRACE("ERR: setsockopt--TCP_NODELAY·¢ÉúŽíÎó #%d.\n", m_hSocket.get_last_error());        
    }
#ifdef FI_MAC
    int set = 1;
    if (!m_hSocket.socket_set_opt(SOL_SOCKET,SO_NOSIGPIPE,(char*)&set,sizeof(int)))
    {
        SUPER_TRACE("ERR: setsockopt--TCP_NODELAY·¢ÉúŽíÎó #%d.", m_hSocket.get_last_error());
    }
#endif	

    int keepAlive   = 1;    // set keep alive flag
    int keepIdle    = 5;    // 30s, begin to send first keep alive packet after 30s
    int keepInterval= 5;    // 5s, between two packet send time
    int keepCount   = 3;    // confirm disconnect after 3 times try

    if (!m_hSocket.socket_set_opt(SOL_SOCKET, SO_KEEPALIVE, (char*)&keepAlive, sizeof(keepAlive)))
    {
        MASTRACE("ERR: setsockopt--SO_KEEPALIVE %d.", errno);       
    }
#ifdef FI_MAC

    int on = 1;
    if (!m_hSocket.socket_set_opt(SOL_SOCKET, SO_NOSIGPIPE,(char *)&on, sizeof(on)))
    {
        MASTRACE("ERR: setsockopt--SO_NOSIGPIPE %d.", errno);   
    }
#endif

#ifdef FI_LINUX
    if (!m_hSocket.socket_set_opt(SOL_TCP, TCP_KEEPIDLE, (char*)&keepIdle, sizeof(keepIdle)))
    {
        MASTRACE("ERR: setsockopt--TCP_KEEPIDLE %d.",errno);
        printf("ERR: setsockopt--TCP_KEEPIDLE %d\n.",errno);
    }

    if (!m_hSocket.socket_set_opt( SOL_TCP, TCP_KEEPINTVL, (char *)&keepInterval, sizeof(keepInterval)))
    {
        MASTRACE("ERR: setsockopt--TCP_KEEPINTVL %d.",errno);
        printf("ERR: setsockopt--TCP_KEEPINTVL %d.\n",errno);
    }

    if (!m_hSocket.socket_set_opt( SOL_TCP, TCP_KEEPCNT, (char *)&keepCount, sizeof(keepCount)))
    {
        MASTRACE("ERR: setsockopt--TCP_KEEPCNT %d.",errno);
        printf("ERR: setsockopt--TCP_KEEPCNT %d.\n",errno);
    }

    int nNetTimeout = 3000;
    
    m_hSocket.socket_set_opt( SOL_SOCKET, SO_SNDTIMEO, (const char *)&nNetTimeout, sizeof(int));
    m_hSocket.socket_set_opt( SOL_SOCKET, SO_RCVTIMEO, (const char *)&nNetTimeout, sizeof(int));

#endif
}

void CTCPSocket::Close()
{
    if (!m_hSocket.socket_valid())
    {
        return;
    }

    m_hSocket.socket_close();
    m_hSocket.zero_socket();
}

void CTCPSocket::Cleanup()
{
    Close();
}

void CTCPSocket::Bind(DWORD dwIp)
{
    if (!m_hSocket.tcp_bind(dwIp, 0))
    {
        return;
    }
}

void CTCPSocket::Bind(DWORD dwIp,DWORD &dwPort)
{
    if (!m_hSocket.tcp_socket_bind(dwIp, (unsigned short &)dwPort))
    {
        ASSERT(0);
        return;
    }
}

int CTCPSocket::Listen(USHORT nPort, DWORD dwQueueSize)
{
    if (!m_hSocket.socket_valid())
    {
        return -1;
    }

    if (!m_hSocket.tcp_socket_bind(INADDR_ANY, nPort))
    {
        return -1;
    }

    if (!m_hSocket.socket_listen(dwQueueSize))
    {
        return -1;
    }

    return 0;
}

int CTCPSocket::Listen(DWORD dwBindIP, USHORT nPort, DWORD dwQueueSize)
{
    if (!m_hSocket.socket_valid())
    {
        return -1;
    }

    if (!m_hSocket.tcp_socket_bind(dwBindIP, nPort))
    {
        return -1;
    }

    if (!m_hSocket.socket_listen(dwQueueSize))
    {
        return -1;
    }
    
    m_dwLocalIP = dwBindIP;
    return 0;
}

void CTCPSocket::Accept(CTCPSocket &sConnect)
{
    if (!m_hSocket.socket_valid())
    {
        return;
    }
#if !(defined(_WIN32) || defined(_WIN64))
    struct sockaddr addr;
#else
	sockaddr addr;
#endif
    memset(&addr, 0, sizeof(sockaddr));

    int n_addr_len = sizeof(sockaddr);

    if (!m_hSocket.socket_accept(sConnect.m_hSocket, &addr, n_addr_len))
    {

#if !(defined(_WIN32) || defined(_WIN64))
        if (ENOTSOCK != m_hSocket.get_last_error())
        {
        }
#endif
        return ;
    }
}

void CTCPSocket::Accept(CTCPSocket &sConnect, LPSOCKADDR psa, int &len)
{
    if (!m_hSocket.socket_valid())
    {
        return;
    }

    if (!m_hSocket.socket_accept(sConnect.m_hSocket, psa, len))
    {
#if !(defined(_WIN32) || defined(_WIN64))
        if (ENOTSOCK != m_hSocket.get_last_error())
        {
        }
#endif
        return ;
    }
}

BOOL CTCPSocket::Connect(LPCSOCKADDR psa)
{
    BOOL bRet = FALSE;
    if (!m_hSocket.socket_valid())
    {
        ASSERT(0);
        return bRet;
    }


    if (!m_hSocket.socket_connect(psa))
    {
        return false;
    }

    if (!m_hSocket.socket_getport((unsigned short &)m_dwPeerPort))
    {
        ASSERT(0);
        m_dwPeerPort = 0;
    }


    return true;
}

int CTCPSocket::Read(LPPER_IO_OPERATION_DATA lpPerIoData, DWORD dwFlags)
{   
    if (!m_hSocket.socket_valid())
    {
        return 0;
    }

    if (lpPerIoData == NULL)
    {
        return 0;
    }

    lpPerIoData->dwOpType = RECV_MSG;

    int dwRecvBytes = m_hSocket.socket_recv(lpPerIoData->DataBuf.buf, lpPerIoData->DataBuf.len, dwFlags);

    if (SOCKET_ERROR == dwRecvBytes)
    {
#if !(defined(_WIN32) || defined(_WIN64))
        if (ENOBUFS == m_hSocket.get_last_error())
        {
            printf("ENOBUFS == m_hSocket.get_last_error() errstr[%s] \n", strerror(errno));
            //lpPerIoData->pMsgBuf->SetBufferSize(2*g_FimasPara.m_dwReadBufSize);
            lpPerIoData->pMsgBuf->SetBufferSize(2*2048);
            lpPerIoData->DataBuf.len = lpPerIoData->pMsgBuf->m_dwBackBufSize;

            dwRecvBytes = CTCPSocket::Read(lpPerIoData,dwFlags);
        }
        else
        {
            printf("m_hSocket.get_last_error() errstr[%s] \n", strerror(errno));
        }
#else
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			return 0;
		}

		if(WSA_NOT_ENOUGH_MEMORY == WSAGetLastError())
		{
			lpPerIoData->pMsgBuf->SetBufferSize(2*2048);
			lpPerIoData->DataBuf.len = lpPerIoData->pMsgBuf->m_dwBackBufSize;
			CTCPSocket::Read(lpPerIoData,dwFlags);
		}
#endif
    }

    return dwRecvBytes;
}

DWORD CTCPSocket::Write(LPPER_IO_OPERATION_DATA lpPerIoData, DWORD dwFlags)
{
    if (!m_hSocket.socket_valid())
    {
        return 0;
    }

    if (lpPerIoData == NULL)
    {
        return 0;
    }

#ifdef FI_LINUX
    dwFlags = MSG_NOSIGNAL;
#endif
    lpPerIoData->dwOpType = SEND_MSG;

    //m_sockLock.Lock();

    DWORD dwSendBytes = m_hSocket.socket_send(lpPerIoData->DataBuf.buf, lpPerIoData->DataBuf.len, dwFlags);

    if (SOCKET_ERROR == dwSendBytes)
    {
        //m_sockLock.Unlock();

        return 0;
    }
    else
    {
        //MASTRACE("MSG: Send Msg Successed(bytes=%d,Socket=%d).\n", dwSendBytes, (int)m_hSocket.get_socket());
    }

    //m_sockLock.Unlock();

    return dwSendBytes;
}

DWORD CTCPSocket::WriteBlks(LPPER_IO_OPERATION_DATA lpPerIoData,LPPER_IO_OPERATION_DATA lpDataBuf,DWORD &dwStatus, DWORD dwFlags)
{
    if (!m_hSocket.socket_valid())
    {
        return 0;
    }

    if (lpPerIoData == NULL)
    {
        return 0;
    }

    lpPerIoData->dwOpType = SEND_MSG;
    lpDataBuf->dwOpType = SEND_MSG;

    //m_sockLock.Lock();

#ifdef FI_LINUX
    dwFlags = MSG_NOSIGNAL;
#endif

    DWORD dwSendBytes = m_hSocket.socket_send(lpPerIoData->DataBuf.buf, lpPerIoData->DataBuf.len, dwFlags);

    if (SOCKET_ERROR == dwSendBytes)
    {
        //m_sockLock.Unlock();

        return 0;
    }
    else
    {
        //MASTRACE("MSG: Send Msg Successed(bytes=%d,Socket=%d).\n", dwSendBytes, (int)m_hSocket.get_socket());
    }

    DWORD dwTmpSend = m_hSocket.socket_send(lpDataBuf->DataBuf.buf, lpDataBuf->DataBuf.len, dwFlags);

    if (SOCKET_ERROR == dwTmpSend)
    {
        //m_sockLock.Unlock();

        return 0;
    }
    else
    {
        //MASTRACE("MSG: Send Msg Successed(bytes=%d,Socket=%d).\n", dwTmpSend, (int)m_hSocket.get_socket());
    }

    //m_sockLock.Unlock();

    dwSendBytes += dwTmpSend;

    return dwSendBytes;
}
void CTCPSocket::RecConnInfo(DWORD dwLocalIP, DWORD dwRemoteIP, USHORT dwRemotePort)
{
    m_dwLocalIP = dwLocalIP;
    m_dwRemoteIP = dwRemoteIP;
    m_uRemotePort = dwRemotePort;
}

BOOL CTCPSocket::SendMsg(CMemItem *pSendMsgBuf, BOOL isASASynWrite)
{
    DWORD dwSendBytes = 0;
    DWORD dwBufIndex = 0;
    CTCPSocket tcpSocket((HANDLE)m_hSocket.get_socket());

    PER_IO_OPERATION_DATA tmpPerIOData;
    ZeroMemory(&tmpPerIOData.Overlapped,sizeof(tmpPerIOData.Overlapped));

    tmpPerIOData.pMsgBuf = pSendMsgBuf;
    tmpPerIOData.DataBuf.buf = (char *)tmpPerIOData.pMsgBuf->m_ptrData;
    tmpPerIOData.DataBuf.len = tmpPerIOData.pMsgBuf->m_dwDataRealSize;
    ASSERT(tmpPerIOData.DataBuf.len);

    do 
    {
        tmpPerIOData.DataBuf.buf = (char *)tmpPerIOData.pMsgBuf->m_ptrData + dwBufIndex;
        tmpPerIOData.DataBuf.len = tmpPerIOData.pMsgBuf->m_dwDataRealSize - dwBufIndex;

        dwSendBytes = tcpSocket.Write(&tmpPerIOData);
        dwBufIndex += dwSendBytes;

    } while (0 < dwBufIndex && dwBufIndex < pSendMsgBuf->m_dwDataRealSize);

    if (dwBufIndex != pSendMsgBuf->m_dwDataRealSize)
    {
        MASTRACE(("SOCK[%d] SEND MSG NOT INTEGRATED, IMAGE SIZE[%d], REAL SIZE[%d]"), 
                 m_hSocket.get_socket(), tmpPerIOData.pMsgBuf->m_dwDataRealSize, dwBufIndex);
        return FALSE;
    }

    return TRUE;
}

BOOL CTCPSocket::SendMsgBlks(CMemItem *pHead,PVOID pDataBuf,DWORD dwBufSize, DWORD &dwStatus,BOOL isASASynWrite)
{
    DWORD dwSendBytes = 0;
    DWORD dwBufIndex  = 0;
    CTCPSocket tcpSocket((HANDLE)m_hSocket.get_socket());

    PER_IO_OPERATION_DATA tmpPerIOData;
    ZeroMemory(&tmpPerIOData.Overlapped,sizeof(tmpPerIOData.Overlapped));

    PER_IO_OPERATION_DATA tmpDataBuf;
    ZeroMemory(&tmpDataBuf.Overlapped,sizeof(tmpDataBuf.Overlapped));

    tmpPerIOData.pMsgBuf = pHead;
    tmpPerIOData.DataBuf.buf = (char *)tmpPerIOData.pMsgBuf->m_ptrData;
    tmpPerIOData.DataBuf.len = tmpPerIOData.pMsgBuf->m_dwDataRealSize;

    tmpDataBuf.pMsgBuf = NULL;
    tmpDataBuf.DataBuf.buf = (char *)pDataBuf;
    tmpDataBuf.DataBuf.len = dwBufSize;

    ASSERT(dwBufSize);
    ASSERT(tmpPerIOData.pMsgBuf->m_dwDataRealSize);

    do 
    {
        tmpPerIOData.DataBuf.buf = (char *)tmpPerIOData.pMsgBuf->m_ptrData + dwBufIndex;
        tmpPerIOData.DataBuf.len = tmpPerIOData.pMsgBuf->m_dwDataRealSize - dwBufIndex;

        tmpDataBuf.DataBuf.buf = (char *)pDataBuf;
        tmpDataBuf.DataBuf.len = dwBufSize;

        dwSendBytes = tcpSocket.WriteBlks(&tmpPerIOData, &tmpDataBuf, dwStatus);
        dwBufIndex += dwSendBytes;

    } while (0 < dwBufIndex && dwBufIndex < tmpPerIOData.pMsgBuf->m_dwDataRealSize);

    while (0 < dwBufIndex && dwBufIndex < tmpPerIOData.pMsgBuf->m_dwDataRealSize + dwBufSize)
    {
        ASSERT(tmpPerIOData.pMsgBuf->m_dwDataRealSize <= dwBufIndex);

        tmpDataBuf.DataBuf.buf = (char *)pDataBuf + (dwBufIndex-tmpPerIOData.pMsgBuf->m_dwDataRealSize);
        tmpDataBuf.DataBuf.len = dwBufSize - ((dwBufIndex-tmpPerIOData.pMsgBuf->m_dwDataRealSize));

        dwSendBytes = tcpSocket.Write(&tmpPerIOData);
        dwBufIndex += dwSendBytes;
    }

    if (dwBufIndex != (tmpPerIOData.pMsgBuf->m_dwDataRealSize+tmpDataBuf.pMsgBuf->m_dwDataRealSize))
    {
        MASTRACE(("SOCK[%d] SEND MSG NOT INTEGRATED, IMAGE SIZE[%d], REAL SIZE[%d]"), m_hSocket.get_socket(),
                  tmpPerIOData.pMsgBuf->m_dwDataRealSize + tmpDataBuf.pMsgBuf->m_dwDataRealSize, dwBufIndex);

        return FALSE;
    }

    return TRUE;
}

void CTCPSocket::Reset()
{
    m_hSocket.zero_socket();
    m_IsRev       = 0;
    m_dwPeerPort  = 0;
    m_Error       = 0;
    m_uRemotePort = 0;
    m_dwRemoteIP  = 0;
    m_dwLocalIP   = 0;
    m_uHostType   = 0;
    m_bChecked    = 0;
}
