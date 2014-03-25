#include <fcntl.h>
#include "tcpserver.h"
#ifndef FI_LINUX
#include <process.h>
#endif

#include "OSUtility.h"

#define MAXEPOLLSIZE 512
#define SVR_REGHOST_OK 10
#define SVR_REGTHRED_NUM 2

#define STRACE

CTcpServer::CTcpServer()
{
    m_state = TCP_SVR;
    m_bListenExit    = false;
    m_Port           = 0;
    m_SocketCreateCB = NULL;
    m_SocketExitCB   = NULL;
    m_SocketMsgCB    = NULL;
    m_bStarted = false;

#if (defined(_WIN32) || defined(_WIN64))
    m_CPUNumber = GetCPUNumber();
#endif
    if (m_CPUNumber == 0)
    {
        m_CPUNumber = 1;
    }

    ASSERT(m_CPUNumber);
    m_CPUNumber *= 2;

    m_bAutoConn = false;
    m_enable = 0;
    SOCKETMSG_FUNC func = _ModuleRegCB;

    m_Mod = new CModuleInfo(1, NULL, _ModuleRegCB, NULL);
    RegisterModule(m_Mod);

    CreateVerifyThd();

    for (int i = 0; i < SVR_REGTHRED_NUM; i++)
    {
        CreateRegThd();
    }
}

CTcpServer::CTcpServer(ULONG port)
{
    m_state = TCP_SVR;
    m_bListenExit    = false;
    m_Port           = port;
    m_SocketCreateCB = NULL;
    m_SocketExitCB   = NULL;
    m_SocketMsgCB    = NULL;
    m_bStarted = false;

#if (defined(_WIN32) || defined(_WIN64))
    m_CPUNumber = GetCPUNumber();
#endif

    ASSERT(m_CPUNumber);
    m_CPUNumber *= 2;
    m_bAutoConn = false;
}

CTcpServer::~CTcpServer()
{
    m_bListenExit = true;

    Sleep(1);

    if (m_ListenThread)
    {
#if (defined(_WIN32) || defined(_WIN64))
        CloseHandle(m_ListenThread);
#else
        pthread_cancel(m_ListenThread);
#endif
    }
}

BOOL CTcpServer::Init(SOCKETCREATE_FUNC createCB, void *createUserData,
                      SOCKETEXIT_FUNC exitCB, void *exitUserData,
                      SOCKETMSG_FUNC msgCB, void *msgUserData, net_mode mode)
{

    map<DWORD, CNetCard>::iterator it;
    m_SocketCreateCB = createCB;
    m_SocketExitCB   = exitCB;
    m_SocketMsgCB    = msgCB;
    m_CreateUserDATA = createUserData;
    m_ExitUserDATA   = exitUserData;
    m_MsgUserDATA    = msgUserData;

    if (m_bStarted)
    {
        printf("started twice !!!!!!!!!!!!!!!!!\n");
        ASSERT(0);
    }

    m_pmsg_site = new CMsgSite(msgCB);
    ASSERT(m_pmsg_site);

    if (!m_cfg.ReadXml())
    {
        printf("read xml failed \n");
        STRACE("createcompletionthread failed");
        return false;
    }

    m_identify =  m_cfg.m_LocalIp.m_identify;
    ASSERT(m_identify);

    if (!CreateCompletionThread())
    {
        STRACE("createcompletionthread failed");
        return false;
    }

    m_state = mode;

    CProcId pid =  m_cfg.m_LocalIp.m_mapProcId.begin()->second;
    if (m_state == TCP_SVR_CLT)
    {
        STRACE("init is server and client mode");

        for (it = pid.m_mapIp.begin(); it != pid.m_mapIp.end(); it++)
        {
            if (!CreateListenThread(it->second.GetIp()))
            {
                printf("local ip [%s] CreateListenThread failed\n", (ConvertIPtoString(it->second.GetIp())).c_str());
            }
        }

        STRACE("init auto connect thread");
        for (it = pid.m_mapIp.begin(); it != pid.m_mapIp.end(); it++)
        {
            map<DWORD, CHostInfo *>::iterator im;
            for (im = m_cfg.m_RemoteIp.begin(); im != m_cfg.m_RemoteIp.end(); im++)
            {
                map<DWORD, CNetCard>::iterator ik;
                CProcId rid = im->second->m_mapProcId.begin()->second;

                for (ik = rid.m_mapIp.begin(); ik != rid.m_mapIp.end(); ik++)
                {
                    CTCPSocket ts;
                    ts.m_dwLocalIP = it->first;
                    ts.m_dwRemoteIP = ik->first;
                    ts.m_uRemotePort = (USHORT)m_Port;
                    ts.m_bAutoConn = true;

                    AddAutoConn(ts, FALSE);
                }
            }
        }

        CreateConnectThread();
    }

    if (m_state == TCP_SVR)
    {
        STRACE("init is server mode");
        /// read config local bind ip list;

        for (it = pid.m_mapIp.begin(); it != pid.m_mapIp.end(); it++)
        {
            if (!CreateListenThread(it->second.GetIp()))
            {
                printf("local ip [%s] CreateListenThread failed\n", (ConvertIPtoString(it->second.GetIp())).c_str());
            }
        }
    } else
    {
        STRACE("init is client mode");
    }

    if (m_bAutoConn && m_state == TCP_CLT)
    {
        STRACE("init auto connect thread");
        for (it = pid.m_mapIp.begin(); it != pid.m_mapIp.end(); it++)
        {
            map<DWORD, CHostInfo *>::iterator im;
            for (im = m_cfg.m_RemoteIp.begin(); im != m_cfg.m_RemoteIp.end(); im++)
            {   
                CProcId rid = im->second->m_mapProcId.begin()->second;

                map<DWORD, CNetCard>::iterator ik;
                for (ik = rid.m_mapIp.begin(); ik != rid.m_mapIp.end(); ik++)
                {
                    CTCPSocket ts;
                    ts.m_dwLocalIP = it->first;
                    ts.m_dwRemoteIP = ik->first;
                    ts.m_uRemotePort = (USHORT)m_Port;
                    ts.m_bAutoConn = true;
                    AddAutoConn(ts, FALSE);
                }
            }
        }

        CreateConnectThread();
    }

    m_bStarted = true;
    return true;
}

BOOL CTcpServer::RegisterModule(u32 modID, SOCKETCREATE_FUNC _create, SOCKETMSG_FUNC _msgrecv, SOCKETEXIT_FUNC _exit)
{
    CModuleInfo *m = new CModuleInfo(modID, _create, _msgrecv, _exit);

    m_mapModlock.OwnerLock();

    if (m_mapMod.end() != m_mapMod.find(m->modID))
    {
        ASSERT(0);
    } else
    {
        m_mapMod.insert(make_pair(m->modID, m));
    }

    m_mapModlock.LeaveLock();

    return TRUE;
}

BOOL CTcpServer::RegisterModule(u32 modID, INetReceiver *pReceiver)
{
    CModuleInfo *m = new CModuleInfo(modID, pReceiver);

    m_mapModlock.OwnerLock();

    if (m_mapMod.end() != m_mapMod.find(m->modID))
    {
        ASSERT(0);
    } else
    {
        m_mapMod.insert(make_pair(m->modID, m));
    }

    m_mapModlock.LeaveLock();

    return TRUE;
}

BOOL CTcpServer::RegisterModule(CModuleInfo *mod)
{
    ASSERT(mod);
    m_mapModlock.OwnerLock();

    if (m_mapMod.end() != m_mapMod.find(mod->modID))
    {
        ASSERT(0);
    } else
    {
        m_mapMod.insert(make_pair(mod->modID, mod));
    }

    m_mapModlock.LeaveLock();
    return TRUE;
}

BOOL CTcpServer::UnRegisterModule(u32 modID)
{
    return true;
}

CModuleInfo* CTcpServer::GetModule(DWORD modid)
{
    CModuleInfo *mod = NULL;
    map<DWORD, CModuleInfo *>::iterator it;

    m_mapModlock.ShareLock();
    if (m_mapMod.end() != (it = m_mapMod.find(modid)))
    {
        mod = it->second;
    }

    m_mapModlock.LeaveLock();

    ASSERT(mod);

    return mod;
}

BOOL CTcpServer::CreateCompletionThread()
{
    int ret = 0;
#if (defined(_WIN32) || defined(_WIN64))
    m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, m_CPUNumber);
#elif defined(__linux__)
    m_fdEpoll = epoll_create(MAXEPOLLSIZE);
#else
    pthread_t tmp_thread_id = 0;
    char pipeName[64] = { 0 };
#endif

    int cpuNum = m_CPUNumber*2;

    STRACE(("rpc cpunum = %d"), cpuNum);
    OSTHREAD_H temHandle;
    while (cpuNum--)
    {
#if (defined(_WIN32) || defined(_WIN64))
        temHandle = beginthread(NULL, 0, _TCPCompletionThread, (LPVOID)this, 0, NULL);
        if (NULL == temHandle)
        {
            STRACE(_T("[fail]_beginthreadex _TCPCompletionThread error[%d]!"), GetLastError());
            return FALSE;
        }

        CloseHandle(temHandle);

#elif defined(__linux__)
        beginthread(&temHandle, NULL, _TCPCompletionThread, (LPVOID)this);
        if (0 != ret)
        {
            STRACE(("[fail]_beginthreadex _TCPCompletionThread error[%d]!"), GetLastError());
            return FALSE;
        }
#else
        memset(pipeName, 0, 64);

        sprintf(pipeName, "/tmp/Fi_pipe%x_%x", (int)getpid(), (int)tmp_thread_id);

        if (mkfifo(pipeName, 0777) != 0 && errno != EEXIST)
        {
            ASSERT(0);
            return FALSE;
        }

        int fd = -1;

        for (;;)
        {
            fd = open(pipeName, O_WRONLY, 0777);
            if (fd == -1 && errno == EINTR)
            {
                RPCSTRACE("open pipe EINTR____");
                continue;
            } else
            {
                break;
            }
        }

        if (fd == -1)
        {
            int nErr = errno;
            ASSERT(0);
            return FALSE;
        }

        m_SelectModeThreadActor.insert(map<pthread_t, DWORD>::value_type(tmp_thread_id, fd));
        m_SelectModeThreadHold.insert(map<pthread_t, DWORD>::value_type(tmp_thread_id, 0));
#endif

    }
    return TRUE;
}


