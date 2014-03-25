#include "tcpclient.h"
#include "FiUtility.h"
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _DEBUG
    #undef THIS_FILE
static char THIS_FILE[]=__FILE__;
    #define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTcpClient* CTcpClient::sm_TCPServer = NULL;
CTcpClient* CTcpClient::m_This = NULL;

CTcpClient* CTcpClient::GetInstance(USHORT nPort)
{
    if (sm_TCPServer == NULL)
    {
        MASTRACE(("¡Ÿ³õÊŒ»¯¡¿CTcpClient"));
        sm_TCPServer = new CTcpClient(nPort);
    }

    return sm_TCPServer;
}
void CTcpClient::DestoryInstance()
{
    if (sm_TCPServer != NULL)
    {
        _SafeDeletePtr(sm_TCPServer);
    }
}

CTcpClient::CTcpClient(USHORT nPort)
{
    m_This = this;
    m_state = true;
    m_bListenExit    = false;
    m_nPort          = nPort;
    m_SocketCreateCB = NULL;
    m_SocketExitCB   = NULL;
    m_SocketMsgCB    = NULL;
    m_ListenThread   = 0;

    m_CPUNumber = CPU_NUMBER;

    if (m_CPUNumber == 0)
    {
        m_CPUNumber = 1;
    }

    MASTRACE(("CTcpClient ³õÊŒ»¯ CPUžöÊý[%d]"), m_CPUNumber);
}

CTcpClient::~CTcpClient()
{
    m_bListenExit = true;

    map<pthread_t, int>::iterator it;
    DWORD dwExitSig = RPC_EXIT_CODE;

    MASTRACE("enter uninit rpcserver");

    for (it = m_SelectModeThreadActor.begin(); it != m_SelectModeThreadActor.end(); it++)
    {
        write(it->second, &dwExitSig, sizeof(DWORD));
    }

    Sleep(1);
    MASTRACE("middle uninit rpcserver");
    if (m_ListenThread)
    {
        pthread_cancel(m_ListenThread);
    }

    if (m_multiNetCardListenThread.size() > 0)
    {
        list<pthread_t>::iterator multiIt;

        for (multiIt=m_multiNetCardListenThread.begin(); multiIt!=m_multiNetCardListenThread.end(); multiIt++)
        {
            pthread_cancel(*multiIt);
        }
    }

    for (it = m_SelectModeThreadActor.begin(); it != m_SelectModeThreadActor.end(); it++)
    {
        if (m_SelectModeThreadEvent[it->first] != NULL)
        {
            WaitForSingleEvent(m_SelectModeThreadEvent[it->first], INFINITE);
            CloseEventHandle(m_SelectModeThreadEvent[it->first]);
        }
    }

    m_SelectModeThreadActor.clear();

    CloseAllSocket();

    MASTRACE("end uninit rpcserver");
}

BOOL CTcpClient::Init(SOCKETCREATE_FUNC createCB, void *createUserData, SOCKETEXIT_FUNC exitCB, 
                      void *exitUserData, SOCKETMSG_FUNC msgCB, void *msgUserData, DWORD dwBindIP)
{
    m_SocketCreateCB = createCB;
    m_SocketExitCB   = exitCB;
    m_SocketMsgCB    = msgCB;
    m_CreateUserDATA = createUserData;  
    m_ExitUserDATA   = exitUserData;
    m_MsgUserDATA    = msgUserData;
    m_dwBindIP       = dwBindIP;

#ifndef _MASCLIENT_

    // MasClient do not need to listen connection require
    // It is also used to surport MasClient multi-process

    if (!CreateListenThread())
    {
        MASTRACE("[FAIL] CreateListenThread FAIL\n");
        return false;
    }

#endif

    if (!CreateCompletionThread())
    {
        MASTRACE("[FAIL] CreateCompletionThread FAIL\n");
        return false;
    }

    return true;    
}

VOID CTcpClient::Init(SOCKETCREATE_FUNC createCB, void *createUserData, SOCKETEXIT_FUNC exitCB, 
                      void *exitUserData, SOCKETMSG_FUNC msgCB, void *msgUserData)
{
    m_SocketCreateCB = createCB;
    m_SocketExitCB   = exitCB;
    m_SocketMsgCB    = msgCB;
    m_CreateUserDATA = createUserData;  
    m_ExitUserDATA   = exitUserData;
    m_MsgUserDATA    = msgUserData;
}

BOOL CTcpClient::BindNetCard(DWORD dwCurNetCard)
{
    if (dwCurNetCard == 0)
    {
        return TRUE;
    }

    CTCPSocket sListenSocket;

    sListenSocket.Create();
    sListenSocket.Listen(dwCurNetCard, m_nPort, MAX_CLIENT);    // ×îŽóÁ¬œÓÊýMAX_CLIENT

    if (m_sListenSocketMap.find(dwCurNetCard) == m_sListenSocketMap.end())
    {
        m_sListenSocketMap.insert(map<DWORD, CTCPSocket>::value_type(dwCurNetCard, sListenSocket));
    }
    else
    {
        m_sListenSocketMap[dwCurNetCard] = sListenSocket;
    }

    pthread_t hListenThread = 0;

    if (0 != pthread_create(&hListenThread, NULL, _MultiNetCardTCPListenThread, (void*)dwCurNetCard))
    {
        sListenSocket.Cleanup();

        MASTRACE(("_beginthreadex _MultiNetCardTCPListenThread fail."));

        return FALSE;
    }

    m_multiNetCardListenThread.push_back(hListenThread);

    return TRUE;
}

