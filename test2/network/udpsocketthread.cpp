
#include "udpsocketthread.h"
#define STRACE
#define SUPER_TRACE

#pragma warning(disable: 4101)

CUDPSocketThread::CUDPSocketThread(RECVUDPMSGHANDLE_FUNC lpfnOnReceive, DWORD dwUserData/* = 0*/)
: m_lpfnOnReceive(lpfnOnReceive), m_dwUserData(dwUserData)
{
    m_dwNumCurrentThreads = 1;
    m_hCompletionPort = NULL;


    m_dwListenPort = 0;

    m_lpUDPPerIOData = NULL;

    m_pUdpBuf = new CMemItem;
}

CUDPSocketThread::~CUDPSocketThread()
{
    StopService();

    if (NULL != m_lpUDPPerIOData)
    {
        m_lpUDPPerIOData->Uninitialize();
        delete m_lpUDPPerIOData;
        m_lpUDPPerIOData = NULL;
    }
}

BOOL CUDPSocketThread::StartService(DWORD &dwPort)
{
    BOOL bRet = FALSE;

    if (dwPort != 0)
    {
        m_dwListenPort = dwPort ;
    }
    else
    {
        ASSERT(m_dwListenPort > 0) ;
    }

    bRet = StartListenThread(dwPort);
    if (FALSE == bRet)
    {

        return bRet;
    }

    m_dwListenPort = dwPort; 
    bRet = StartWorkThread();
    if (FALSE == bRet)
    {

        return bRet;
    }



    return bRet;
}

BOOL CUDPSocketThread::StopService()
{
    return FALSE;
}

#ifdef _WIN32
BOOL CUDPSocketThread::CreateNewCompletionPort(DWORD dwNumberOfConcurrentThreads)
{
    m_dwNumCurrentThreads = dwNumberOfConcurrentThreads;

    m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, m_dwNumCurrentThreads);
    if (NULL == m_hCompletionPort)
    {
        return FALSE;
    }

    return TRUE;
}
#endif

LPUDPPER_IO_OPERATION_DATA CUDPSocketThread::AssociateSocketWithCompletionPort()
{
    ASSERT(m_cListenSocket.GetSocketHandle());

    HANDLE hResult = NULL;

    LPUDPPER_IO_OPERATION_DATA lpUDPPerIOData = NULL;


    if (NULL == (lpUDPPerIOData = CreateInfo(m_cListenSocket)))
    {
        return NULL;
    }

    m_lpUDPPerIOData = lpUDPPerIOData;

    return m_lpUDPPerIOData;
}

void*  __stdcall CUDPSocketThread::_ServerWorkThread(LPVOID lpParam)
{
    printf("[UDP] 1 enter _ServerWorkThread\n");
    ASSERT(lpParam);

    CUDPSocketThread *pUDPSocketThread = (CUDPSocketThread *)lpParam;

    DWORD dwNumBytes,dwRet;
    LPUDPPER_HANDLE_DATA lpUDPPerHandleData = NULL;
    LPUDPPER_IO_OPERATION_DATA lpUDPPerIOConText = NULL;

    lpUDPPerIOConText = pUDPSocketThread->m_lpUDPPerIOData;
    BOOL bSuccess = FALSE;

    fd_set  readfds;
    int n_ret = 0;


    while (TRUE)
    {

        int fd = (int)(long)pUDPSocketThread->m_cListenSocket.GetSocketHandle();
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        n_ret = select(fd+1, &readfds, NULL, NULL,0);

        if (n_ret == -1)
        {
            if (EINTR == errno)
            {
                STRACE(("Select is Interrupted by system call\n"));
                continue;
            }
            printf("select error [%d] %s\n",errno,strerror(errno));
            ASSERT(0);
            //exit(-1);
        }
        else if (n_ret ==0)
        {
            SUPER_TRACE("select timeout,continue circle\n");
            continue;
        }

        if (FD_ISSET((int)(long)pUDPSocketThread->m_cListenSocket.GetSocketHandle(),&readfds))
        {

            n_ret = pUDPSocketThread->m_cListenSocket.Read(pUDPSocketThread->m_lpUDPPerIOData);

            if (n_ret <= 0)
            {
                // closeclient
                FD_CLR((int)(long)pUDPSocketThread->m_cListenSocket.GetSocketHandle(), &readfds);

                //fd_sock[n_index] = 0;

                continue;
            }

            pUDPSocketThread->m_lpUDPPerIOData->pMsgBuf->SetDataRealSize(n_ret);


            dwRet = pUDPSocketThread->ProcessMsg(lpUDPPerIOConText, n_ret);

        }

        continue;
    }

    return 0;
}

