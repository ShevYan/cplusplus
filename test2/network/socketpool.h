#ifndef __SOCKETPOOL_H__
#define __SOCKETPOOL_H__

#include "channel.h"
#include "netdefine.h"
#include "tcpsocket.h"
#pragma pack(push,1)


typedef map<HANDLE, CChannel*> CSOCKET_CHANNELTMAP;
struct connect_rec
{
    DWORD dwLocalIP;
    USHORT uRemotePort;
    DWORD dwRemoteIP;
    BOOL  bConned;

    connect_rec()
    :dwLocalIP(0),dwRemoteIP(0),uRemotePort(0),bConned(FALSE)
    {
    }

    connect_rec(DWORD lip,DWORD rip, USHORT up, BOOL bc)
    :dwLocalIP(lip),dwRemoteIP(rip),uRemotePort(up),bConned(bc)
    {
    }
};

class CNetCard
{
public:
    CNetCard()
    :m_ip(0){}

    CNetCard(DWORD ip)
    :m_ip(ip){}

    const CNetCard &operator=(const CNetCard &c)
    {
		m_ip = c.m_ip;
		m_mapConn = c.m_mapConn;
		return *this;
    }

    ~CNetCard(){};

public:
    /// return conn size, error -1
    int AddConn(CTCPSocket& s);
    int EraseConn(CTCPSocket& s);
    DWORD GetIp(){return m_ip;}
    CTCPSocket GetOneSocket();

private:
    DWORD m_ip;
    FiCritSec m_lock;
    map<HANDLE,CTCPSocket> m_mapConn;
};

struct ReqCxt
{
    ReqCxt()
    {
	used  = 0;
	modid    = 0;
	pktid    = 0;
	ev       = 0;
	replymsg = NULL;
	tunnelbuf = NULL;
	tunnelbuflen = 0;
    }

    u8  used;
    u32 modid;
    u64 pktid;
    FiEvent* ev;
    char* tunnelbuf;
    u32	  tunnelbuflen;
    
    CMemItem* replymsg ;/// Tsheet msg 
};

class CProcId
{
public:
    CProcId():m_dwProcId(0),m_ipnum(0){
    }
    CProcId(DWORD dw) :m_dwProcId(dw),m_ipnum(0){}

    ~CProcId(){}

    const CProcId &operator=(const CProcId &id)
    {
        m_dwProcId = id.m_dwProcId;
        m_ipnum   = id.m_ipnum;
        m_mapIp = id.m_mapIp;
        return *this;
    }

    int AddCard(CNetCard& card);
    int AddConn(DWORD ip, CTCPSocket& s);

public:
    u32 m_dwProcId;
    u32 m_ipnum;
    map<DWORD,CNetCard> m_mapIp;
};

class CHostInfo
{

public:

    CHostInfo()
    :m_hostnamelen(0),m_hostname(""),m_identify(0),m_mapsize(0){}

    CHostInfo(string hostname, u32 identify)
    :m_hostnamelen(0),m_hostname(hostname),m_identify(identify),m_mapsize(0){}

    const CHostInfo &operator=(const CHostInfo &h)
    {
		m_hostnamelen = h.m_hostnamelen;
		m_hostname = h.m_hostname;
		m_identify = h.m_identify;
		m_mapProcId    = h.m_mapProcId;	
		m_mapsize = h.m_mapsize;

		return *this;
    }

    ~CHostInfo(){}

public:

    int AddCard(CNetCard& card, DWORD procid);
    int AddConn(DWORD ip, CTCPSocket& s, DWORD pid);

    int EraseConn(DWORD ip,CTCPSocket& s);
    int LoadFromMem(byte* buff, int len);
    int SaveToMem(CMemItem* pItem);

    void GetAllHostSocket(vector<CTCPSocket>& vec_out, vector<DWORD>& priority, DWORD& pid);
	void GetAllHostSocket(vector<CTCPSocket>& vec_out, DWORD& pid);

    CTCPSocket GetOneSocket(DWORD& pid);
    CTCPSocket GetIpSocket(DWORD ip, DWORD& pid);

public:

    FiCritSec m_lock;
    u16    m_hostnamelen; /// = m_hostname.size() + 1;
    string m_hostname;
    u64  m_identify;
    u8	 m_mapsize;

    map<DWORD,CProcId> m_mapProcId;
    //map<DWORD,CNetCard> m_mapIp;
};

class CSocketPool
{

public:

    CSocketPool();
    ~CSocketPool();
    static CSocketPool* GetInstance();
    static void DestoryInstance();

    /// Add Remove
    BOOL AddSocket(CTCPSocket *sokt, CChannel *chl);
    BOOL RemoveSocket(CTCPSocket *sokt);

private:
    CSOCKET_CHANNELTMAP m_SockeChanneltMap;
    CShareLock m_SockeChanneltMapLock;

    static CSocketPool *sm_SocketPool;
    static CCritSec sm_SocketPoolLock;
}; 

#pragma pack(pop)

#endif 