void * __stdcall CTcpClient::_MultiNetCardTCPListenThread(LPVOID lpParam)
{
    /*
    DWORD dwCurNetCard = (DWORD)lpParam;
    LPPER_IO_OPERATION_DATA lpPerIOData = NULL;
    CTCPSocket			   &sListenSocket = m_This->m_sListenSocketMap[dwCurNetCard];
    sockaddr_in		        InternetAddr;
    DWORD					dwIp = 0;
    string					strIp = ("");
    int						len = 0;
    
    while (!m_This->m_bListenExit)
    {
        CTCPSocket sAccept;

        len = sizeof(InternetAddr);
            
        sListenSocket.Accept(sAccept, (sockaddr*)&InternetAddr,len);

        strIp=inet_ntoa(InternetAddr.sin_addr);

        dwIp = inet_addr(strIp);

        // °ÑÐÂÁ¬œÓµÄSocket·ÅÈëÍê³É¶Ë¿Ú¶ÔÁÐ // ÔÝÊ±²ÉÈ¡¿œ±ŽµÄÐÎÊœ
        lpPerIOData = m_This->AssociateSocketWithCompletionPort(sAccept);
        
        if (NULL == lpPerIOData)
        {
            MASTRACE((": ¹ØÁªÍê³É¶Ë¿ÚÊ±·¢ÉúŽíÎó"));
            
            continue;
        }

        m_This->m_IPMapLock.Lock();
        CIp_SocketNumMap::iterator it = m_This->m_IPMap.find(dwIp);
        m_This->m_IPMap[dwIp] ++;
        m_This->m_IPMapLock.Unlock();

#if _ISSOCKELOCK_
        AddSocketLock(sAccept.GetSocketHandle());
#endif	

        // »Øµ÷œÓ¿Úº¯Êý
        if (m_This->m_SocketCreateCB != NULL)
        {
            CTCPSocket tempSocket;
            tempSocket = (SOCKET)sAccept.GetSocketHandle();
            tempSocket.m_IsRev = TRUE;
            m_This->m_SocketCreateCB(&tempSocket, dwIp, m_This->m_CreateUserDATA);
        }

        sAccept.Read(lpPerIOData);
        
    }
    */

    return 0;
}

BOOL CTcpClient::StartWork()
{
    if (!CreateCompletionThread())
    {
        MASTRACE(("[fail]Íê³É¶Ë¿ÚÏß³ÌŽŠÀíÊ§°Ü£¡"));

        return FALSE;
    }

    return TRUE;    
}

BOOL CTcpClient::CreateCompletionThread()
{
    int cpuNum = m_CPUNumber;

    MASTRACE("Create IOCP Thread NUM[%d]\n", cpuNum);

    pthread_t tmp_thread_id = 0;
    char pipeName[64] = {0};
    WAITHANDLE hEvent = NULL;

    while (cpuNum --)
    {
        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        ASSERT(hEvent);

        if (0 != pthread_create(&tmp_thread_id, NULL, _TCPCompletionThread, (void *)hEvent))
        {
            MASTRACE("[FAIL] pthread_create FAIL, ERROR[%d]\n", errno);
            return FALSE;
        }

        memset(pipeName, 0, 64);

        sprintf(pipeName, "/tmp/Fi_maspipe%x_%x",(int)getpid(), (int)tmp_thread_id);

        if (mkfifo(pipeName, 0777) != 0 && errno != EEXIST)
        {
            MASTRACE("[FAIL]mkfifo[%s] FAIL, ERROR[%d]\n", pipeName, errno);
            return FALSE;
        }

        int fd = open(pipeName, O_WRONLY, 0777);

        if (fd == -1)
        {
            MASTRACE("[FAIL]open[%s] FAIL for O_WRONLY, ERROR[%d]\n", pipeName, errno);
            return FALSE;
        }

        m_SelectModeThreadActor.insert(map<pthread_t, DWORD>::value_type(tmp_thread_id, fd));
        m_SelectModeThreadHold.insert(map<pthread_t, DWORD>::value_type(tmp_thread_id, 0));
        m_SelectModeThreadEvent.insert(map<pthread_t, WAITHANDLE>::value_type(tmp_thread_id, hEvent));
    }

    return TRUE;
}


BOOL CTcpClient::CreateConnectThread()
{
    pthread_t tmp_thread_id;

    if (0 != create_thread(&tmp_thread_id, NULL, _TCPAutoConnect, (void *)this))
    {
        MASTRACE("[FAIL] pthread_create FAIL, ERROR[%d]\n", errno);
        return FALSE;
    }

    return TRUE;
}

