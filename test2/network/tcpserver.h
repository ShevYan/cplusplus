#if !defined(__TCPSERVER_H__)
#define __TCPSERVER_H__

#include "netdefine.h"
#include "socketpool.h"
#include "module_reg.h"
#include "msgsite.h"
#include "netconfig.h"

#include<list>

#if !(defined(_WIN32) || defined(_WIN64))
	#include<pthread.h>
	#include <sys/epoll.h>
	#include <unistd.h>
#endif
/// linux ctcpserver is server and client
/// macosx only use tcpclient


typedef std::map<ULONG, LPPER_IO_OPERATION_DATA> CSocket_IODATA;
typedef std::map<u32, CHostInfo*> CHostMap;
typedef std::map<ULONG, CCritSec *>  CSocket_CritSecMap;
enum net_mode
{
    TCP_SVR = 0,
    TCP_CLT = 1,
	TCP_SVR_CLT = 2
};


class CTcpServer;
struct ListenThdParam
{
    CTCPSocket *ls;
    CTcpServer *ptr;
};



struct io_func
{
    SOCKETCREATE_FUNC createCB;
    SOCKETEXIT_FUNC   exitCB;
    SOCKETMSG_FUNC    rcvCB;
    void* 	      userdata;
};

struct io_conn_ip
{
    DWORD dwLocBindIP;
    DWORD dwLocPort;
    DWORD dwRemoteIP;
    USHORT uRemotePort;
    BOOL  bAutoConn;
};

struct reg_host
{
   CMemItem* shbuf;
   CMemItem* buff;
   CTCPSocket s;

   reg_host():shbuf(NULL),buff(NULL){}

   const reg_host &operator=(const reg_host h)
   { 
       shbuf = h.shbuf;
       buff = h.buff;
       s = h.s;

	   return *this;
   }
};

class CTcpServer  
{
public:
    CTcpServer();
    CTcpServer(ULONG port );
    virtual ~CTcpServer();


    BOOL Init(SOCKETCREATE_FUNC crateCB, void *createUserData,
              SOCKETEXIT_FUNC exitCB, void *exitUserData,
              SOCKETMSG_FUNC msgCB, void *msgUserData, net_mode mode);

    BOOL CreateCompletionThread();

    BOOL RegisterModule(u32 modID, SOCKETCREATE_FUNC _create, SOCKETMSG_FUNC _msgrecv, SOCKETEXIT_FUNC _exit);
	BOOL RegisterModule(u32 modID, INetReceiver* pReceiver);
	BOOL RegisterModule(CModuleInfo* mod);
    BOOL UnRegisterModule(u32 modID);
    CModuleInfo *GetModule(DWORD modid);

    void DisableNet()   { m_enable = 0; }
    void EnableNet()    { m_enable = 1; }
    BOOL GetNetStatus() { return (BOOL)m_enable; }
     
    void CloseClient(LPPER_HANDLE_DATA lpPerHandleData, LPPER_IO_OPERATION_DATA lpPerIOConText ); 

    void CloseAllSocket();
    void CloseClientSafeOutOfIOCP(CTCPSocket &sokt);

    BOOL CreateListenThread(DWORD dwBindIp);

    HANDLE ConnSvr(DWORD dwLocalIp, DWORD RemoteIp, USHORT nPort, BOOL bIsCtrlSvr=FALSE);

    SOCKETCREATE_FUNC m_SocketCreateCB;
    SOCKETEXIT_FUNC   m_SocketExitCB;
    SOCKETMSG_FUNC    m_SocketMsgCB;
    void                 *m_CreateUserDATA; 
    void                 *m_ExitUserDATA;   
    void                 *m_MsgUserDATA;    
    BOOL                 m_bListenExit;        

    void SetPort(USHORT port) {m_Port = port ;}
    ULONG GetPort(){return m_Port;}

    static OSTHD_FUNC_RET __stdcall _TCPListenThread(LPVOID lpParam);    

    static OSTHD_FUNC_RET __stdcall _TCPCompletionThread(LPVOID lpParam);
	static OSTHD_FUNC_RET __stdcall _VerifySvrNotify(LPVOID lpParam);


    void DeleteIOData( LPPER_IO_OPERATION_DATA lpPerIOConText );

    LPPER_IO_OPERATION_DATA CreateInfo(CTCPSocket &tcpSocket);
    LPPER_IO_OPERATION_DATA AcceptConnection(CTCPSocket* ls,CTCPSocket &sConn);
    LPPER_IO_OPERATION_DATA AssociateSocketWithCompletionPort(CTCPSocket tcpSocket);

    BOOL SendMesg(CMemItem *temBuf, CTCPSocket *sokt, BOOL isASASynWrite = FALSE);
    BOOL SendMsgBlks(CMemItem *temBuf, PVOID pDataBuf,DWORD dwBufSize,CTCPSocket *sokt, DWORD &dwStatus,BOOL isASASynWrite = FALSE);
    BOOL GetInitError(){return m_state;}

    /// auto conn info
    void AddAutoConn(CTCPSocket &ws, BOOL bConn);

    void SetAutoConnFlag(BOOL bAutoConn){m_bAutoConn = bAutoConn;}

