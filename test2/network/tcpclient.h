
#ifndef __TCPCLINET_H__
#define __TCPCLINET_H__

#include "netdefine.h"

#include "tcpsocket.h"
#include "socketpool.h"
#include "FiMsgThread.h"

typedef map<ULONG, LPPER_IO_OPERATION_DATA> CSocket_IODATA;
typedef map<DWORD, int> CIp_SocketNumMap;
typedef map<HANDLE, CCritSec *>  CSocket_CritSecMap;




class CTcpClient  
{
public:
    CTcpClient(USHORT nPort );
    virtual ~CTcpClient();

    static CTcpClient* GetInstance(USHORT port );
    static void DestoryInstance();

    BOOL Init(
             SOCKETCREATE_FUNC crateCB, 
             void *createUserData, 
             SOCKETEXIT_FUNC exitCB, 
             void *exitUserData, 
             SOCKETMSG_FUNC msgCB, 
             void *msgUserData,
             DWORD m_strBindIP
             );

    VOID Init(
             SOCKETCREATE_FUNC createCB, 
             void *createUserData,
             SOCKETEXIT_FUNC exitCB, 
             void *exitUserData,
             SOCKETMSG_FUNC msgCB, 
             void *msgUserData
             );

    BOOL BindNetCard(DWORD dwCurNetCard);
    BOOL StartWork();

    BOOL CreateCompletionThread();
    BOOL CreateConnectThread();

    void CloseClient(LPPER_HANDLE_DATA lpPerHandleData, LPPER_IO_OPERATION_DATA lpPerIOConText );   
    void CloseClientSafeOutOfIOCP(HANDLE sokt);
    void CloseAllSocket();

    BOOL CreateListenThread();

    HANDLE ConnSvr(DWORD dwLocalIp, DWORD RemoteIp, USHORT nPort, BOOL bIsCtrlSvr=FALSE);

    SOCKETCREATE_FUNC m_SocketCreateCB;
    SOCKETEXIT_FUNC   m_SocketExitCB;
    SOCKETMSG_FUNC    m_SocketMsgCB;
    void                 *m_CreateUserDATA; 
    void                 *m_ExitUserDATA;   
    void                 *m_MsgUserDATA;    
    BOOL                 m_bListenExit;        


    USHORT GetPort(){return m_nPort;}
    int GetSocketNum(DWORD strIp);
    int RemoveSocektNum(DWORD strIp);   

    static void * __stdcall _TCPListenThread(LPVOID lpParam);   

    static void * __stdcall _MultiNetCardTCPListenThread(LPVOID lpParam);   

    static void * __stdcall _TCPCompletionThread(LPVOID lpParam);
    static void * __stdcall  _TCPAutoConnect(LPVOID lpParam);

    void DeleteIOData( LPPER_IO_OPERATION_DATA lpPerIOConText );


    LPPER_IO_OPERATION_DATA CreateInfo(CTCPSocket &tcpSocket);
    LPPER_IO_OPERATION_DATA AcceptConnection(CTCPSocket &sConn);
    LPPER_IO_OPERATION_DATA AssociateSocketWithCompletionPort(CTCPSocket tcpSocket);


    BOOL SendMesg(CMemItem *temBuf, CTCPSocket *sokt, BOOL isASASynWrite = FALSE);


    BOOL SendMsgBlks(CMemItem *temBuf, PVOID pDataBuf,DWORD dwBufSize,CTCPSocket *sokt, DWORD &dwStatus,BOOL isASASynWrite = FALSE);

    BOOL GetInitError(){return m_state;}

    void AddAutoConn(CTCPSocket &ws, BOOL bConn);

    CCritSec* AddSocketLock(HANDLE s);
    CCritSec* GetSocketLock(HANDLE s);
    void       RemoveSocketLock(HANDLE s);

    DWORD m_dwBindIP;
private:

    CTCPSocket m_cListenSocket;

    map<DWORD, CTCPSocket> m_sListenSocketMap;
    list<pthread_t> m_multiNetCardListenThread;
    //////////////////////////////±äÁ¿////////////////////////////////////////////
    CIp_SocketNumMap m_IPMap;
    CCritSec m_IPMapLock;

    USHORT m_nPort;

    pthread_t m_ListenThread;
    int    m_CPUNumber;  

    CSocket_IODATA m_IODataMap;
    CCritSec  m_IODataMapLock; 

    CSocket_CritSecMap m_socketCritSec;
    CShareLock         m_socketCritSecLock; 

    CCritSec m_TCPSerCritSec;

    map<pthread_t, int> m_SelectModeThreadActor;
    map<pthread_t, WAITHANDLE> m_SelectModeThreadEvent;

    CCritSec m_ThreadHoldSizeLock;
    map<pthread_t, DWORD> m_SelectModeThreadHold;

    BOOL m_state;

    FiEvent m_EvConn;
    CCritSec m_lkconn;
    vector<connect_rec> m_vecconn; 

    static CTcpClient* sm_TCPServer;
    static CTcpClient* m_This;
};
#endif 