DWORD CUDPSocketThread::ProcessMsg(LPUDPPER_IO_OPERATION_DATA lpUDPPerIOData, DWORD dwNumBytes)
{
    DWORD dwFlags;
    DWORD dwRet=0;

    DWORD SendBytes,RecvBytes;
    int DataItem;
    DWORD dwFi = MAKEFOURCC('F','I','S','H');

    if (lpUDPPerIOData->dwOpType == RECV_MSG)
    {
        if (*((DWORD*)(lpUDPPerIOData->pMsgBuf->GetBufferPtr()) + 1) != dwFi)
        {

            ASSERT(0);
            return 0;
        }
    }

    switch (lpUDPPerIOData->dwOpType)
    {
    case RECV_MSG:

        m_BufLock.Lock();
        m_pUdpBuf->AddData(lpUDPPerIOData->pMsgBuf->GetBufferPtr(), lpUDPPerIOData->pMsgBuf->GetDataRealSize());
        while (*(DWORD*)m_pUdpBuf->GetBufferPtr() <= m_pUdpBuf->GetDataRealSize() && m_pUdpBuf->GetDataRealSize()!=0)
        {
            if (*(DWORD*)m_pUdpBuf->GetBufferPtr() < 2*sizeof(DWORD))
            {
                break;
            }
            CMemItem *Buf = new CMemItem;
            Buf->AddData(m_pUdpBuf->GetBufferPtr(), *(DWORD*)m_pUdpBuf->GetBufferPtr());    
            m_pUdpBuf->SetCurPos(*(DWORD*)m_pUdpBuf->GetBufferPtr());
            m_pUdpBuf->MoveUnusedDataToBufTop();

            m_lpfnOnReceive(Buf, m_dwUserData);
        }
        m_pUdpBuf->Reset();
        m_BufLock.Unlock();

        break;
    case SEND_MSG:
        OnSendMsg();
        break;
    default:
        break;
    }

    return dwRet;
}

BOOL CUDPSocketThread::StartListenThread(DWORD &dwPort)
{

    if (m_cListenSocket.Create(dwPort) == -1)
    {
        return FALSE;
    }

    if (NULL == AssociateSocketWithCompletionPort())
    {
        ASSERT(0);
        return FALSE;
    }

    return TRUE;
}

BOOL CUDPSocketThread::StartWorkThread()
{

    if (0 != create_thread(&m_hWorkThread, NULL, _ServerWorkThread, (LPVOID)this))
    {
        return FALSE;
    }

    return TRUE;
}

VOID CUDPSocketThread::CloseClient(LPUDPPER_HANDLE_DATA lpUDPPerHandleData)
{
    if (lpUDPPerHandleData)
    {
        lpUDPPerHandleData->Socket = INVALID_SOCKET;

        lpUDPPerHandleData = NULL;
    }


    return;    
}

void CUDPSocketThread::OnAccept(HANDLE hSocket)
{
    STRACE(("MSG: OnAccept!\n"));
}

void CUDPSocketThread::OnReceive(HANDLE hSocket, CMemItem *pMsgBuf)
{

}

void CUDPSocketThread::OnSendMsg()
{
    STRACE(("MSG: OnSendMsg!\n"));
}

DWORD CUDPSocketThread::SendUDPMsg(LPCSOCKADDR lpSockAddr, CMemItem *pSendMsgBuf)
{   
    DWORD dwSendBytes = 0 ;

    LPUDPPER_IO_OPERATION_DATA lpUDPPerIOData = new UDPPER_IO_OPERATION_DATA ;
    ZeroMemory(&lpUDPPerIOData->Overlapped , sizeof(lpUDPPerIOData->Overlapped ));

    lpUDPPerIOData->pMsgBuf = pSendMsgBuf;  
    lpUDPPerIOData->DataBuf.buf = (char *)lpUDPPerIOData->pMsgBuf->m_ptrData ;
    lpUDPPerIOData->DataBuf.len  = lpUDPPerIOData->pMsgBuf->m_dwDataRealSize  ;

    dwSendBytes = m_cListenSocket.Write(lpSockAddr, lpUDPPerIOData);


    _SafeDeletePtr(lpUDPPerIOData);

    return dwSendBytes;
}

LPUDPPER_IO_OPERATION_DATA CUDPSocketThread::CreateInfo(CUDPSocket &udpSocket)
{

    LPUDPPER_IO_OPERATION_DATA lpPerIOData = NULL;

    lpPerIOData = new UDPPER_IO_OPERATION_DATA;
    if (FALSE == lpPerIOData->Initialize(udpSocket))
    {
        return NULL;
    }

    return lpPerIOData; 
}

void CUDPSocketThread::OnClose(HANDLE hSocket)
{

}