void * __stdcall CTcpClient::_TCPCompletionThread(LPVOID lpParam)
{
    CTcpClient *pTCPServerThread = m_This;

    MASTRACE(("_TCPCompletionThread::_TCPCompletionThread(0x%x) was created."), (int)GetCurrentThreadId());

    PER_HANDLE_DATA PerHandleData;
    LPPER_IO_OPERATION_DATA lpPerIOConText = NULL;
    pthread_t p_thread_id = pthread_self();
    char pipe_name[64] = {0};
    char pipe_msg[64] = {0};

    sprintf(pipe_name, "/tmp/Fi_maspipe%x_%x", (int)getpid(),(int)p_thread_id);

    if (mkfifo(pipe_name, 0777) != 0 && errno != EEXIST)
    {
        assert(0);
        return NULL;
    }

    int fd_pipe = open(pipe_name, O_RDONLY, 0777);

    if (fd_pipe == -1)
    {
        ASSERT(0);
        //exit(-1);
    }

    fd_set readfds;
    int fd_sock[FD_SETSIZE];
    int n_index = 0;
    int n_last_zero_index = 0;
    int n_hold_sock_size = 0;
    int n_ret = 0;
    int n_max_sock = 0;
    int n_max_sock_index = 0;
    int pipe_return = 0;
    bool b_has_add = false;
    bool bNeedClose = true;

    while (TRUE)
    {
        n_max_sock = fd_pipe;
        n_max_sock_index = -1;

        FD_ZERO(&readfds);
        FD_SET(fd_pipe, &readfds);

        for (n_index = 0; n_index < n_hold_sock_size; n_index++)
        {
            if (fd_sock[n_index] != 0)
            {
                FD_SET(fd_sock[n_index], &readfds);

                if (n_max_sock < fd_sock[n_index])
                {
                    n_max_sock = fd_sock[n_index]; 
                    n_max_sock_index = n_index;
                }
            }
        }

        n_ret = select(n_max_sock+1, &readfds, NULL, NULL, 0);

        //printf("[mas_tcp_%x]Select Return<%d>.\n", (unsigned int)p_thread_id, n_ret);

        if (n_ret < 0)
        {
            if (EINTR == errno)
            {
                //printf(("[mas_tcp_%x] Select is Interrupted by system call \n"), (unsigned int)p_thread_id);
                continue;
            }
            else if (EBADF == errno)
            {
                MASTRACE(("[mas_tcp_%x] Bad file num[%d] "), (unsigned int)p_thread_id, n_max_sock);

                if (n_max_sock_index >= 0)
                {
                    fd_sock[n_max_sock_index] = 0;
                }
                struct stat st;
                for (n_index = 0; n_index < n_hold_sock_size; n_index++)
                {
                    if (fd_sock[n_index] != fd_pipe)
                    {
                        int tret = fstat(fd_sock[n_index],&st);
                        if (tret == -1 && errno == EBADF)
                        {
                            pTCPServerThread->m_IODataMapLock.Lock();
                            PerHandleData.Socket = fd_sock[n_index];
                            pTCPServerThread->CloseClient(&PerHandleData, lpPerIOConText);
                            pTCPServerThread->m_IODataMapLock.Unlock();
                            FD_CLR(fd_sock[n_index], &readfds);
                            fd_sock[n_index] = 0;

                            break;
                        }
                    }
                }
                continue;
            }
            else
            {
                int nErr = errno;
                assert(0);
                MASTRACE("[mas_tcp] select error<%d>.", errno);
                break;
            }
        }

        // ³¬Ê±, ŽËŽŠÓÀÔ¶²»»á·¢Éú.
        if (n_ret == 0)
        {
            MASTRACE(("[mas_tcp_%x] ________n_ret == 0___________"), (unsigned int)p_thread_id);
            continue;
        }

        for (n_index = 0; n_index < n_hold_sock_size; n_index++)
        {
            if (FD_ISSET(fd_sock[n_index], &readfds))
            {
                //printf("[mas_tcp_%x] sock[%d] singled \n", (unsigned int)p_thread_id, fd_sock[n_index]);
                PerHandleData.Socket = fd_sock[n_index];
                lpPerIOConText = NULL;
                //bNeedClose = TRUE;

                pTCPServerThread->m_IODataMapLock.Lock();

                if (pTCPServerThread->m_IODataMap.find(fd_sock[n_index]) != pTCPServerThread->m_IODataMap.end())
                {
                    lpPerIOConText = pTCPServerThread->m_IODataMap[fd_sock[n_index]];

                    //if (lpPerIOConText != NULL)
                    //	bNeedClose = FALSE;
                }
                else
                //if (bNeedClose)
                {
                    //printf("[mas_tcp_%x]Find Invalid Socket<%d>, Clean now.\n", (unsigned int)p_thread_id,fd_sock[n_index]);

                    pTCPServerThread->CloseClient(&PerHandleData, lpPerIOConText);

                    FD_CLR(fd_sock[n_index], &readfds);

                    fd_sock[n_index] = 0;
                    pTCPServerThread->m_IODataMapLock.Unlock();

                    continue;
                }

                pTCPServerThread->m_IODataMapLock.Unlock();

                n_ret = PerHandleData.Socket.Read(lpPerIOConText);

//  			if(n_ret == 0)
//  			{
//  				for (n_index = 0; n_index < n_hold_sock_size; n_index++)
//  				{
//  					printf("\n [mas_tcp_dump_%x] socket[%d]",(unsigned int)p_thread_id,fd_sock[n_index]);
//  				}
//
//  				printf(("\n [mas_tcp_%x]__n_ret == 0_fd[%d] pos[%d] errno[%d]\n"),(unsigned int)p_thread_id,fd_sock[n_index],n_index,errno);
//  				continue;
//  			}
//
                if (n_ret <= 0)
                {
                    pTCPServerThread->CloseClient(&PerHandleData, lpPerIOConText);

                    //printf(("\n[mas_tcp_%x] n_ret < 0_fd[%d] pos[%d]errno[%d]\n"),
                    //	   (unsigned int)p_thread_id,fd_sock[n_index], n_index,errno);

                    FD_CLR(fd_sock[n_index], &readfds);

                    fd_sock[n_index] = 0;

                    continue;
                }

                lpPerIOConText->pMsgBuf->SetDataRealSize(n_ret);

                if (pTCPServerThread->m_SocketMsgCB != NULL)
                {
                    //printf("[mas_tcp_%x] Get Data From SOCK<%d>.\n", (unsigned int)p_thread_id,fd_sock[n_index]);
                    CTCPSocket temSocket;
                    temSocket = PerHandleData.Socket;
                    (pTCPServerThread->m_SocketMsgCB)(lpPerIOConText->pMsgBuf, &temSocket, pTCPServerThread->m_MsgUserDATA); //ŽŠÀíÏûÏ¢»Øµ÷
                }
            }
        }

        if (FD_ISSET(fd_pipe, &readfds))
        {
            memset(pipe_msg, 0, 64);

            if (read(fd_pipe, pipe_msg, sizeof(pipe_msg)) == -1)
            {
                MASTRACE("read pipe<%s> error<%d>.\n", pipe_name, errno);

                continue;
            }

            sprintf((char *)&pipe_return, "%s", (char *)pipe_msg);
            if (pipe_return == 0)
            {
                MASTRACE("[mas_tcp] warning IOCP read pipe msg 0 ");
            }
            if (pipe_return == RPC_EXIT_CODE)
            {
                MASTRACE("[mas_tcp_%x] IOCP Thread Safe Exit!",(unsigned int)p_thread_id);
                assert(0);
                break;
            }

            b_has_add = false;
            n_last_zero_index = -1;

            for (n_index = 0; n_index < n_hold_sock_size; n_index++)
            {
                if (fd_sock[n_index] == pipe_return)
                {
                    assert(0);
                    break;
                }

                if (n_last_zero_index == -1 && fd_sock[n_index] == 0)
                {
                    n_last_zero_index = n_index;
                }
            }

            // ÐÂŒÓSOCKET
            if (n_last_zero_index != -1 && fd_sock[n_last_zero_index] == 0)
            {
                fd_sock[n_last_zero_index] = pipe_return;

                //printf("\n [mas_tcp_%x] has add pipe socket[%d] pos[%d]\n",(unsigned int)p_thread_id,pipe_return,n_index);

                b_has_add = true;
            }

            if (!b_has_add && n_hold_sock_size < FD_SETSIZE)
            {
                //printf("\n [mas_tcp_%x] no add pipe socket[%d] pos[%d]\n",(unsigned int)p_thread_id,pipe_return,n_hold_sock_size);
                fd_sock[n_hold_sock_size] = pipe_return;
                n_hold_sock_size++;
            }
            else if (n_hold_sock_size == FD_SETSIZE)
            {
                // ÐÂÆô¹€×÷Ïß³Ì, ÔÝ²»ÊµÏÖ.
            }
        }
    } 

    // Close pipe handle
    close(fd_pipe);

    // remove pipe file
    if (0 != remove(pipe_name))
    {
        MASTRACE("[mas_tcp] IOCP Delete File<%s> Fail, ERRNO[%d].", pipe_name, errno);
    }
    else
    {
        MASTRACE("[mas_tcp] IOCP Delete File<%s> OK.", pipe_name);
    }

    SetEvent((WAITHANDLE)lpParam);
    pthread_detach(pthread_self());
    return 0;
}

