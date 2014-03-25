// TCPSocket.h: interface for the CTCPSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPSOCKET_H__1A5863F1_F68C_4CB5_86B7_9784538748DE__INCLUDED_)
#define AFX_TCPSOCKET_H__1A5863F1_F68C_4CB5_86B7_9784538748DE__INCLUDED_

#include "c_socket.h"
#include "netdefine.h"
#include "OSThread.h"

#if !(defined(_WIN32) || defined(_WIN64))
	#include <netinet/tcp.h>
#endif

class CTCPSocket;
typedef stPER_HANDLE_DATA <CTCPSocket> PER_HANDLE_DATA;
typedef stPER_HANDLE_DATA <CTCPSocket>  *LPPER_HANDLE_DATA;

typedef stPER_IO_OPERATION_DATA <CTCPSocket> PER_IO_OPERATION_DATA;
typedef stPER_IO_OPERATION_DATA <CTCPSocket> *LPPER_IO_OPERATION_DATA;

typedef bool (__stdcall *SOCKETCREATE_FUNC)(CTCPSocket *sokt, DWORD strip, void *userData);
typedef bool (__stdcall *SOCKETEXIT_FUNC)(CTCPSocket *sokt, void *userData);
typedef bool (__stdcall *SOCKETMSG_FUNC)(CMemItem* sheetData,CMemItem *soktData, CTCPSocket *psokt, void *userData);

class INetReceiver
{
public:
	// 接收数据处理
	virtual bool __stdcall ReceivePacket(CMemItem* sheetData, CMemItem* soktData, CTCPSocket* psokt, void* userData ) = 0;
	// 连接建立处理
	virtual bool __stdcall ConnectionNotify(CTCPSocket* sokt, DWORD strip, void* userData) = 0;
	// 连接断开处理
	virtual bool __stdcall BreakNotify(CTCPSocket* sokt, void* userData) = 0;
	//
	virtual ~INetReceiver(){};
};


struct way_cxt
{
	vector<DWORD> vecSrc;
	vector<DWORD> vecDst;

	/// whether polled 
	bool      polled;
	int	      src_pos;
	int	      des_pos;
	DWORD	  time_out;
	way_cxt()
		:polled(1),src_pos(0),des_pos(0),time_out(30000){}
};

struct call_cxt
{
	DWORD in_modid;
	char* in_buf1;
	int   in_buf1len;
	/// big wirte buf2
	char* in_buf2; 
	int   in_buf2len;

	/// expect svr deal with msg once
	u8	  in_once;

	way_cxt inway;

	/// the caller must delete out_buf;
	CMemItem* out_buf;
	/// such us read use outorgbuf will reduce memcpy times
	char* in_tunnelbuf;
	int   in_tunnelbuflen;

	call_cxt()
		:in_modid(0),in_buf1(NULL),in_buf1len(0),in_buf2(NULL),in_buf2len(0),in_once(1),out_buf(NULL),in_tunnelbuf(NULL),in_tunnelbuflen(0){}

	void set_call(DWORD mid, char* inbuf, int inbuflen,DWORD srcIP,DWORD dstIP)
	{
		in_modid   = mid;
		in_buf1    = inbuf;
		in_buf1len = inbuflen;

		inway.vecSrc.push_back(srcIP);
		inway.vecDst.push_back(dstIP);
	}

	CMemItem* get_reply_data()
	{
		return out_buf;
	}

	void free_reply_data()
	{
		_SafeDeletePtr(out_buf);
	}
};


#define MASTRACE

class CTCPSocket  
{
public:
	CTCPSocket();
	CTCPSocket(const CTCPSocket&sock);
	CTCPSocket(HANDLE hSocket);
	virtual ~CTCPSocket();
	const CTCPSocket &operator=(const SOCKET &s)
	{
		m_hSocket = s;
		return *this;
	}

	const CTCPSocket &operator=(const CTCPSocket &s)
	{
		m_hSocket     = s.m_hSocket;
		m_IsRev       = s.m_IsRev;
		m_dwPeerPort  = s.m_dwPeerPort;
		m_Error       = s.m_Error;
	   	m_uRemotePort = s.m_uRemotePort;
		m_dwRemoteIP  = s.m_dwRemoteIP;
		m_dwLocalIP   = s.m_dwLocalIP; 
		m_uHostType   = s.m_uHostType;
		m_bChecked    = s.m_bChecked;
		m_bAutoConn   = s.m_bAutoConn;
		return *this;
	}
public:
	void Create();
	void Close();
	void Cleanup();

public:
	void Bind(DWORD dwIp);
	void Bind(DWORD dwIp,DWORD &dwPort);
	int  Listen(DWORD dwBindIP, USHORT dwPort, DWORD dwQueueSize);
	int  Listen(USHORT dwPort, DWORD dwQueueSize);
	void Accept(CTCPSocket &sConnect);
	void Accept(CTCPSocket &sConnect,LPSOCKADDR psa,int &len);
	BOOL Connect(LPCSOCKADDR psa);

public:
	int Read(LPPER_IO_OPERATION_DATA lpPerIoData, DWORD dwFlags = 0);
	DWORD Write(LPPER_IO_OPERATION_DATA lpPerIoData, DWORD dwFlags = 0);
	DWORD WriteBlks(LPPER_IO_OPERATION_DATA lpPerIoData,LPPER_IO_OPERATION_DATA lpDataBuf, DWORD &dwStatus,DWORD dwFlags =0);

public:
        void RecConnInfo(DWORD dwLocalIP, DWORD dwRemoteIP, USHORT dwRemotePort); 
	BOOL SendMsg(CMemItem *pSendMsgBuf, BOOL isASASynWrite = FALSE);
	BOOL SendMsgBlks(CMemItem *pHead,PVOID pDataBuf,DWORD dwBufSize, DWORD &dwStatus,BOOL isASASynWrite = FALSE);
public:
	HANDLE GetSocketHandle() 
	{ 
		return (HANDLE)m_hSocket.get_socket(); 
	}

	VOID ZeroSocketHandle() 
	{ 
		m_hSocket.zero_socket();
	}
	
       // operator SOCK ()
       // {
       // 	return m_hSocket.get_socket();
       // }
	
	operator HANDLE () 
	{ 
		return (HANDLE)m_hSocket.get_socket(); 
	}

	operator int ()
	{
		return m_hSocket.get_socket(); 
	}

    string GetError()
    {
        return strerror(m_Error);
    }

	BOOL IsRev()
	{
	    return m_IsRev;
	}
public:
	void Reset();
public:
	int m_Error;
	u8 m_bChecked;			
	DWORD m_dwPeerPort; // add peer port
	u8 m_IsRev;       
	USHORT m_uRemotePort;
	DWORD m_dwRemoteIP;
	DWORD m_dwLocalIP;
	ULONG	m_uHostType;	
	bool  m_bAutoConn; // reconnect flag, svr accept socket m_bAutoConn = false, clt true
protected:
	c_socket m_hSocket;
};

#endif // !defined(AFX_TCPSOCKET_H__1A5863F1_F68C_4CB5_86B7_9784538748DE__INCLUDED_)