    CCritSec* AddSocketLock(int s);
    CCritSec* GetSocketLock(int s);
    void       RemoveSocketLock(int s);
    BOOL SocketLock(int s);
    BOOL SocketUnLock(int s);

    void EraseHostConn(CTCPSocket &t);

	void AddClt2SvrPair(u64 &index, CTCPSocket &t);
	void EraseClt2SvrPair(CTCPSocket &t);

 ///////////////////////////////////////////////////////////////////////   
    /// Send: send to network then return
    BOOL Send(CMemItem *temBuf, HANDLE sock);

    /// Call: send msg to network then wait mas 
    /// such as read write meta call
    int Call(call_cxt& cxt);
    int Reply(CMemItem* sheet, char* buf1,int buf1len, char* buf2, int buf2len);
	int Route(call_cxt& cxt, CMemItem* sheet);

    /// local call 
    int LocalCall(call_cxt& cxt);

    int RouteCall(call_cxt& cxt, CMemItem* sheet);
    /// call back all modules
    void _createcb(CTCPSocket *sokt, DWORD strip, void *userData);
    void _exitcb(CTCPSocket *sokt, void *userData);  // call back all modules
    static bool __stdcall _recvcb(CMemItem* sheetData, CMemItem *soktData, CTCPSocket *psokt, void *userData);
	static bool __stdcall _verifycb(CMemItem* sheetData, CMemItem *soktData, CTCPSocket *psokt, void *userData);

    static bool __stdcall _ModuleRegCB(CMemItem* sheetData, CMemItem *soktData, CTCPSocket *psokt, void *userData);
    static OSTHD_FUNC_RET __stdcall _ClientRegisterThd(LPVOID lpParam);
    
    int CreateRegThd();
	int CreateVerifyThd();
private:

    int CallSend(CTCPSocket* s, CMemItem *pMsg, char* buf2, int buf2len);

    void GetAllCltSocket(Tsheet* sh,vector<CTCPSocket>& vec);
    BOOL SendVerifyMsg(CTCPSocket &s);  /// svr frist send  verify msg to client ,then client sent reg msg to svr

    CTCPSocket GetRightSvr(call_cxt& cxt);
    CHostInfo* GetSvrHost(DWORD ip);

    CMemItem*  GenSheet(u8 sheettype,call_cxt& cxt);
    CMemItem*  GetReplySheet(Tsheet *src,char* buf1, int buf1len, char* buf2,int buf2len);
	CMemItem*  GetRouteSheet(Tsheet *src,char* buf, int buflen);
    BOOL CreateConnectThread();
    static OSTHD_FUNC_RET __stdcall  _TCPAutoConnect(LPVOID lpParam);

    void AddMapIdentify(DWORD &dwip, DWORD& identify);
    DWORD GetMapIdentify(DWORD &dwip);
    void EraseMapIdentify(DWORD &dwip);

    int RegHostCall(CTCPSocket *s);
    int RegHostReply(CMemItem* sheet,CTCPSocket *s);

    u64 make_pair_ip(DWORD L, DWORD R);
	void GetOriginIP(CMemItem* sheet, vector<DWORD> &org);

	OSTHREAD_H GetCompletionPort();

protected:
	bool m_bStarted;
    u8  	m_enable;
    list<CTCPSocket*> m_arrListenSocket;


    CHostMap m_mapHost;
    CCritSec m_mapHostLock;
    /// client  all ip map to client identify ip
    map<u32,u32> m_mapIdentify;

    CShareLock m_mapModlock;
    map<DWORD,CModuleInfo*> m_mapMod;

    ULONG m_Port;
	OSTHREAD_H m_CompletionPort;
    OSTHREAD_H m_ListenThread;

    int    m_CPUNumber;  

    CSocket_IODATA m_IODataMap;
    CShareLock  m_IODataMapLock; 

    CSocket_CritSecMap m_socketCritSec;
    CShareLock         m_socketCritSecLock; 


    net_mode m_state;

    FiSem m_EvConn;
    CCritSec m_lkconn;
    vector<connect_rec> m_vecconn; 

    int m_fdEpoll;
    BOOL m_bAutoConn;

    CMsgSite *m_pmsg_site;

    CShareLock m_mapClt2SvrConnLock;
    map<u64,CTCPSocket> m_mapClt2SvrConn;
    CShareLock m_mapConfigSvrHostLock;
    CNetConfig m_cfg;/// just only client config all svr ip


    u32 m_identify; /// client identify select one ip mac address identify one client;

    FiCritSec    m_ModuleLock;
    queue<reg_host> m_qModuleMsg;
    FiSem	 m_ModuleSem;

	FiCritSec  m_NotifyLock;
	queue<reg_host> m_qNotify;
	FiSem 	   m_NotifySem;

    CModuleInfo *m_Mod;

#if (defined(__APPLE__) || defined(macintosh))
	map<pthread_t, int> m_SelectModeThreadActor;

	CCritSec m_ThreadHoldSizeLock;
	map<pthread_t, DWORD> m_SelectModeThreadHold;
#endif

private:
    FiEvent m_exit_ev;
};

#endif 