void CTcpClient::CloseClient( LPPER_HANDLE_DATA lpPerHandleData, LPPER_IO_OPERATION_DATA lpPerIOConText )
{
    CTCPSocket tempSocket;

    if (lpPerHandleData)
    {
        //	lock
        m_IODataMapLock.Lock();
        tempSocket = (SOCKET)(lpPerHandleData->Socket.GetSocketHandle()); 

        if (lpPerIOConText != NULL && NULL == lpPerIOConText->IoData.Socket.GetSocketHandle())
        {
            MASTRACE(("Free lpPerHandleData<0x%p>, sokt removed outside."), (void **)lpPerHandleData);
	    RemoveSocketLock((HANDLE)tempSocket.GetSocketHandle());
            DeleteIOData(lpPerIOConText);
        }
        else
        {
            CSocket_IODATA::iterator it = m_IODataMap.find((int)lpPerHandleData->Socket.GetSocketHandle());

            if (it != m_IODataMap.end())
            {
                if (NULL != m_SocketExitCB)
                    (m_SocketExitCB)(&tempSocket, m_ExitUserDATA); //¹Ø±Õ»Øµ÷

                AddAutoConn(it->second->IoData.Socket,FALSE);
                DeleteIOData(it->second);
                m_IODataMap.erase(it++);

                RemoveSocketLock((HANDLE)tempSocket.GetSocketHandle());
                lpPerHandleData->Socket.Close();
                MASTRACE(("³É¹ŠÉŸ³ýLPPER_HANDLE_DATA socket[%d]"), (SOCKET)tempSocket.GetSocketHandle());
            }
            else
            {

                MASTRACE(("[warning]Ã»ÓÐÕÒµœÐèÒªÉŸ³ýµÄÊýŸÝ socket¡Ÿ%d¡¿¿ÉÄÜ¶ÔÓŠµÄÖžÕëŒžŸ­±»ÊÍ·Å"), (SOCKET)lpPerHandleData->Socket.GetSocketHandle());
                //MAS_ASSERT(0);
                lpPerHandleData->Socket.Close();
            }

            pthread_t selfThread = pthread_self();

            m_ThreadHoldSizeLock.Lock();

            if (m_SelectModeThreadHold.find(selfThread) != m_SelectModeThreadHold.end())
            {
                ASSERT(0 < m_SelectModeThreadHold[selfThread]);
                m_SelectModeThreadHold[selfThread] = m_SelectModeThreadHold[selfThread] - 1;
            }

            m_ThreadHoldSizeLock.Unlock();
        }

        m_IODataMapLock.Unlock();

        lpPerHandleData = NULL;
    }

    return;    
}

