
#include "udpsocket.h"

CUDPSocket::CUDPSocket() 
{
    m_hSocket = (SOCKET)NULL;
}

CUDPSocket::~CUDPSocket()
{
//	Close();
}


CUDPSocket::CUDPSocket(HANDLE hSocket)
{
    m_hSocket = (SOCKET)hSocket;
}

int CUDPSocket::Create(DWORD &dwPort)
{
    if (m_hSocket.socket_valid())
    {
        return -1 ;
    }

    if (!m_hSocket.socket_create(AF_INET, SOCK_DGRAM, 0))
    {
        return -1;
    }

   // int dwRecvBufSize = GetSocketRecvBufSize();
    //int dwSendBufSize = GetSocketSendBufSize();

    //if (!m_hSocket.socket_set_opt( SOL_SOCKET, SO_RCVBUF, (const char *)&dwRecvBufSize, sizeof(DWORD)))
    //{
    //    //ASSERT(0);
    //    printf("setsockopt error %d\n",errno);
        //STRACE(("ERR: setsockopt--RcvBuf·¢ÉúŽíÎó #%d\n"), WSAGetLastError());
   // }
   // if (!m_hSocket.socket_set_opt( SOL_SOCKET, SO_SNDBUF, (const char *)&dwSendBufSize, sizeof(DWORD)))
   // {
   //     //ASSERT(0);
   //     //STRACE(("ERR: setsockopt--SndBuf·¢ÉúŽíÎó #%d\n"), WSAGetLastError());
   //     printf("setsockopt error %d\n",errno);
   // }

    DWORD   dwBytesReturned = 0;
    BOOL    bNewBehavior = FALSE;
    DWORD   status;

    if (!m_hSocket.udp_socket_bind(htonl(INADDR_ANY),(short unsigned int&)dwPort))
    {
        return -1;
    }

    return 0;
}

void CUDPSocket::Close()
{
    if (!m_hSocket.socket_valid())
    {
        return;
    }

    m_hSocket.socket_close();
    m_hSocket.zero_socket();
}

void CUDPSocket::Cleanup()
{

}

// œÓÊÕÐÅÏ¢
long CUDPSocket::Read(LPUDPPER_IO_OPERATION_DATA lpPerIoData, DWORD dwFlags)
{
    ASSERT(lpPerIoData);

    DWORD dwRecvBytes = 0, dwErrCode ;
    lpPerIoData->dwOpType = RECV_MSG;
    int recvCount = 0;
    sockaddr_in fromAddr;
    int SenderAddrSize = sizeof(fromAddr);


    while (SOCKET_ERROR == (dwRecvBytes = m_hSocket.socket_recv_from((char*)(lpPerIoData->DataBuf.buf), lpPerIoData->DataBuf.len, dwFlags,&fromAddr, &SenderAddrSize)))
    {

        if (recvCount % 10 == 0 && recvCount != 0)
        {
            if (recvCount == 100)
            {
                //ASSERT(0);	//ŽËŽŠÔÚÍøÂç¶Ï¿ªµÄÇé¿öÏÂºÜÈÝÒ×Âú×ãrecvCount == 100ÕâžöÌõŒþ£¬²»ÓÃASSERT
                return -1;
            }
            Sleep(1000);
        }
    }

    //TRACE(("info: (socket=0x%x) recv cnt = 0x%x "), m_hSocket,dwRecvBytes);

    return dwRecvBytes;
}

// ·¢ËÍÏûÏ¢
long CUDPSocket::Write(LPCSOCKADDR lpSockAddr, LPUDPPER_IO_OPERATION_DATA lpPerIoData, DWORD dwFlags)
{

    ASSERT(lpPerIoData);

    DWORD dwSendBytes = 0;
    DWORD dwErrcode;
    lpPerIoData->dwOpType = SEND_MSG;

    dwSendBytes = m_hSocket.socket_send_to((char*)(lpPerIoData->DataBuf.buf), (int)lpPerIoData->DataBuf.len, (int)dwFlags, (struct sockaddr_in*)lpSockAddr, (int)sizeof(sockaddr));

    return dwSendBytes;
}