OSTHD_FUNC_RET __stdcall CTcpServer::_TCPCompletionThread(LPVOID lpParam)
{
    ASSERT(lpParam);

    CTcpServer *pSvr = (CTcpServer *)lpParam;

    BOOL bSuccess = FALSE;
    DWORD dwNumBytes;
    PER_HANDLE_DATA PerHandleData;
    LPPER_HANDLE_DATA lpPerHandleData = NULL;
    LPPER_IO_OPERATION_DATA lpPerIOConText;
    FiEvent sev;
    int trytimes = 0;
    MSG_CXT *pMsgCxt = NULL;
    bool bGetMsg = false;

    CTCPSocket temSocket;

#if defined(__linux__)
    struct epoll_event ev;

    struct epoll_event events[MAXEPOLLSIZE];
    int nfd = 0;
    int n_ret = 0;
#elif ((defined(__APPLE__) && defined(__GNUC__)) || defined(macintosh) || defined(__MACOSX__))
    pthread_t p_thread_id = pthread_self();
    char pipe_name[64] = { 0 };
    char pipe_msg[64] = { 0 };

    sprintf(pipe_name, "/tmp/Fi_pipe%x_%x", (int)getpid(), (int)p_thread_id);

    if (mkfifo(pipe_name, 0777) != 0 && errno != EEXIST)
    {
        ASSERT(0);
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
    int n_hold_sock_size = 0;
    int n_last_zero_index = 0;
    int n_ret = 0;
    int n_max_sock = 0;
    int n_max_sock_index = 0;
    int pipe_return = 0;
    bool b_has_add = false;

#endif

    while (TRUE)
    {

#if (defined(_WIN32) || defined(_WIN64))
        bSuccess = GetQueuedCompletionStatus((HANDLE)pSvr->GetCompletionPort(),
                                             &dwNumBytes, (LPDWORD) & lpPerHandleData, (LPOVERLAPPED *)&lpPerIOConText, INFINITE);

        if (FALSE == bSuccess)
        {
            DWORD dwErrTemp = GetLastError();

            if (dwNumBytes == 0)
            {
                if (NULL != lpPerHandleData)
                {

                    pSvr->CloseClient(lpPerHandleData, lpPerIOConText);

                    lpPerHandleData = NULL;
                    lpPerIOConText  = NULL;

                    continue;
                } else
                {
                    STRACE(_T("[Warnint] Find Disconnect socket[%d]"), lpPerHandleData->Socket.GetSocketHandle());
                }
            }
        }	else if (dwNumBytes == 0)
        {
            DWORD dwErrTemp = GetLastError();

            if (NULL != lpPerHandleData)
            {

                pSvr->CloseClient(lpPerHandleData, lpPerIOConText);

                lpPerHandleData = NULL;
                lpPerIOConText  = NULL;

                continue;
            } else
            {
                STRACE(_T("[Warnint] Find Disconnect socket[%d]"), lpPerHandleData->Socket.GetSocketHandle());
            }
        }

        if (0xFFFFFFFF == dwNumBytes)
        {
            STRACE(_T("IOCP exit safe!"));

            lpPerHandleData = NULL;
            lpPerIOConText  = NULL;

            break;
        }

        //	处理数据
        try
        {
            lpPerIOConText->pMsgBuf->SetDataRealSize(dwNumBytes);

            if (pSvr->m_SocketMsgCB != NULL)
            {

                temSocket = lpPerHandleData->Socket;

                pMsgCxt = pSvr->m_pmsg_site->Lock_sock(&temSocket);
                ASSERT(pMsgCxt);
                bGetMsg = pSvr->m_pmsg_site->GetMsg(pSvr->m_identify, pSvr->m_mapModlock, pSvr->m_mapMod, 
                                                    lpPerIOConText->pMsgBuf, &temSocket, pSvr->m_MsgUserDATA, pMsgCxt);

                pSvr->m_pmsg_site->Unlock_sock(pMsgCxt);

                if (!bGetMsg && NULL != lpPerHandleData)
                //bCheck = (pSvr->m_SocketMsgCB)(lpPerIOConText->pMsgBuf, temSocket, pSvr->m_MsgUserDATA); //处理消息回调

                //if (!bCheck && NULL != lpPerHandleData)
                {

                    pSvr->CloseClient(lpPerHandleData, lpPerIOConText);

                    lpPerHandleData = NULL;
                    lpPerIOConText  = NULL;

                    continue;
                }
            }

            lpPerIOConText->IoData.Socket.Read(lpPerIOConText);
        }
        catch (FiExcepction& err)
        {
            err.ShowErrMsg();
            STRACE(_T("[ERROR] Read from Socket[%d] error!"), lpPerIOConText->IoData.Socket.GetSocketHandle());

            if (NULL != lpPerHandleData)
            {
                pSvr->CloseClient(lpPerHandleData, lpPerIOConText);

                lpPerHandleData = NULL;
                lpPerIOConText  = NULL;

                continue;
            }
        }

#elif defined(__linux__)
        nfd = 0;
        nfd = epoll_wait(pSvr->m_fdEpoll, events, MAXEPOLLSIZE, -1);
        if (nfd == -1)
        {
            STRACE(("EPOLL WAIT RETURN ERROR%d"), errno);
            if (errno == EINTR)
            {
                continue;
            }

            STRACE(("epoll wait error ,errno:%d--%s(%d)"), errno, __FUNCTION__, __LINE__);
            return 0;
        }

        if (nfd == 0)
        {
            printf("epoll_wait time out\n");
            continue;
        }
        //printf("epoll_wait has msg\n");
        trytimes  = 0;
        for (int i = 0; i < nfd; ++i)
        {
            //printf("epoll_wait _________event[%x] \n", events[i].events);
            if (events[i].events & EPOLLIN)
            {
                //printf("epoll_wait epollin \n");
                PerHandleData.Socket = events[i].data.fd;
                lpPerIOConText = NULL;
TRYLABLE:
                pSvr->m_IODataMapLock.ShareLock();

                if (pSvr->m_IODataMap.find(events[i].data.fd) != pSvr->m_IODataMap.end())
                {
                    lpPerIOConText = pSvr->m_IODataMap[events[i].data.fd];
                } else
                {
                    printf("epoll_wait can not find iocontext %d,then try !!!\n", events[i].data.fd);
                    pSvr->m_IODataMapLock.LeaveLock();
                    trytimes++;

                    sev.Wait(16);

                    if (trytimes < 2)
                    {
                        goto TRYLABLE;
                    }

                    printf("epoll_wait trytimes[%d] then can not find iocontext %d ,the CloseSocket!!!\n", trytimes, events[i].data.fd);
                    pSvr->CloseClient(&PerHandleData, lpPerIOConText);

                    ev.events = EPOLLHUP | EPOLLERR | EPOLLIN | EPOLLET;
                    ev.data.fd = events[i].data.fd;

                    epoll_ctl(pSvr->m_fdEpoll, EPOLL_CTL_DEL, events[i].data.fd, &ev);

                    continue;
                }

                pSvr->m_IODataMapLock.LeaveLock();
///socket lock read
                temSocket = lpPerIOConText->IoData.Socket;

                pMsgCxt = pSvr->m_pmsg_site->Lock_sock(&temSocket);

                n_ret = lpPerIOConText->IoData.Socket.Read(lpPerIOConText);
                //printf("epoll_wait epollin  Read \n");
                if (n_ret  <= 0 &&( errno == EAGAIN))
                {
                    printf("epoll read mesg is 0,continue,err[%s] %d \n",strerror(errno), errno);
                    pSvr->m_pmsg_site->Unlock_sock(pMsgCxt);

                    continue;
                }

                if (n_ret <= 0 || pMsgCxt == NULL)
                {
                    printf("epoll read mesg is 0,continue,err[%s] %d \n",strerror(errno), errno);
                    pSvr->m_pmsg_site->Unlock_sock(pMsgCxt);

                    printf("read mesg buff len [%d] closesocket \n", n_ret);
                    pSvr->CloseClient(&lpPerIOConText->IoData, lpPerIOConText);

                    ev.events = EPOLLHUP | EPOLLERR | EPOLLIN | EPOLLET;
                    ev.data.fd = events[i].data.fd;

                    epoll_ctl(pSvr->m_fdEpoll, EPOLL_CTL_DEL, events[i].data.fd, &ev);

                    continue;
                }

                lpPerIOConText->pMsgBuf->SetDataRealSize(n_ret);

                bGetMsg = pSvr->m_pmsg_site->GetMsg(pSvr->m_identify, pSvr->m_mapModlock, pSvr->m_mapMod, 
                                                    lpPerIOConText->pMsgBuf, &temSocket, pSvr->m_MsgUserDATA, pMsgCxt);

                pSvr->m_pmsg_site->Unlock_sock(pMsgCxt);

                if (!bGetMsg)
                {
                    printf("analyze msg error ,close socket  %d \n", (int)temSocket);

                    pSvr->CloseClient(&lpPerIOConText->IoData, lpPerIOConText);

                    ev.events = EPOLLHUP | EPOLLERR | EPOLLIN | EPOLLET;
                    ev.data.fd = events[i].data.fd;

                    epoll_ctl(pSvr->m_fdEpoll, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                }
            }
        }

#elif ((defined(__APPLE__) && defined(__GNUC__)) || defined(macintosh) || defined(__MACOSX__))
        n_max_sock = fd_pipe;
        n_max_sock_index = -1;
        FD_ZERO(&readfds);
        FD_CLR(fd_pipe, &readfds);
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

        RPCSTRACE(_T("[RPC] pre select "));
        n_ret = select(n_max_sock + 1, &readfds, NULL, NULL, 0);

        //cout << "[RPC]-----------select-----------------\r"<< endl;

        if (n_ret < 0)
        {
            if (EINTR == errno)
            {
                RPCSTRACE(_T("[mas_tcp_%x] Select is Interrupted by system call "), (unsigned int)p_thread_id);
                continue;
            } else if (EBADF == errno)
            {
                RPCSTRACE(_T("[mas_tcp_%x] Bad file num[%d] "), (unsigned int)p_thread_id, n_max_sock);

                if (n_max_sock_index >= 0)
                {
                    fd_sock[n_max_sock_index] = 0;
                }
                // 2011 add
                struct stat st;
                for (n_index = 0; n_index < n_hold_sock_size; n_index++)
                {

                    if (fd_sock[n_index] != fd_pipe)
                    {
                        int tret = fstat(fd_sock[n_index], &st);
                        if (tret == -1 && errno == EBADF)
                        {
                            pSvr->m_IODataMapLock.Lock();
                            PerHandleData.Socket = fd_sock[n_index];
                            PerHandleData.Socket.m_bAutoConn = true; /// set clt auto connect flag
                            pSvr->CloseClient(&PerHandleData, lpPerIOConText);
                            pSvr->m_IODataMapLock.Unlock();
                            FD_CLR(fd_sock[n_index], &readfds);
                            fd_sock[n_index] = 0;

                            break;
                        }
                    }
                }
                continue;
            } else
            {
                RPCSTRACE("select error<%d>.\n", errno);
                int nErr = errno;
                ASSERT(0);
                break;
            }
        }

        if (n_ret == 0)
        {
            RPCSTRACE(_T("n_ret == 0 n_ret == 0n_ret == 0n_ret == 0"));
            continue;
        }

        RPCSTRACE(_T("[RPC] ___ select has msg_____ begin"));
        for (n_index = 0; n_index < n_hold_sock_size; n_index++)
        {
            if (FD_ISSET(fd_sock[n_index], &readfds))
            {
                PerHandleData.Socket = fd_sock[n_index];
                PerHandleData.Socket.m_bAutoConn = true;
                lpPerIOConText = NULL;

                RPCSTRACE(_T("[RPC]____m_IODataMapLock begin____ index[%d] socket[%d]"), n_index, fd_sock[n_index]);
                pSvr->m_IODataMapLock.Lock();

                if (pSvr->m_IODataMap.find(fd_sock[n_index]) != pSvr->m_IODataMap.end())
                {
                    lpPerIOConText = pSvr->m_IODataMap[fd_sock[n_index]];
                } else
                {
                    RPCSTRACE("[RPC] ____Find Invalid Socket<%d>, Clean now.\n", fd_sock[n_index]);

                    pSvr->CloseClient(&PerHandleData, lpPerIOConText);

                    FD_CLR(fd_sock[n_index], &readfds);

                    fd_sock[n_index] = 0;
                    RPCSTRACE(_T("[RPC] ___ select has msg_____ end continue"));

                    pSvr->m_IODataMapLock.Unlock();
                    continue;
                }

                pSvr->m_IODataMapLock.Unlock();

                RPCSTRACE(_T("[RPC]____m_IODataMapLock end____"));

                RPCSTRACE(_T("[RPC]____read begin____"));

                temSocket = PerHandleData.Socket;

                pMsgCxt = pSvr->m_pmsg_site->Lock_sock(&temSocket)

                n_ret = PerHandleData.Socket.Read(lpPerIOConText);

                RPCSTRACE(_T("[RPC]____read end____"));

                if (n_ret <= 0 || NULL == pMsgCxt)
                {
                    RPCSTRACE("[RPC] ____read <= 0 Socket<%d>, Clean now.\n", fd_sock[n_index]);
                    pSvr->m_pmsg_site->Unlock_sock(pMsgCxt);

                    pSvr->CloseClient(&PerHandleData, lpPerIOConText);

                    FD_CLR(fd_sock[n_index], &readfds);

                    fd_sock[n_index] = 0;

                    RPCSTRACE(_T("[RPC] ___ select has msg_____ end continue"));
                    continue;
                }

                RPCSTRACE(_T("[RPC]____SetDataRealSize begin____"));
                lpPerIOConText->pMsgBuf->SetDataRealSize(n_ret);
                RPCSTRACE(_T("[RPC]____SetDataRealSize end____"));

                RPCSTRACE(_T("[RPC]____callback begin____"));
                ASSERT(pSvr->m_SocketMsgCB);
                bGetMsg = pSvr->m_pmsg_site->GetMsg(pSvr->m_identify, pSvr->m_mapModlock, pSvr->m_mapMod, 
                                                    lpPerIOConText->pMsgBuf, &temSocket, pSvr->m_MsgUserDATA, pMsgCxt);

                pSvr->m_pmsg_site->Unlock_sock(pMsgCxt);

                if (!bGetMsg)
                {
                    printf("getmsg error close current socket")
                    pSvr->CloseClient(&PerHandleData, lpPerIOConText);

                    FD_CLR(fd_sock[n_index], &readfds);

                    fd_sock[n_index] = 0;
                }
                RPCSTRACE(_T("[RPC]____callback end____"));
                
            }
        }

        RPCSTRACE(_T("[RPC] ___ select has msg_____ end"));

        if (FD_ISSET(fd_pipe, &readfds))
        {
            memset(pipe_msg, 0, 64);

            if (read(fd_pipe, pipe_msg, sizeof(pipe_msg)) == -1)
            {
                RPCSTRACE("[RPC]__read pipe<%s> error<%d>.\n", pipe_name, errno);

                continue;
            }

            sprintf((char *)&pipe_return, "%s", (char *)pipe_msg);
            if (pipe_return == 0)
            {
                RPCSTRACE("[rpc_tcp] warning IOCP read pipe msg 0 ");
            }

            if (pipe_return == RPC_EXIT_CODE)
            {
                RPCSTRACE("[RPC] RPC pipe exit \n");
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

            // 脨脗艗脫SOCKET
            if (n_last_zero_index != -1 && fd_sock[n_last_zero_index] == 0)
            {
                fd_sock[n_last_zero_index] = pipe_return;

                RPCSTRACE("\n [mas_tcp_%x] has add pipe socket[%d] pos[%d]", (unsigned int)p_thread_id, pipe_return, n_index);

                b_has_add = true;
            }

            if (!b_has_add && n_hold_sock_size < FD_SETSIZE)
            {
                fd_sock[n_hold_sock_size] = pipe_return;
                n_hold_sock_size++;
            } else if (n_hold_sock_size == FD_SETSIZE)
            {
                // 脨脗脝么鹿鈧椕访徝熉趁? 脭脻虏禄脢碌脧脰.
            }
        }
#endif
    }

#if ((defined(__APPLE__) && defined(__GNUC__)) || defined(macintosh) || defined(__MACOSX__))

    // Close pipe handle
    close(fd_pipe);

    // remove pipe file
    if (0 != remove(pipe_name))
    {
        RPCSTRACE("[RPC] IOCP Delete File<%s> Fail, ERRNO[%d].\n", pipe_name, errno);
    } else
    {
        RPCSTRACE("[RPC] IOCP Delete File<%s> OK.\n", pipe_name);
    }
#endif

#if (defined(_WIN32) || defined(_WIN64))
    return 0;
#else
    return (void *)0;
#endif
}

void CTcpServer::CloseClient(LPPER_HANDLE_DATA lpPerHandleData, LPPER_IO_OPERATION_DATA lpPerIOConText)
{
    CTCPSocket tempSocket;

    if (lpPerHandleData)
    {
        m_IODataMapLock.OwnerLock();
        tempSocket = (lpPerHandleData->Socket);

        if (lpPerIOConText != NULL && NULL == lpPerIOConText->IoData.Socket.GetSocketHandle())
        {
            MASTRACE(("Free lpPerHandleData<0x%p>, sokt removed outside."), (void **)lpPerHandleData);
            RemoveSocketLock((SOCKET)tempSocket.GetSocketHandle());
            DeleteIOData(lpPerIOConText);
        } else
        {
            CSocket_IODATA::iterator it = m_IODataMap.find((int)lpPerHandleData->Socket);

            if (it != m_IODataMap.end())
            {
                tempSocket = (lpPerHandleData->Socket);
                if (NULL != m_SocketExitCB) (m_SocketExitCB)(&tempSocket, m_ExitUserDATA); //关闭回调

                DeleteIOData(it->second);
                m_IODataMap.erase(it++);

                m_pmsg_site->RemoveSock(tempSocket.GetSocketHandle());

                EraseHostConn(tempSocket);
                RemoveSocketLock((SOCKET)tempSocket.GetSocketHandle());

                EraseClt2SvrPair(tempSocket);

                if (tempSocket.m_bAutoConn)
                {
                    AddAutoConn(tempSocket, FALSE);
                }
                tempSocket.Close();


                printf(("erase LPPER_HANDLE_DATA socket[%d]"), (int)tempSocket);
            } else
            {
                printf(("[warning]CloseClient can not find socket %d"), (int)lpPerHandleData->Socket);
            }
#if (defined(__APPLE__) || defined(macintosh) || defined(__MACOSX__))
            pthread_t selfThread = pthread_self();

            m_ThreadHoldSizeLock.Lock();

            if (m_SelectModeThreadHold.find(selfThread) != m_SelectModeThreadHold.end())
            {
                ASSERT(0 < m_SelectModeThreadHold[selfThread]);
                m_SelectModeThreadHold[selfThread] = m_SelectModeThreadHold[selfThread] - 1;
            }

            m_ThreadHoldSizeLock.Unlock();
#endif
            lpPerHandleData = NULL;
        }

        m_IODataMapLock.LeaveLock();
    }

    return;

}

void CTcpServer::AddClt2SvrPair(u64& index, CTCPSocket& t)
{
    m_mapClt2SvrConnLock.OwnerLock();

    if (m_mapClt2SvrConn.end() == m_mapClt2SvrConn.find(index))
    {
        m_mapClt2SvrConn.insert(make_pair(index, t));
    } else
    {
        ASSERT(0);
    }

    m_mapClt2SvrConnLock.LeaveLock();
}

void CTcpServer::EraseClt2SvrPair(CTCPSocket& t)
{
    map<u64, CTCPSocket>::iterator it;
    u64 pairip = make_pair_ip(t.m_dwLocalIP, t.m_dwRemoteIP);

    printf("erase ctl2svr  c[%s] s[%s] socket[%d]\n", ConvertIPtoString(t.m_dwLocalIP).c_str(),
           ConvertIPtoString(t.m_dwRemoteIP).c_str(), (int)t);

    m_mapClt2SvrConnLock.OwnerLock();

    if (m_mapClt2SvrConn.end() != (it = m_mapClt2SvrConn.find(pairip)))
    {
        printf("real erase clt2svr socket[%d] key[%llx]\n", (int)(it->second), pairip);
        m_mapClt2SvrConn.erase(it);
    }

    m_mapClt2SvrConnLock.LeaveLock();
}

void CTcpServer::CloseClientSafeOutOfIOCP(CTCPSocket& sokt)
{
    LPPER_IO_OPERATION_DATA lpPerIoData = NULL;
    CSocket_IODATA::iterator it;

    if (sokt.GetSocketHandle())
    {
        m_pmsg_site->RemoveSock(sokt.GetSocketHandle());

        m_IODataMapLock.OwnerLock();
        it = m_IODataMap.find((SOCKET)sokt.GetSocketHandle());
        if (it != m_IODataMap.end())
        {
            if (it->second->IoData.Socket.m_bAutoConn)
            {
                AddAutoConn(it->second->IoData.Socket, FALSE);
            }

            lpPerIoData = m_IODataMap[(SOCKET)sokt.GetSocketHandle()];

            EraseHostConn(lpPerIoData->IoData.Socket);
            RemoveSocketLock((SOCKET)sokt.GetSocketHandle());
            EraseClt2SvrPair(sokt);

            lpPerIoData->IoData.Socket.ZeroSocketHandle();
            m_IODataMap.erase((SOCKET)sokt);
            if (NULL != m_SocketExitCB) (m_SocketExitCB)(&sokt, m_ExitUserDATA);


            sokt.Close();
        } else
        {
            MASTRACE("[warning] closeclientsafeoutofiocp can not find socket[%d]", (SOCKET)sokt);
        }

        m_IODataMapLock.LeaveLock();
    }

    return;
}

void CTcpServer::CloseAllSocket()
{
    CTCPSocket tempSocket;
    LPPER_IO_OPERATION_DATA lpPerIoData = NULL;

    vector<CTCPSocket> vecTmp;
    m_IODataMapLock.OwnerLock();
    while (m_IODataMap.size())
    {
        CSocket_IODATA::iterator it = m_IODataMap.begin();
        if (it == m_IODataMap.end())
        {
            break;
        }

        if (it->second->IoData.Socket.m_bAutoConn)
        {
            AddAutoConn(it->second->IoData.Socket, FALSE);
        }

        tempSocket = it->first;
        lpPerIoData = it->second;

        EraseHostConn(lpPerIoData->IoData.Socket);
        vecTmp.push_back(lpPerIoData->IoData.Socket);

        lpPerIoData->IoData.Socket.ZeroSocketHandle();
        m_IODataMap.erase(it++);

        if (NULL != m_SocketExitCB)
        {
            (m_SocketExitCB)(&tempSocket, m_ExitUserDATA);
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

    m_IODataMapLock.LeaveLock();

    for (int i = 0; i < (int)vecTmp.size(); i++)
    {
        m_pmsg_site->RemoveSock(vecTmp[i].GetSocketHandle());
    }
}

BOOL CTcpServer::CreateListenThread(DWORD dwBindIp)
{
    int ret = 0;

    ASSERT(m_Port);
    CTCPSocket *ls = new CTCPSocket();


    ls->Create();

    if (0 != ls->Listen(dwBindIp, (USHORT)m_Port, MAX_CLIENT))
    {
        return FALSE;
    }

    m_arrListenSocket.push_back(ls);

    ListenThdParam *lp = new ListenThdParam;

    lp->ls = ls;
    lp->ptr = this;

#if (defined(_WIN32) || defined(_WIN64))
    m_ListenThread = beginthread(NULL, 0, _TCPListenThread, (LPVOID)this, 0, NULL);

    if (NULL == m_ListenThread)
    {
        return FALSE;
    }
#else
    ret = beginthread(&m_ListenThread, NULL, _TCPListenThread, (LPVOID)lp);
    if (0 != ret)
    {
        return FALSE;
    }
#endif

    return TRUE;
}

BOOL CTcpServer::CreateConnectThread()
{
    OSTHREAD_H tmp_thread_id;

#if (defined(_WIN32) || defined(_WIN64))
    tmp_thread_id = beginthread(NULL, 0, _TCPAutoConnect, (LPVOID)this, 0, NULL);

    if (NULL == m_ListenThread)
    {
        return FALSE;
    }

    CloseHandle(tmp_thread_id);
#else
    if (0 != beginthread(&tmp_thread_id, NULL, _TCPAutoConnect, (void *)this))
    {
        MASTRACE("[FAIL] pthread_create FAIL, ERROR[%d]\n", errno);
        return FALSE;
    }
#endif

    return TRUE;
}


OSTHD_FUNC_RET __stdcall CTcpServer::_TCPListenThread(LPVOID lpParam)
{

    CTCPSocket sAccept;
    ListenThdParam *lp = (ListenThdParam *)lpParam;

    LPPER_IO_OPERATION_DATA lpPerIOData = NULL;

    DWORD dwRecvBytes = 0;

    CTcpServer *pTcpSvr = (CTcpServer *)lp->ptr;

    while (!pTcpSvr->m_bListenExit)
    {
        lpPerIOData = pTcpSvr->AcceptConnection(lp->ls, sAccept);
        if (NULL == lpPerIOData)
        {
            continue;
        }

#if (defined(_WIN32) || defined(_WIN64))
        try
        {
            dwRecvBytes = sAccept.Read(lpPerIOData);
        }
        catch (FiExcepction& err)
        {
            err.ShowErrMsg();
        }
#endif
    }

#if (defined(_WIN32) || defined(_WIN64))
    return 0;
#else
    return (void *)0;
#endif
}

HANDLE CTcpServer::ConnSvr(DWORD dwLocalIp, DWORD RemoteIp, USHORT nPort, BOOL bIsCtrlSvr)
{
    CTCPSocket sConnecter;
    struct sockaddr_in server;
    LPPER_IO_OPERATION_DATA lpPerIOData = NULL;

    sConnecter.m_dwLocalIP = dwLocalIp;
    sConnecter.m_dwRemoteIP = RemoteIp;
    sConnecter.m_dwPeerPort = nPort;
    sConnecter.m_IsRev = TRUE;

    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(nPort);
    server.sin_addr.s_addr = RemoteIp;

    sConnecter.RecConnInfo(dwLocalIp, RemoteIp, nPort);
    sConnecter.Create();

    if (0 != dwLocalIp)
    {
        sConnecter.Bind(dwLocalIp);
    }

    BOOL bSuccess = sConnecter.Connect((LPCSOCKADDR) & server);

    sConnecter.m_bAutoConn = true;

    if (!bSuccess)
    {
        printf("connect svr %s port[%d] failed\n", ConvertIPtoString(RemoteIp).c_str(), nPort);
        sConnecter.Cleanup();

        AddAutoConn(sConnecter, FALSE);

        return NULL;
    }

    lpPerIOData = AssociateSocketWithCompletionPort(sConnecter);

    if (FALSE == lpPerIOData)
    {
        MASTRACE(("[fail] AssociateSocketWithCompletionPort(%d) ERRORCode[%d]!"),
                 (int)(long)(HANDLE)sConnecter, (int)errno);

        sConnecter.Close();

        return NULL;
    }

    AddSocketLock((SOCKET)sConnecter.GetSocketHandle());

    MASTRACE(("accpte client ip[%s] connected  socket[%d]"),
             ConvertIPtoString(RemoteIp).c_str(), (SOCKET)sConnecter.GetSocketHandle());

    AddAutoConn(sConnecter, TRUE);

    if (m_SocketCreateCB != NULL)
    {
        CTCPSocket tempSocket;
        tempSocket = (SOCKET)sConnecter.GetSocketHandle();
        tempSocket.m_IsRev = TRUE;
        tempSocket.m_bAutoConn = sConnecter.m_bAutoConn;
        tempSocket.RecConnInfo(dwLocalIp, RemoteIp, nPort);

        m_SocketCreateCB(&tempSocket, RemoteIp, m_CreateUserDATA);
    }

    return sConnecter.GetSocketHandle();
}

LPPER_IO_OPERATION_DATA CTcpServer::AcceptConnection(CTCPSocket *ls, CTCPSocket& sConn)
{
    ASSERT(ls);

    LPPER_IO_OPERATION_DATA lpPerIOData = NULL;
    sockaddr_in InternetAddr;

    int len;
    DWORD dwIp = 0;

    memset(&InternetAddr, 0, sizeof(sockaddr_in));

    len = sizeof(InternetAddr);
    ls->Accept(sConn, (sockaddr *)&InternetAddr, len);

    dwIp = InternetAddr.sin_addr.s_addr;
    sConn.m_dwLocalIP = ls->m_dwLocalIP;
    sConn.m_dwRemoteIP = dwIp;
    sConn.m_IsRev =  TRUE;

    if ((SOCKET)sConn.GetSocketHandle() == 0)
    {
        MASTRACE("socket is null continue");
        return NULL;
    }

    lpPerIOData = AssociateSocketWithCompletionPort(sConn);
    if (NULL == lpPerIOData)
    {
        STRACE(("associate socket failed"));
        ASSERT(0);
        return NULL;
    }

    AddSocketLock((SOCKET)sConn.GetSocketHandle());

    MASTRACE(("accpte client ip[%s] connected  socket[%d]"),
             ConvertIPtoString(dwIp).c_str(), (SOCKET)sConn.GetSocketHandle());
    CTCPSocket tempSocket;
    tempSocket = (SOCKET)sConn.GetSocketHandle();
    tempSocket.m_IsRev = TRUE;
    tempSocket.m_dwRemoteIP = dwIp;
    tempSocket.m_dwLocalIP = ls->m_dwLocalIP;

    //AddHostConn(tempSocket);

    SendVerifyMsg(tempSocket);

    if (m_SocketCreateCB != NULL)
    {
        m_SocketCreateCB(&tempSocket, dwIp, m_CreateUserDATA);
    }

    return lpPerIOData;
}

LPPER_IO_OPERATION_DATA CTcpServer::AssociateSocketWithCompletionPort(CTCPSocket tcpSocket)
{
    LPPER_IO_OPERATION_DATA lpPerIOData = NULL;

    lpPerIOData = CreateInfo(tcpSocket);
    if (NULL == lpPerIOData)
    {
        STRACE(("[fail] 内存分配错误!"));
        return NULL;
    }

#if (defined(_WIN32) || defined(_WIN64))
    HANDLE hResult = CreateIoCompletionPort(tcpSocket, m_CompletionPort, (DWORD) & (lpPerIOData->IoData), 0);
    if (NULL == hResult)
    {
        _SafeDeletePtr(lpPerIOData);
        STRACE(_T("[FAIL] Add Socket[%d] to IOCP fail!"));
        return NULL;
    }
#elif defined(__linux__)
    int opts = fcntl((SOCKET)tcpSocket.GetSocketHandle(), F_GETFL);
    opts = opts | O_NONBLOCK;

    fcntl((SOCKET)tcpSocket.GetSocketHandle(), F_SETFL, opts);

    struct epoll_event ev;
    ev.events =    EPOLLHUP | EPOLLERR | EPOLLIN | EPOLLET;
    ev.data.fd = (SOCKET)tcpSocket.GetSocketHandle();

    if (epoll_ctl(m_fdEpoll, EPOLL_CTL_ADD, (SOCKET)tcpSocket.GetSocketHandle(), &ev) < 0)
    {
        STRACE(("EPOLL_CTL FAIL ERROR:%d"), errno);
    } else
    {
        STRACE(("EPOLL_CTL add success:%d"), (SOCKET)tcpSocket.GetSocketHandle());
    }


#else
    map<pthread_t, int>::iterator it;
    map<pthread_t, DWORD>::iterator holdIt;
    SOCKET curSock = (SOCKET)tcpSocket.GetSocketHandle();

    m_ThreadHoldSizeLock.Lock();

    holdIt = m_SelectModeThreadHold.begin();

    DWORD dwFisrtHold = holdIt->second;

    // 脙驴沤脦露艗路脜脭脷鹿脺脌铆脳卯脡脵SOCKET碌脛脧脽鲁脤

    for (it = m_SelectModeThreadActor.begin(); it != m_SelectModeThreadActor.end(); it++)
    {
        if (m_SelectModeThreadHold[it->first] < dwFisrtHold && m_SelectModeThreadHold[it->first] < FD_SETSIZE)
        {
            // 脫脡脫脷脢脟脥卢脪禄啪枚艙酶鲁脤脛脷虏驴艙禄禄楼, 脧脠虏禄驴艗脗脟脢搂掳脺脟茅驴枚
            write(it->second, &curSock, sizeof(SOCKET));

            m_SelectModeThreadHold[it->first] = m_SelectModeThreadHold[it->first] + 1;
            break;
        }
    }

    if (it == m_SelectModeThreadActor.end())
    {
        // 脫脡脫脷脢脟脥卢脪禄啪枚艙酶鲁脤脛脷虏驴艙禄禄楼, 脧脠虏禄驴艗脗脟脢搂掳脺脟茅驴枚
        it = m_SelectModeThreadActor.begin();

        if (m_SelectModeThreadHold[it->first] <= FD_SETSIZE)
        {
            write(it->second, &curSock, sizeof(SOCKET));

            m_SelectModeThreadHold[it->first] = m_SelectModeThreadHold[it->first] + 1;
        } else
        {
            RPCSTRACE("All IOCP Handle Thread is full...");
            DeleteIOData(lpPerIOData);
            m_ThreadHoldSizeLock.Unlock();
            return NULL;
        }
    }

    m_ThreadHoldSizeLock.Unlock();
#endif
    printf("accepte epoll_ctl add socket[%d] \n", (int)tcpSocket);
    m_IODataMapLock.OwnerLock();
    m_IODataMap[(int)tcpSocket] = lpPerIOData;
    m_IODataMapLock.LeaveLock();

    m_pmsg_site->InsertSock(tcpSocket.GetSocketHandle());

    return lpPerIOData;
}

LPPER_IO_OPERATION_DATA CTcpServer::CreateInfo(CTCPSocket& tcpSocket)
{
    LPPER_IO_OPERATION_DATA lpPerIOData = NULL;

    lpPerIOData = new PER_IO_OPERATION_DATA;
    if (FALSE == lpPerIOData->Initialize(tcpSocket))
    {
        return NULL;
    }

    return lpPerIOData;
}

BOOL CTcpServer::SendMesg(CMemItem *temBuf, CTCPSocket *sokt, BOOL isASASynWrite)
{
    BOOL br = FALSE;
    CCritSec *pSlk = NULL;

    int fd = (SOCKET)sokt->GetSocketHandle();

    m_socketCritSecLock.ShareLock();

    CSocket_CritSecMap::iterator it = m_socketCritSec.begin();
    for (; it != m_socketCritSec.end(); it++)
    {
        //printf("sokcet [%d] lock[%p] \n", it->first, it->second);
    }

    it = m_socketCritSec.find(fd);
    if (it != m_socketCritSec.end())
    {
        pSlk = it->second;
        if (NULL == pSlk)
        {
            MASTRACE(("[fail]can not find socket lock发送消息失败！[%s]"), sokt->GetError().c_str());
            goto LABEL;
        }
    } else
    {
        MASTRACE(("[fail]can not find socket lock发送消息失败！[%s]"), sokt->GetError().c_str());
        goto LABEL;
    }

    pSlk->Lock();

    if (sokt->SendMsg(temBuf, isASASynWrite))
    {
        //printf("sendmsg success!\n");
        br = TRUE;
    } else
    {
        printf("sendmsg failed!\n");
        br = FALSE;
    }

    pSlk->Unlock();

LABEL:
    m_socketCritSecLock.LeaveLock();

    return br;
}

BOOL CTcpServer::SendMsgBlks(CMemItem *temBuf, PVOID pDataBuf, DWORD dwBufSize, CTCPSocket *sokt, DWORD& dwStatus, BOOL isASASynWrite)
{
    BOOL br = FALSE;
    CCritSec *pSlk = NULL;

    m_socketCritSecLock.ShareLock();

    CSocket_CritSecMap::iterator it = m_socketCritSec.find((SOCKET)sokt);
    if (it != m_socketCritSec.end())
    {
        pSlk = it->second;
        if (NULL == pSlk)
        {
            MASTRACE(("[fail]can not find socket lock发送消息失败！[%s]"), sokt->GetError().c_str());
            goto LABEL;
        }
    } else
    {
        MASTRACE(("[fail]can not find socket lock发送消息失败！[%s]"), sokt->GetError().c_str());
        goto LABEL;
    }

    pSlk->Lock();
    if (sokt->SendMsgBlks(temBuf, pDataBuf, dwBufSize, dwStatus, isASASynWrite))
    {
        MASTRACE(("[ok]SOCKET[%d]SendMsgBlks"), (SOCKET)sokt);
        br = TRUE;
    } else
    {
        MASTRACE(("[fail]SendMsgBlks[%s]"), sokt->GetError().c_str());
        br = FALSE;
    }

    pSlk->Unlock();

LABEL:
    m_socketCritSecLock.LeaveLock();

    return br;
}

void CTcpServer::DeleteIOData(LPPER_IO_OPERATION_DATA lpPerIOConText)
{
    if (lpPerIOConText != NULL)
    {
        if (lpPerIOConText->pMsgBuf != NULL)
        {
            _SafeDeletePtr(lpPerIOConText->pMsgBuf);
        }

        _SafeDeletePtr(lpPerIOConText);
    }
}

CCritSec* CTcpServer::AddSocketLock(int s)
{
    CCritSec *critsec = NULL;
    m_socketCritSecLock.OwnerLock();
    CSocket_CritSecMap::iterator it = m_socketCritSec.find(s);
    if (it == m_socketCritSec.end())
    {
        critsec = new CCritSec;
        m_socketCritSec[s] = critsec;
        printf(("AddSocketLock Socket[%d]  CritSec[%p]\n"), s, critsec);

    } else
    {
        STRACE(("AddSocketLock exist Socket"));
        ASSERT(0);
    }

    m_socketCritSecLock.LeaveLock();

    return critsec;
}

CCritSec* CTcpServer::GetSocketLock(int s)
{
    CCritSec *ret = NULL;
    m_socketCritSecLock.ShareLock();

    CSocket_CritSecMap::iterator it = m_socketCritSec.find(s);

    if (it != m_socketCritSec.end())
    {
        ret = it->second;
    }

    m_socketCritSecLock.LeaveLock();

    return ret;
}

void  CTcpServer::RemoveSocketLock(int s)
{
    CCritSec *lk = NULL;

    m_socketCritSecLock.OwnerLock();

    CSocket_CritSecMap::iterator it = m_socketCritSec.find(s);
    if (it != m_socketCritSec.end())
    {
        lk = it->second;

        // if other thread just locked ,then must wait other thread unlocked when delete
        lk->Lock();
        lk->Unlock();

        _SafeDeletePtr(lk);
        m_socketCritSec.erase(it);
        printf("RemoveSocketLock %d \n", s);
    } else
    {
        STRACE(("Socket lock can not find"));
    }

    m_socketCritSecLock.LeaveLock();
}

void CTcpServer::AddAutoConn(CTCPSocket& ws, BOOL bConn)
{
    BOOL bFind = FALSE;

    ASSERT(ws.m_bAutoConn);

    m_lkconn.Lock();

    for (int i = 0; i < (int)m_vecconn.size(); i++)
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
        if (!((ws.m_dwLocalIP == ws.m_dwRemoteIP) && (ws.m_dwPeerPort == m_Port)))
        {
            connect_rec rec(ws.m_dwLocalIP, ws.m_dwRemoteIP, ws.m_uRemotePort, bConn);
            m_vecconn.push_back(rec);
        } else
        {
            bFind = TRUE;
            STRACE(" IN one process can not self to connect self!!");
        }
    }

    m_lkconn.Unlock();

    if (!bConn)
    {
        m_EvConn.Post();
    }
}

void CTcpServer::EraseHostConn(CTCPSocket& t)
{
    std::map<u32, CHostInfo *>::iterator im;
    CHostInfo *hs = NULL;
    u32 identify = 0;

    m_mapHostLock.Lock();

    identify = GetMapIdentify(t.m_dwRemoteIP);

    if (m_mapHost.end() != (im = m_mapHost.find(identify)))
    {
        hs = im->second;
        hs->EraseConn(t.m_dwRemoteIP, t);

        if (hs->m_mapProcId.size() == 0)
        {
            printf("erase host all info identify [%s] obj[%p]____________\n", ConvertIPtoString((DWORD)hs->m_identify).c_str(), hs);
            _SafeDeletePtr(hs);
        }

        printf("erase host map ip[%s] to  obj[%p]\n", ConvertIPtoString(t.m_dwRemoteIP).c_str(), hs);

        m_mapHost.erase(im);
    } else
    {
        printf("EraseHostConn client %s has not register!! \n", ConvertIPtoString(t.m_dwRemoteIP).c_str());
    }

    EraseMapIdentify(t.m_dwRemoteIP);

    m_mapHostLock.Unlock();
}

int CTcpServer::RegHostCall(CTCPSocket *s)
{
    int  ret = -1;
    FiEvent ev;
    CMemItem *retmsg = NULL;
    CMemItem *pNewMsg = NULL;
    ReqCxt req;
    call_cxt cxt;

    CMemItem mi;
    mi.SetBufferSize(1024);
    m_cfg.m_LocalIp.SaveToMem(&mi);

    cxt.in_modid = m_Mod->modID;
    cxt.in_buf1 = (char *)mi.GetBufferPtr();
    cxt.in_buf1len = mi.GetDataRealSize();
    cxt.in_buf2 = NULL;
    cxt.in_buf2len = 0;

    cxt.inway.vecSrc.push_back(s->m_dwLocalIP);
    cxt.inway.vecDst.push_back(s->m_dwRemoteIP);
    cxt.inway.time_out = 10000; // 10 second

    CModuleInfo *mod = GetModule(cxt.in_modid);

    pNewMsg = GenSheet(SHEET_CALL, cxt);
    Tsheet *sh = (Tsheet *)pNewMsg->GetBufferPtr();

    req.ev = &ev;
    req.modid = cxt.in_modid;
    req.pktid = sh->sheet_num;
    req.tunnelbuf = cxt.in_tunnelbuf;
    req.tunnelbuflen = cxt.in_tunnelbuflen;

    mod->AddReqCxt(req);

    if (NULL != s)
    {
        /// send msg then wait msg reply
        ret = CallSend(s, pNewMsg, cxt.in_buf2, cxt.in_buf2len);

        if (ret == 0) /// send failed -1
        {
            //NET_TIMEOUT

            ret = ev.Wait(-1);

            if (ret != -1)
            {
                retmsg = mod->GetModMsgReply(req.pktid);

                cxt.out_buf = retmsg;
                ASSERT(SVR_REGHOST_OK == *((DWORD *)retmsg->GetBufferPtr()));
                ret =  0;

                printf("local ip [%s] reg to svr [%s]success \n", ConvertIPtoString(s->m_dwLocalIP).c_str(), ConvertIPtoString(s->m_dwRemoteIP).c_str());
                /// fixme delete retsh msg
            }
        }
    }

    mod->EraseReqCxt(req.pktid);

    _SafeDeletePtr(pNewMsg);

    if (NULL != cxt.out_buf)
    {
        _SafeDeletePtr(cxt.out_buf);
    }
    return ret;
}

int CTcpServer::RegHostReply(CMemItem *sheet, CTCPSocket *s)
{
    int i = 0;
    DWORD dwStatus = 0;
    Tsheet *newsh = NULL;
    CMemItem *pNewMsg = NULL;
    BOOL ret = FALSE;
    BOOL bFind = FALSE;
    BOOL bOther = FALSE;
    DWORD dwRegRet = SVR_REGHOST_OK;

    Tsheet *sh = (Tsheet *)sheet->GetBufferPtr();

    ASSERT(sh);

    pNewMsg = GetReplySheet(sh, (char *)&dwRegRet, sizeof(DWORD), NULL, 0);
    ASSERT(pNewMsg);

    ret = SendMesg(pNewMsg, s);

    if (sh->oneoff)
    {
        CModuleInfo *mod = GetModule(sh->reg_module_type);
        mod->EraseOnceCall(sh->sheet_num);
    }

    /// delete old msg
    _SafeDeletePtr(sheet);
    _SafeDeletePtr(pNewMsg);

    return 0;
}

OSTHD_FUNC_RET CTcpServer::_ClientRegisterThd(LPVOID lpParam)
{
    CHostMap::iterator ih;
    CTcpServer *pIO = (CTcpServer *)lpParam;
    Tsheet *sh = NULL;
    reg_host hs;

    while (true)
    {
        pIO->m_ModuleSem.Wait();

        /// client multiple netcard register
        pIO->m_ModuleLock.Lock();

        if (pIO->m_qModuleMsg.size() > 0)
        {
            hs =  pIO->m_qModuleMsg.front();
            pIO->m_qModuleMsg.pop();
        }

        pIO->m_ModuleLock.Unlock();

        ASSERT(hs.shbuf);
        ASSERT(hs.buff);
        if (NULL != hs.shbuf)
        {
            /// deal with register

            sh = (Tsheet *)hs.shbuf->GetBufferPtr();

            pIO->m_mapHostLock.Lock();

            printf("recv client register hs.s.m_dwRemoteIP[%s], socket[%d]\n",
                   ConvertIPtoString(hs.s.m_dwRemoteIP).c_str(), (int)hs.s);
            ASSERT(hs.s.m_dwRemoteIP);

            pIO->AddMapIdentify(hs.s.m_dwRemoteIP, sh->identity);
            
            if (pIO->m_mapHost.end() == (ih = pIO->m_mapHost.find(sh->identity)))
            {
                CHostInfo *ht = new CHostInfo;
                ht->LoadFromMem(hs.buff->GetBufferPtr(), hs.buff->GetDataRealSize());

                ASSERT(ht->m_identify == sh->identity);
                ASSERT(sh->pid == (ht->m_mapProcId.begin())->first);
                ht->AddConn(hs.s.m_dwRemoteIP,  hs.s, sh->pid);
                pIO->m_mapHost.insert(make_pair(sh->identity, ht));
            } else
            {
                /// fix me, maybe recv twice when network unstable
                ih->second->AddConn(hs.s.m_dwRemoteIP, hs.s, sh->pid);
            }

            pIO->m_mapHostLock.Unlock();

            /// fix me :reture register call reply to client;

            pIO->RegHostReply(hs.shbuf, &hs.s);
        }
    }

#if (defined(_WIN32) || defined(_WIN64))
    return 0;
#endif
}

int CTcpServer::CreateRegThd()
{
    OSTHREAD_H tmp_thread_id;
#if (defined(_WIN32) || defined(_WIN64))
    tmp_thread_id = beginthread(NULL, 0, _ClientRegisterThd, (LPVOID)this, 0, NULL);
    if (NULL == tmp_thread_id)
    {
        printf("[FAIL] CreateRegThd FAIL, ERROR[%d]\n", errno);
        return FALSE;
    }

    CloseHandle(tmp_thread_id);
#else
    if (0 != beginthread(&tmp_thread_id, NULL, _ClientRegisterThd, (void *)this))
    {
        printf("[FAIL] CreateRegThd FAIL, ERROR[%d]\n", errno);
        return FALSE;
    }
#endif
    return TRUE;
}

int CTcpServer::CreateVerifyThd()
{
    OSTHREAD_H tmp_thread_id;
#if (defined(_WIN32) || defined(_WIN64))
    tmp_thread_id = beginthread(NULL, 0, _VerifySvrNotify, (LPVOID)this, 0, NULL);
    if (NULL == tmp_thread_id)
    {
        printf("[FAIL] create _VerifySvrNotify FAIL, ERROR[%d]\n", errno);
        return FALSE;
    }

    CloseHandle(tmp_thread_id);
#else
    if (0 != beginthread(&tmp_thread_id, NULL, _VerifySvrNotify, (void *)this))
    {
        printf("[FAIL] create _VerifySvrNotify FAIL, ERROR[%d]\n", errno);
        return FALSE;
    }
#endif
    return TRUE;
}

BOOL CTcpServer::Send(CMemItem *temBuf, HANDLE sock)
{
    ASSERT(sock);
    CTCPSocket s(sock);

    return SendMesg(temBuf, &s);
}

void CTcpServer::_createcb(CTCPSocket *sokt, DWORD strip, void *userData)
{
    ;
}

void CTcpServer::_exitcb(CTCPSocket *sokt, void *userData)
{
    ;
}

/// net module register
bool CTcpServer::_recvcb(CMemItem *sheetData, CMemItem *soktData, CTCPSocket *psokt, void *userData)
{
    bool ret = true;
    ASSERT(soktData);

    CTcpServer *pSvr = (CTcpServer *)userData;
    ASSERT(pSvr);

    Tsheet *sh = (Tsheet *)sheetData->GetBufferPtr();
    /// notify msg specially svr sended to client,so there is  modid is not exist
    if (sh->direction == SHEET_NOTIFY)
    {
        ret = _verifycb(sheetData, soktData, psokt, userData);
    } else
    {
        CModuleInfo *mod = pSvr->GetModule(sh->reg_module_type);

        if (NULL != mod->modRecv)
        {   ///
            /// reply msg set syn event to notify blocked thread
            if (sh->direction == SHEET_REPLY)
            {
                if (!mod->Notify(sh->sheet_num, soktData))
                {
                    ret = false;
                } else
                {
                    _SafeDeletePtr(sheetData);
                }
            } else
            {
                if (sh->oneoff && sh->direction == SHEET_CALL)
                {
                    if (mod->AddOnceCall(sh->sheet_num))
                    {
                        ret = ((SOCKETMSG_FUNC)(mod->modRecv))(sheetData, soktData, psokt, userData);
                    } else
                    {
                        /// throw the msg
                        ret = false;
                    }
                } else
                {
                    ret = ((SOCKETMSG_FUNC)(mod->modRecv))(sheetData, soktData, psokt, userData);
                }
            }
        } else if (NULL != mod->m_pReceiver)
        {
            /// reply msg set syn event to notify blocked thread
            if (sh->direction == SHEET_REPLY)
            {
                if (!mod->Notify(sh->sheet_num, soktData))
                {
                    ret = false;
                } else
                {
                    _SafeDeletePtr(sheetData);
                }
            } else
            {
                if (sh->oneoff && sh->direction == SHEET_CALL)
                {
                    if (mod->AddOnceCall(sh->sheet_num))
                    {
                        ret = mod->m_pReceiver->ReceivePacket(sheetData, soktData, psokt, userData);
                    } else
                    {
                        ret = false;
                    }
                } else
                {
                    ret = mod->m_pReceiver->ReceivePacket(sheetData, soktData, psokt, userData);
                }
            }
        } else
        {
            ret = false;
        }
    }

    if (!ret)
    {
        _SafeDeletePtr(soktData);
        _SafeDeletePtr(sheetData);
    }

    return ret;
}

bool CTcpServer::_verifycb(CMemItem *sheetData, CMemItem *soktData, CTCPSocket *psokt, void *userData)
{
    ASSERT(sheetData);
    ASSERT(soktData);
    ASSERT(userData);

    Tsheet *sh = (Tsheet *)sheetData->GetBufferPtr();

    CTcpServer *pSvr = (CTcpServer *)userData;

    ASSERT(sh->direction == SHEET_NOTIFY);
    reg_host hs;

    hs.shbuf = sheetData;
    hs.buff = soktData;
    hs.s    = *psokt;

    pSvr->m_NotifyLock.Lock();
    pSvr->m_qNotify.push(hs);
    pSvr->m_NotifyLock.Unlock();

    pSvr->m_NotifySem.Post();

    return true;
}

int CTcpServer::Call(call_cxt& cxt)
{
    int  ret = -1;
    FiEvent ev;
    CTCPSocket ts;
    CMemItem *retmsg = NULL;
    CMemItem *pNewMsg = NULL;
    ReqCxt req;
    int trytimes = cxt.inway.time_out / NET_TIMEOUT;
    int testt = NET_TIMEOUT;
    CModuleInfo *mod = GetModule(cxt.in_modid);

    ASSERT(cxt.inway.vecDst.size() > 0);
    ASSERT(cxt.inway.time_out > 0);

    if (cxt.in_buf1 == NULL)
    {
        ASSERT(cxt.in_buf2 == NULL);
    }

    m_mapModlock.ShareLock();

    if (m_mapMod.end() == m_mapMod.find(cxt.in_modid))
    {
        ASSERT(0);
    }

    m_mapModlock.LeaveLock();

    pNewMsg = GenSheet(SHEET_CALL, cxt);
    Tsheet *sh = (Tsheet *)pNewMsg->GetBufferPtr();

    req.ev = &ev;
    req.modid = cxt.in_modid;
    req.pktid = sh->sheet_num;
    req.tunnelbuf = cxt.in_tunnelbuf;
    req.tunnelbuflen = cxt.in_tunnelbuflen;

    mod->AddReqCxt(req);

    while (true)
    {
        ts  = GetRightSvr(cxt);

        trytimes--;

        if (NULL == ts.GetSocketHandle())
        {
            if (trytimes > 0)
            {
                ret  = m_exit_ev.Wait(1000000);

                if (ret != -1)
                {
                    printf("tcpio recv exit event\n");
                    break;
                }

                printf("can find socket then continue trytimes[%d]\n", trytimes);
                continue;
            }

            break;
        }

        /// send msg then wait msg reply
        ret = CallSend(&ts, pNewMsg, cxt.in_buf2, cxt.in_buf2len);

        if (ret == 0) /// send failed -1
        {
            ret = ev.Wait(1000000);

            if (ret != -1)
            {
                retmsg = mod->GetModMsgReply(req.pktid);

                if (NULL == retmsg)
                {
                    continue;
                }

                cxt.out_buf = retmsg;
                /// fixme delete retsh msg
                ret = 0;
                break;
            }
        }
    }

    mod->EraseReqCxt(req.pktid);

    _SafeDeletePtr(pNewMsg);

    return ret;
}


///
int CTcpServer::Reply(CMemItem *sheet, char *buf1, int buf1len, char *buf2, int buf2len)
{
    int i = 0;
    DWORD dwStatus = 0;
    CTCPSocket s;
    Tsheet *newsh = NULL;
    CMemItem *pNewMsg = NULL;
    BOOL ret = FALSE;
    BOOL bFind = FALSE;
    BOOL bOther = FALSE;
    Tsheet *sh = (Tsheet *)sheet->GetCurDataPtr();

    ASSERT(sh);
    ASSERT(buf1);
    ASSERT(buf1len);

    pNewMsg = GetReplySheet(sh, buf1, buf1len, buf2, buf2len);
    ASSERT(pNewMsg);

    /// route message must select right socket(pair origin client and current server)
    /// route via GetAllCltSocket find suitable socket
    if (sh->direction != SHEET_ROUTE)
    {
        s = (HANDLE)sh->trace_back;

        if (NULL == buf2)
        {
            ret = SendMesg(pNewMsg, &s);
        } else
        {
            ret = SendMsgBlks(pNewMsg, buf2, buf2len, &s, dwStatus);
        }
    }

    /// find right socket
    if (!ret)
    {
        vector<CTCPSocket> vec;

        GetAllCltSocket(sh, vec);

        for (int i = 0; i < (int)vec.size(); i++)
        {
            if (vec[i].GetSocketHandle() != s.GetSocketHandle())
            {
                if (buf2)
                {
                    ret = SendMesg(pNewMsg, &s);
                } else
                {
                    ret = SendMsgBlks(pNewMsg, buf2, buf2len, &s, dwStatus);
                }

                if (ret)
                {
                    break;
                }
            }
        }
    }

    if (sh->oneoff)
    {
        CModuleInfo *mod = GetModule(sh->reg_module_type);
        mod->EraseOnceCall(sh->sheet_num);
    }

    /// delete old msg
    _SafeDeletePtr(sheet);
    _SafeDeletePtr(pNewMsg);

    return ret;
}

int CTcpServer::Route(call_cxt& cxt, CMemItem *sheet)
{
    CTCPSocket ts;
    DWORD trytimes = cxt.inway.time_out / NET_TIMEOUT;
    CMemItem *pNewMsg = NULL;
    BOOL ret = FALSE;
    Tsheet *sh = (Tsheet *)sheet->GetCurDataPtr();

    ASSERT(sh);
    ASSERT(cxt.in_buf1);
    ASSERT(cxt.in_buf1len);

    pNewMsg = GetRouteSheet(sh, cxt.in_buf1, cxt.in_buf1len);

    ASSERT(pNewMsg);

    /// find right socket
    while (true)
    {
        ts  = GetRightSvr(cxt);

        trytimes--;

        if (NULL == ts.GetSocketHandle())
        {
            if (trytimes > 0)
            {
                continue;
            } else
            {
                printf("trytiem is 0 \n");
            }

            break;
        }

        ret = SendMesg(pNewMsg, &ts);

        if (ret)
        {
            break;
        }
    }

    if (sh->oneoff)
    {
        CModuleInfo *mod = GetModule(sh->reg_module_type);
        mod->EraseOnceCall(sh->sheet_num);
    }

    /// delete old msg
    _SafeDeletePtr(sheet);
    _SafeDeletePtr(pNewMsg);

    return ret;
}

int CTcpServer::LocalCall(call_cxt& cxt)
{
    int r = -1;

    return r;
}

int CTcpServer::RouteCall(call_cxt& cxt, CMemItem* sheet)
{
      int r = -1;

    return r;
}

void CTcpServer::GetAllCltSocket(Tsheet *sh, vector<CTCPSocket>& vec)
{
    int i = 0;
    CHostInfo host;
    BOOL bFind = FALSE;
    /// get clt send ip
    vector<DWORD> vecCltSendIp;
    map<u32, CHostInfo *>::iterator it;

    if (sh->direction != SHEET_ROUTE)
    {
        for (i = 0; i < sh->dst_ip_num; i++)
        {
            DWORD ip  = *(DWORD *)(TSHEET_MSG_OFFS(sh) - sh->dst_ip_num * sizeof(DWORD) + i * sizeof(DWORD));
            vecCltSendIp.push_back(ip);
        }
    }

    m_mapHostLock.Lock();

    if (m_mapHost.end() != (it = m_mapHost.find(sh->identity)))
    {
        host =  *it->second;
        bFind = TRUE;
    }

    m_mapHostLock.Unlock();

    if (bFind)
    {
        if (sh->direction == SHEET_ROUTE)
        {
            host.GetAllHostSocket(vec, sh->pid);
        } else
        {
            host.GetAllHostSocket(vec, vecCltSendIp, sh->pid);
        }
    }
}

CMemItem* CTcpServer::GetReplySheet(Tsheet *src, char *buf1, int buf1len, char *buf2, int buf2len)
{
    Tsheet *t = NULL;
    CMemItem *pMsg = NULL;

    ASSERT(src);
    ASSERT(buf1);

    if (NULL == buf2)
    {
        ASSERT(0 == buf2len);
    }

    int totalsize = src->sheet_size + buf1len + buf2len;

    pMsg = new CMemItem;
    pMsg->SetBufferSize(totalsize);
    pMsg->SetDataRealSize(totalsize);

    t = (Tsheet *)pMsg->GetBufferPtr();

    memcpy((char *)t, (char *)src, src->sheet_size);

    t->direction = SHEET_REPLY;
    t->total_size = totalsize;
    t->carry_msg_len = buf1len + buf2len;

    memcpy(TSHEET_MSG_OFFS(t), buf1, buf1len);

    if (buf2)
    {
        t->MarkSheet(buf2, buf2len);
    } else
    {
        t->MarkSheet(TSHEET_MSG_OFFS(t), buf1len);
    }

    return pMsg;
}

CMemItem*  CTcpServer::GetRouteSheet(Tsheet *src, char *buf, int buflen)
{
    Tsheet *t = NULL;
    CMemItem *pMsg = NULL;

    ASSERT(src);
    ASSERT(buf);

    int totalsize = src->sheet_size + buflen;

    pMsg = new CMemItem;
    pMsg->SetBufferSize(totalsize);
    pMsg->SetDataRealSize(totalsize);

    t = (Tsheet *)pMsg->GetBufferPtr();

    memcpy((char *)t, (char *)src, src->sheet_size);

    t->direction = SHEET_ROUTE;
    t->total_size = totalsize;
    t->carry_msg_len = buflen;

    memcpy(TSHEET_MSG_OFFS(t), buf, buflen);

    t->MarkSheet(TSHEET_MSG_OFFS(t), buflen);

    return pMsg;
}

void CTcpServer::GetOriginIP(CMemItem *sheet, vector<DWORD>& org)
{
    DWORD *pt = NULL;
    Tsheet *sh = (Tsheet *)sheet->GetCurDataPtr();

    ASSERT(sh);
    ASSERT(sh->src_ip_num);

    pt = (DWORD *)(sheet->GetCurDataPtr() + sizeof(Tsheet));

    for (u8 i = 0; i < sh->src_ip_num; i++)
    {
        org.push_back(*pt++);
    }
}

CHostInfo* CTcpServer::GetSvrHost(DWORD ip)
{
    CHostInfo *pH = NULL;
    map<DWORD, CHostInfo *>::iterator it;
    ASSERT(ip);

    m_mapConfigSvrHostLock.ShareLock();
    if (m_cfg.m_RemoteIp.end() != (it = m_cfg.m_RemoteIp.find(ip)))
    {
        pH = it->second;
    }

    m_mapConfigSvrHostLock.LeaveLock();

    return pH;
}

CTCPSocket CTcpServer::GetRightSvr(call_cxt& cxt)
{
    int i = 0;
    CTCPSocket ts;
    BOOL bFind = FALSE;
    DWORD TryTimes = 0;
    CHostInfo *pSvr = NULL;
    u64 index = 0;
    DWORD dst = 0;

    map<u64, CTCPSocket>::iterator it;

    ASSERT(cxt.inway.vecDst.size());

    TryTimes = cxt.inway.vecSrc.size() * cxt.inway.vecDst.size();

    if (0 == TryTimes)
    {
        TryTimes = cxt.inway.vecDst.size();
    }

    m_mapClt2SvrConnLock.ShareLock();

    if (cxt.inway.polled)
    {
        while (TryTimes)
        {
            TryTimes--;

            index = make_pair_ip(cxt.inway.vecSrc[cxt.inway.src_pos], cxt.inway.vecDst[cxt.inway.des_pos]);
            cxt.inway.des_pos++;

            if (m_mapClt2SvrConn.end() != (it = m_mapClt2SvrConn.find(index)))
            {
                ts = it->second;
                bFind = TRUE;
                break;
            }
            /// des loop over, loop src;
            if (0 == cxt.inway.des_pos % cxt.inway.vecDst.size())
            {
                cxt.inway.src_pos = (++cxt.inway.src_pos) % cxt.inway.vecSrc.size();
                cxt.inway.des_pos = cxt.inway.des_pos % cxt.inway.vecDst.size();
            }

        }
    } else
    {
        while (TryTimes)
        {
            index = make_pair_ip(cxt.inway.vecSrc[cxt.inway.src_pos], cxt.inway.vecDst[cxt.inway.des_pos]);

            if (m_mapClt2SvrConn.end() != (it = m_mapClt2SvrConn.find(index)))
            {
                ts = it->second;
                bFind = TRUE;
                break;
            }

            cxt.inway.des_pos++;

            if (0 == cxt.inway.des_pos % cxt.inway.vecDst.size())
            {
                cxt.inway.src_pos = (++cxt.inway.src_pos) % cxt.inway.vecSrc.size();
                cxt.inway.des_pos = cxt.inway.des_pos % cxt.inway.vecDst.size();
            }

            TryTimes--;
        }
    }

    /// find other connect in config all svr multiple netcard
    if (!bFind)
    {
        for (int j = 0; j < cxt.inway.vecDst.size(); j++)
        {
            pSvr = GetSvrHost(cxt.inway.vecDst[j]);
            map<DWORD, CNetCard>::iterator im;

            if (NULL == pSvr)
            {
                continue;
            }          

            CProcId pd = (pSvr->m_mapProcId.begin()->second);
            for (im = pd.m_mapIp.begin(); im != pd.m_mapIp.end(); im++)
            {
                for (i = 0; i < (int)cxt.inway.vecDst.size(); i++)
                {
                    if (im->first == cxt.inway.vecDst[i])
                    {
                        break;
                    }
                }

                if (i == cxt.inway.vecDst.size())
                {
                    dst = im->first;

                    for (int s = 0; s < (int)cxt.inway.vecSrc.size(); s++)
                    {
                        index = make_pair_ip(cxt.inway.vecSrc[s], dst);

                        if (m_mapClt2SvrConn.end() != (it = m_mapClt2SvrConn.find(index)))
                        {
                            ts = it->second;
                            bFind = TRUE;
                            break;
                        }
                    }

                    if (bFind)
                    {
                        break;
                    }
                }
            }

            if (bFind)
            {
                break;
            }
        }
    }

    m_mapClt2SvrConnLock.LeaveLock();

    return ts;
}

int CTcpServer::CallSend(CTCPSocket *s, CMemItem *pMsg, char *buf2, int buf2len)
{
    BOOL bRet = 0;
    DWORD dwStatus = 0;

    ASSERT(s);
    ASSERT(pMsg);

    if (NULL != buf2)
    {
        bRet = SendMsgBlks(pMsg, buf2, buf2len, s, dwStatus);;
    } else
    {
        bRet = SendMesg(pMsg, s);
    }

    return bRet ? 0 : -1;
}

CMemItem* CTcpServer::GenSheet(u8 sheettype, call_cxt& cxt)
{
    int i = 0;
    int totalsize = 0;
    CMemItem *pMsg = NULL;
    ASSERT(cxt.in_buf1);
    ASSERT(cxt.in_buf1len);

    if (cxt.in_buf1len > SHEET_MAX_SIZE)
    {
        ASSERT(0);
    }

    totalsize = sizeof(Tsheet);

    totalsize += cxt.inway.vecSrc.size() * sizeof(DWORD) + cxt.inway.vecDst.size() * sizeof(DWORD);

    totalsize += cxt.in_buf1len;

    pMsg = new CMemItem;
    ASSERT(pMsg);
    pMsg->SetBufferSize(totalsize);
    pMsg->SetDataRealSize(totalsize);

    Tsheet *sh = (Tsheet *)pMsg->GetBufferPtr();

    sh->head		= SHEET_HEAD;
    sh->total_size	= totalsize;
    sh->sheet_size	= sizeof(Tsheet) + cxt.inway.vecSrc.size() * sizeof(DWORD) + cxt.inway.vecDst.size() * sizeof(DWORD);
    sh->identity		= m_identify;
    sh->version		= SHEET_VERION;
    sh->pid         = FiGetPid();
    sh->tunnelsize	= 0;
    sh->tunnel		= 0;
    sh->direction	= sheettype;
    sh->oneoff		= cxt.in_once;
    sh->reg_module_type	= cxt.in_modid; /// when type msg return call type_call_back_func
    sh->out_time       	= 0;
    sh->query          	= 0; /// when client can not recv svr call back, then query svr sheet_num
    sh->src_ip_num     	= cxt.inway.vecSrc.size();
    sh->dst_ip_num     	= cxt.inway.vecDst.size();
    sh->trace_back     	= 0; /// trace back recv msg socket
    sh->carry_msg_len  	= cxt.in_buf1len; /// user msg header len + user msg data len
    if (sheettype == SHEET_NOTIFY)
    {
        sh->sheet_num      	= 0xFFFFFBFB;
    } else
    {
        sh->sheet_num      	= GetModule(cxt.in_modid)->GetModPktSeq();
    }
    sh->reserve_data   =	 0;


    DWORD *p = (DWORD *)((char *)sh + sizeof(Tsheet));

    for (i = 0; i < (int)cxt.inway.vecSrc.size(); i++)
    {
        *p = cxt.inway.vecSrc[i];
        p++;
    }

    for (i = 0; i < (int)cxt.inway.vecDst.size(); i++)
    {
        *p = cxt.inway.vecDst[i];
        p++;
    }

    memcpy(TSHEET_MSG_OFFS(sh), cxt.in_buf1, cxt.in_buf1len);

    if (NULL != cxt.in_tunnelbuf && 0 != cxt.in_tunnelbuflen)
    {
        sh->tunnelsize = cxt.in_tunnelbuflen;
    }
    /// only add buf2 size to total size
    if (cxt.in_buf2 != NULL && cxt.in_buf2len != 0)
    {
        sh->total_size    += cxt.in_buf2len;
        sh->carry_msg_len += cxt.in_buf2len;

        sh->MarkSheet(cxt.in_buf2, cxt.in_buf2len);
    } else
    {
        sh->MarkSheet(TSHEET_MSG_OFFS(sh), cxt.in_buf1len);
    }

    return pMsg;
}

OSTHD_FUNC_RET __stdcall  CTcpServer::_TCPAutoConnect(LPVOID lpParam)
{
    vector<connect_rec> vecTmp;
    CTcpServer *pObj = (CTcpServer *)lpParam;
    int i = 0;

    while (TRUE)
    {
        pObj->m_EvConn.Wait();

        while (TRUE)
        {
            vecTmp.clear();

            pObj->m_lkconn.Lock();
            for (i = 0; i < (int)pObj->m_vecconn.size(); i++)
            {
                if (!pObj->m_vecconn[i].bConned)
                {
                    printf("get disconnect local <%s> to conn svr <%s> \n", ConvertIPtoString(pObj->m_vecconn[i].dwLocalIP).c_str(),
                           ConvertIPtoString(pObj->m_vecconn[i].dwRemoteIP).c_str());
                    vecTmp.push_back(pObj->m_vecconn[i]);
                }
            }

            pObj->m_lkconn.Unlock();

            for (i = 0; i < (int)vecTmp.size(); i++)
            {
                printf("auto thread begin  local connect [%s] to svr [%s] port[%d] \n", ConvertIPtoString(vecTmp[i].dwLocalIP).c_str(),
                       ConvertIPtoString(vecTmp[i].dwRemoteIP).c_str(),  vecTmp[i].uRemotePort);
                pObj->ConnSvr(vecTmp[i].dwLocalIP, vecTmp[i].dwRemoteIP, vecTmp[i].uRemotePort);
            }

            Sleep(5000);
        }
    }
#if (defined(_WIN32) || defined(_WIN64))
    return 0;
#else
    return (void *)0;
#endif
}

u64 CTcpServer::make_pair_ip(DWORD L, DWORD R)
{
    u64 index = (u32)L;
    index = index << 32;
    index = index | (u32)R;
    return index;
}

OSTHREAD_H CTcpServer::GetCompletionPort()
{
    return m_CompletionPort;
}

void CTcpServer::AddMapIdentify(DWORD &dwip, DWORD& identify)
{
    if(m_mapIdentify.find(dwip) == m_mapIdentify.end())
    {
        m_mapIdentify.insert(make_pair(dwip, identify));
    }
}

DWORD CTcpServer::GetMapIdentify(DWORD &dwip)
{
    DWORD dwi = 0;
    map<u32,u32>::iterator it;

    if((it = m_mapIdentify.find(dwip)) != m_mapIdentify.end())
    {
        dwi = it->second;
    }

    return dwi;
}

void CTcpServer::EraseMapIdentify(DWORD &dwip)
{
    m_mapIdentify.erase(dwip);
}


BOOL CTcpServer::SendVerifyMsg(CTCPSocket& s)
{
    BOOL ret = true;
    CMemItem *pNewMsg = NULL;
    call_cxt cxt;

    CMemItem mi;
    mi.SetBufferSize(1024);
    m_cfg.m_LocalIp.SaveToMem(&mi);

    cxt.in_modid = 0xFFFFFBFB;
    cxt.in_buf1 = (char *)mi.GetBufferPtr();
    cxt.in_buf1len = mi.GetDataRealSize();
    cxt.in_buf2 = NULL;
    cxt.in_buf2len = 0;

    cxt.inway.vecSrc.push_back(s.m_dwLocalIP);
    cxt.inway.vecDst.push_back(s.m_dwRemoteIP);
    cxt.inway.time_out = 10000; // 10 second

    pNewMsg = GenSheet(SHEET_NOTIFY, cxt);
    Tsheet *sh = (Tsheet *)pNewMsg->GetBufferPtr();

    ret = SendMesg(pNewMsg, &s);
    if (ret)
    {
        STRACE("svr send notify msg to [%s] success!", ConvertIPtoString(s.m_dwRemoteIP));
    } else
    {
        STRACE("svr send notify msg to [%s] failed!", ConvertIPtoString(s.m_dwRemoteIP));
    }

    _SafeDeletePtr(pNewMsg);

    return ret;
}

bool CTcpServer::_ModuleRegCB(CMemItem *sheetData, CMemItem *soktData, CTCPSocket *psokt, void *userData)
{
    ASSERT(sheetData);

    reg_host hs;

    hs.shbuf = sheetData;
    hs.buff = soktData;
    hs.s    = *psokt;

    CTcpServer *io = (CTcpServer *)userData;

    io->m_ModuleLock.Lock();
    io->m_qModuleMsg.push(hs);
    io->m_ModuleLock.Unlock();

    io->m_ModuleSem.Post();

    return true;
}

OSTHD_FUNC_RET __stdcall CTcpServer::_VerifySvrNotify(LPVOID lpParam)
{
    CTcpServer *pIO = (CTcpServer *)lpParam;
    ASSERT(pIO);

    reg_host hs;
    while (true)
    {
        pIO->m_NotifySem.Wait();

        /// client multiple netcard register
        pIO->m_NotifyLock.Lock();

        if (pIO->m_qNotify.size() > 0)
        {
            hs =  pIO->m_qNotify.front();
            pIO->m_qNotify.pop();
        }

        pIO->m_NotifyLock.Unlock();

        ASSERT(hs.shbuf);
        ASSERT(hs.buff);
        if (NULL != hs.shbuf)
        {
            /// deal with register

            if (0 != pIO->RegHostCall(&hs.s))
            {
                STRACE("reg connect svr %s port[%d] failed\n", ConvertIPtoString(hs.s.m_dwRemoteIP).c_str(), hs.s.m_dwPeerPort);
                pIO->CloseClientSafeOutOfIOCP(hs.s);
            } else
            {
                STRACE("reg connect local [%s] to svr [%s] port [%d] success,then add socket to clt2svrpair \n",
                       ConvertIPtoString(hs.s.m_dwLocalIP).c_str(), ConvertIPtoString(hs.s.m_dwRemoteIP).c_str(), hs.s.m_dwPeerPort);

                u64 pairip = pIO->make_pair_ip(hs.s.m_dwLocalIP, hs.s.m_dwRemoteIP);

                pIO->AddClt2SvrPair(pairip, hs.s);
            }

            _SafeDeletePtr(hs.shbuf);
            _SafeDeletePtr(hs.buff);
        }
    }

#if (defined(_WIN32) || defined(_WIN64))
    return 0;
#endif

}