void CTcpClient::AddAutoConn(CTCPSocket &ws , BOOL bConn)
{
    BOOL bFind = FALSE;

    m_lkconn.Lock();

    for (int i = 0 ; i < m_vecconn.size(); i++)
    {
        if (m_vecconn[i].dwRemoteIP == ws.m_dwRemoteIP && 
            m_vecconn[i].dwLocalIP  == ws.m_dwLocalIP  &&
            m_vecconn[i].uRemotePort == ws.m_uRemotePort)
        {
            m_vecconn[i].bConned = bConn;
            bFind = TRUE;
        }
    }

    if (!bFind)
    {
        connect_rec rec(ws.m_dwLocalIP, ws.m_dwRemoteIP, ws.m_uRemotePort, bConn);
        m_vecconn.push_back(rec);
    }

    m_lkconn.Unlock();
}

void CTcpClient::CloseClientSafeOutOfIOCP(HANDLE sokt)
{
    LPPER_IO_OPERATION_DATA lpPerIoData = NULL;
    CSocket_IODATA::iterator it;
    CTCPSocket tempSocket;
    tempSocket = (SOCKET)sokt;

    if (sokt)
    {
        m_IODataMapLock.Lock();
        it = m_IODataMap.find((SOCKET)sokt);
        if (it != m_IODataMap.end())
        {
            AddAutoConn(it->second->IoData.Socket,FALSE);

            lpPerIoData = m_IODataMap[(SOCKET)sokt];
            lpPerIoData->IoData.Socket.ZeroSocketHandle();
            m_IODataMap.erase((SOCKET)sokt);
            if (NULL != m_SocketExitCB)
                (m_SocketExitCB)(&tempSocket, m_ExitUserDATA);


            tempSocket.Close();
        }
        else
        {
            MASTRACE("[warning] closeclientsafeoutofiocp can not find socket[%d]",(SOCKET)sokt);
        }

        m_IODataMapLock.Unlock();
    }

    return;
}

void CTcpClient::CloseAllSocket()
{
    CTCPSocket tempSocket;
    LPPER_IO_OPERATION_DATA lpPerIoData = NULL;

    m_IODataMapLock.Lock();
    while (m_IODataMap.size())
    {
        CSocket_IODATA::iterator it = m_IODataMap.begin();
        if (it == m_IODataMap.end())
        {
            break;
        }

        AddAutoConn(it->second->IoData.Socket,FALSE);

        tempSocket = it->first;
        lpPerIoData = it->second;
        lpPerIoData->IoData.Socket.ZeroSocketHandle();
        m_IODataMap.erase(it++);

        if (NULL != m_SocketExitCB)
        {
            (m_SocketExitCB)(&tempSocket, m_ExitUserDATA); //关闭回调
        }

        try
        {
            tempSocket.Close();
        }
        catch (...)
        {
            MASTRACE(("[ERROR] Close SOCKET[%d] exception"), (SOCKET)tempSocket.GetSocketHandle());
        }
    }

    m_IODataMapLock.Unlock();
}

BOOL CTcpClient::CreateListenThread()
{
    m_cListenSocket.Create();
    m_cListenSocket.Listen(m_dwBindIP, m_nPort, MAX_CLIENT);    // ×îŽóÁ¬œÓÊýMAX_CLIENT

    pthread_t hListenThread = 0;

    if (0 != pthread_create(&hListenThread, NULL, _TCPListenThread, (void *)this))
    {
        m_cListenSocket.Cleanup();

        MASTRACE(("pthread_create _TCPListenThread fail."));

        return FALSE;
    }

    return TRUE;
}

void * __stdcall  CTcpClient::_TCPAutoConnect(LPVOID lpParam)
{
    vector<connect_rec> vecTmp;
    CTcpClient* pObj = (CTcpClient*)lpParam;
    int i = 0;

    while (TRUE)
    {
       pObj->m_EvConn.Wait(); 
       
       while(TRUE)
       {
	   vecTmp.clear();
           pObj->m_lkconn.Lock();
           for(i = 0; i < pObj->m_vecconn.size(); i++)
           {
               if(!pObj->m_vecconn[i].bConned)
               {
		   vecTmp.push_back(pObj->m_vecconn[i]);
               }
           }
           pObj->m_lkconn.Unlock();

	   for(i = 0; i < vecTmp.size(); i++)
	   {
	       pObj->ConnSvr(vecTmp[i].dwLocalIP,vecTmp[i].dwRemoteIP, vecTmp[i].uRemotePort);
	   }
           Sleep(5000);
       }
    }
}

void * __stdcall CTcpClient::_TCPListenThread(LPVOID lpParam)
{
    CTCPSocket sAccept;  // ÓëÔ¶¶ËÍšÑ¶µÄÌ×œÓ×Ö

    LPPER_IO_OPERATION_DATA lpPerIOData = NULL;

    DWORD dwRecvBytes = 0;

    CTcpClient *pTcpServerThread = (CTcpClient *)lpParam;

    while (!pTcpServerThread->m_bListenExit)    // ŽËŽŠÒªŒÓœáÊøÌõŒþ
    {
        lpPerIOData = pTcpServerThread->AcceptConnection(sAccept);
        if (NULL == lpPerIOData)
        {
            printf("mas AcceptConnection is null \n");
            continue;   // Á¬œÓÊ§°Ü
        }


        dwRecvBytes = sAccept.Read(lpPerIOData);
    }

    return 0;
}

LPPER_IO_OPERATION_DATA CTcpClient::AcceptConnection(CTCPSocket &sConn)
{
    LPPER_IO_OPERATION_DATA lpPerIOData = NULL;
    sockaddr_in InternetAddr;
    DWORD dwIp = 0;
    int len = 0;

    memset(&InternetAddr, 0, sizeof(sockaddr_in));

    len = sizeof(InternetAddr);
    m_cListenSocket.Accept(sConn, (sockaddr*)&InternetAddr,len);

    dwIp = InternetAddr.sin_addr.s_addr;

    if (sConn.GetSocketHandle() == 0)
    {
        MASTRACE(("socket is null continue"));
        return NULL;
    }
    // °ÑÐÂÁ¬œÓµÄSocket·ÅÈëÍê³É¶Ë¿Ú¶ÔÁÐ // ÔÝÊ±²ÉÈ¡¿œ±ŽµÄÐÎÊœ
    lpPerIOData = AssociateSocketWithCompletionPort(sConn);
    if (NULL == lpPerIOData)
    {
        MASTRACE((": ¹ØÁªÍê³É¶Ë¿ÚÊ±·¢ÉúŽíÎó"));
        return NULL;
    }

    m_IPMapLock.Lock();
    m_IPMap[dwIp] ++;
    m_IPMapLock.Unlock();

#if _ISSOCKELOCK_
    AddSocketLock(sConn.GetSocketHandle());
#endif	

    // »Øµ÷œÓ¿Úº¯Êý
    if (m_SocketCreateCB != NULL)
    {
        CTCPSocket tempSocket;
        tempSocket = (SOCKET)sConn.GetSocketHandle();
        tempSocket.m_IsRev = TRUE;
        m_SocketCreateCB(&tempSocket, dwIp, m_CreateUserDATA);
    }

    return lpPerIOData;
}

LPPER_IO_OPERATION_DATA CTcpClient::AssociateSocketWithCompletionPort(CTCPSocket tcpSocket)
{
    LPPER_IO_OPERATION_DATA lpPerIOData = NULL;

    lpPerIOData = CreateInfo(tcpSocket);

    if (NULL == lpPerIOData)
    {
        MASTRACE(("[fail] ÄÚŽæ·ÖÅäŽíÎó!"));
        return NULL;
    }

    map<pthread_t, int>::iterator it;
    map<pthread_t, DWORD>::iterator holdIt;
    SOCKET curSock = (SOCKET)tcpSocket.GetSocketHandle();

    m_ThreadHoldSizeLock.Lock();

    holdIt = m_SelectModeThreadHold.begin();

    DWORD dwFisrtHold = holdIt->second;

    // Ã¿ŽÎ¶Œ·ÅÔÚ¹ÜÀí×îÉÙSOCKETµÄÏß³Ì
    MASTRACE("[mas_conn] connect new  socket[%d]",curSock);
    for (it = m_SelectModeThreadActor.begin(); it != m_SelectModeThreadActor.end(); it++)
    {
        if (m_SelectModeThreadHold[it->first] < dwFisrtHold && m_SelectModeThreadHold[it->first] < FD_SETSIZE)
        {
            // ÓÉÓÚÊÇÍ¬Ò»žöœø³ÌÄÚ²¿œ»»¥, ÏÈ²»¿ŒÂÇÊ§°ÜÇé¿ö
            MASTRACE("[mas_conn_find] push pipe[%d] socket[%d]",it->second,curSock);
            write(it->second, &curSock, sizeof(SOCKET));

            m_SelectModeThreadHold[it->first] = m_SelectModeThreadHold[it->first] + 1;

            break;
        }
    }

    if (it == m_SelectModeThreadActor.end())
    {
        // ÓÉÓÚÊÇÍ¬Ò»žöœø³ÌÄÚ²¿œ»»¥, ÏÈ²»¿ŒÂÇÊ§°ÜÇé¿ö
        it = m_SelectModeThreadActor.begin();

        if (m_SelectModeThreadHold[it->first] <= FD_SETSIZE)
        {
            MASTRACE("[mas_conn] push pipe[%d] socket[%d]",it->second,curSock);
            write(it->second, &curSock, sizeof(SOCKET));

            m_SelectModeThreadHold[it->first] = m_SelectModeThreadHold[it->first] + 1;
        }
        else
        {
            MASTRACE("All IOCP Handle Thread is full...");
            DeleteIOData(lpPerIOData);
            m_ThreadHoldSizeLock.Unlock();
            return NULL;
        }
    }

    m_ThreadHoldSizeLock.Unlock();

    MASTRACE(("lpPerIOData<0x%p>"), (void **)lpPerIOData);

    //	lock
    m_IODataMapLock.Lock();
    m_IODataMap[(int)tcpSocket.GetSocketHandle()] = lpPerIOData;
    m_IODataMapLock.Unlock();

    return lpPerIOData;
}

LPPER_IO_OPERATION_DATA CTcpClient::CreateInfo(CTCPSocket &tcpSocket)
{
    LPPER_IO_OPERATION_DATA lpPerIOData = NULL;

    lpPerIOData = new PER_IO_OPERATION_DATA;

    if (FALSE == lpPerIOData->Initialize(tcpSocket))
    {
        return NULL;
    }

    return lpPerIOData; 
}



int CTcpClient::GetSocketNum(DWORD dwIp)
{
    int br = 0;
    m_IPMapLock.Lock();
    CIp_SocketNumMap::iterator it = m_IPMap.find(dwIp);
    if (it != m_IPMap.end())
    {
        br = m_IPMap[dwIp];
    }
    m_IPMapLock.Unlock();

    return br;
}

BOOL CTcpClient::SendMesg(CMemItem *temBuf, CTCPSocket *sokt, BOOL isASASynWrite)
{
    BOOL br = FALSE;
    CCritSec *pSlk = NULL;

    m_socketCritSecLock.ShareLock();

    CSocket_CritSecMap::iterator it = m_socketCritSec.find((HANDLE)sokt);
    if (it != m_socketCritSec.end())
    {
        pSlk = it->second;
        if (NULL == pSlk)
        {
            TCPSTRANCE(("[fail]can not find socket lock发送消息失败！[%s]"), sokt->GetError().c_str());
            goto LABEL;
        }
    }
    else
    {
        TCPSTRANCE(("[fail]can not find socket lock发送消息失败！[%s]"), sokt->GetError().c_str());
        goto LABEL;
    }

    pSlk->Lock();

    if (sokt->SendMsg(temBuf, isASASynWrite))                //	发送
    {
        TCPSTRANCE(("[发送]SOCKET[%d]发送消息成功"), (SOCKET)sokt);
        br = TRUE;
    }
    else
    {
        TCPSTRANCE(("[fail]发送消息失败！[%s]"), sokt->GetError().c_str());
        br = FALSE;
    }   

    pSlk->Unlock();

LABEL:
    m_socketCritSecLock.LeaveLock();

    return br;
}

BOOL CTcpClient::SendMsgBlks(CMemItem *temBuf, PVOID pDataBuf,DWORD dwBufSize,CTCPSocket *sokt, DWORD &dwStatus,BOOL isASASynWrite)
{
    BOOL br = FALSE;
    CCritSec *pSlk = NULL;

    m_socketCritSecLock.ShareLock();

    CSocket_CritSecMap::iterator it = m_socketCritSec.find((HANDLE)sokt);
    if (it != m_socketCritSec.end())
    {
        pSlk = it->second;
        if (NULL == pSlk)
        {
            TCPSTRANCE(("[fail]can not find socket lock发送消息失败！[%s]"), sokt->GetError().c_str());
            goto LABEL;
        }
    }
    else
    {
        TCPSTRANCE(("[fail]can not find socket lock发送消息失败！[%s]"), sokt->GetError().c_str());
        goto LABEL;
    }

    pSlk->Lock();
    if (sokt->SendMsgBlks(temBuf, pDataBuf,dwBufSize,dwStatus,isASASynWrite))                //	���� fo 
    {
        MASTRACE(("[ok]SOCKET[%d]SendMsgBlks"), (SOCKET)sokt);
        br = TRUE;
    }
    else
    {
        MASTRACE(("[fail]SendMsgBlks[%s]"), sokt->GetError().c_str());
        br = FALSE;
    }   

    pSlk->Unlock();

LABEL:
    m_socketCritSecLock.LeaveLock();

    return br;
}


void CTcpClient::DeleteIOData( LPPER_IO_OPERATION_DATA lpPerIOConText )
{
    if (lpPerIOConText != NULL)
    {
        if (lpPerIOConText->pMsgBuf != NULL)
        {
            delete lpPerIOConText->pMsgBuf;
            lpPerIOConText->pMsgBuf = NULL;
        }

        _SafeDeletePtr(lpPerIOConText);
    }
}


CCritSec* CTcpClient::AddSocketLock(HANDLE s)
{
    CCritSec* critsec = NULL;
    m_socketCritSecLock.OwnerLock();
    CSocket_CritSecMap::iterator it = m_socketCritSec.find(s);
    if (it == m_socketCritSec.end())
    {
      critsec = new CCritSec;
      m_socketCritSec[(HANDLE)s] = critsec;
      STRACE(("AddSocketLock Socket[%d]  CritSec[%d]"), s, critsec);

    }
    else
    {
      STRACE(("AddSocketLock exist Socket"));
      ASSERT(0);
    }

    m_socketCritSecLock.LeaveLock();

    return critsec;
}

CCritSec* CTcpClient::GetSocketLock(HANDLE s)
{
    CCritSec* ret = NULL;
    m_socketCritSecLock.ShareLock();

    CSocket_CritSecMap::iterator it = m_socketCritSec.find(s);

    if (it != m_socketCritSec.end())
    {
    	ret = it->second;
    }

    m_socketCritSecLock.LeaveLock();
}

void  CTcpClient::RemoveSocketLock(HANDLE s)
{
    CCritSec* lk = NULL;

    m_socketCritSecLock.OwnerLock();

    CSocket_CritSecMap::iterator it = m_socketCritSec.find(s);
    if (it != m_socketCritSec.end())
    {
    	lk = it->second;
     	
    	// if other thread just locked ,then must wait other thread unlocked when delete
    	lk->Lock();
    	lk->Unlock();

    	_SafeDeletePtr( lk);
    	m_socketCritSec.erase(it);
    }
    else
    {
    	STRACE(("Socket lock can not find"));
    }

    m_socketCritSecLock.LeaveLock();
}

int CTcpClient::RemoveSocektNum( DWORD dwIp )
{
    int br = 0;
    m_IPMapLock.Lock();
    CIp_SocketNumMap::iterator it = m_IPMap.find(dwIp);
    if (it != m_IPMap.end())
    {
        m_IPMap.erase(dwIp);
    }
    m_IPMapLock.Unlock();

    return br;

}

HANDLE CTcpClient::ConnSvr(DWORD dwLocalIp, DWORD RemoteIp, USHORT nPort, BOOL bIsCtrlSvr)
{
    CTCPSocket sConnecter;
    struct sockaddr_in server;
    DWORD rePort;
    LPPER_IO_OPERATION_DATA lpPerIOData = NULL;

    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(nPort);
    server.sin_addr.s_addr = RemoteIp;

    sConnecter.RecConnInfo(dwLocalIp, RemoteIp, nPort);
    sConnecter.Create();

    if(0 != dwLocalIp)
    {
        sConnecter.Bind(dwLocalIp,rePort);
    }

    BOOL bSuccess = sConnecter.Connect((LPCSOCKADDR)&server);

    if (!bSuccess)
    {
        sConnecter.Cleanup();
	AddAutoConn(sConnecter,FALSE);
        return NULL;
    }
    AddAutoConn(sConnecter,TRUE);

    CAutoLock lck(&m_TCPSerCritSec);

    lpPerIOData = AssociateSocketWithCompletionPort(sConnecter);

    if (FALSE == lpPerIOData)
    {
        MASTRACE(("[fail] AssociateSocketWithCompletionPort(%d) ERRORCode[%d]!"), 
                 (int)(long)(HANDLE)sConnecter, (int)errno);

        sConnecter.Close();

        return NULL;
    }
    else
    {
        sConnecter.Read(lpPerIOData);
    }

    m_IPMapLock.Lock();

    CIp_SocketNumMap::iterator it = m_IPMap.find(RemoteIp);
    m_IPMap[RemoteIp] ++;

    m_IPMapLock.Unlock();

#if _ISSOCKELOCK_
    AddSocketLock(sConnecter.GetSocketHandle());
#endif

    if (m_SocketCreateCB != NULL)
    {

        CTCPSocket tempSocket;
        tempSocket = (SOCKET)sConnecter.GetSocketHandle();
        tempSocket.m_dwLocalIP= dwLocalIp;
        tempSocket.m_dwRemoteIP = RemoteIp;
        tempSocket.m_uRemotePort = nPort;
        m_SocketCreateCB(&tempSocket, RemoteIp, m_CreateUserDATA);
    }
    else
    {
        MASTRACE(("[fail]ÐÂœšÁ¢socket»Øµ÷ ²»ŽæÔÚ"));
    }

    return sConnecter.GetSocketHandle();
}
