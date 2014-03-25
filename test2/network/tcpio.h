#if !defined(__TCPIO_H__)
#define __TCPIO_H__

#include "netdefine.h"
#include "socketpool.h"

#if defined(_WIN32) || defined(_WIN64)
	#ifdef _TCPIODLL
		#define TCPIODLL_CLASS_DECL     __declspec(dllexport)
	#else
		#define TCPIODLL_CLASS_DECL     __declspec(dllimport)
	#endif
#else
	#include <pthread.h>
	#include <sys/epoll.h>
	#include <unistd.h>

	#define TCPIODLL_CLASS_DECL
#endif


/// linux ctcpserver is server and client
/// macosx only use tcpclient
class	CTcpServer;


class  TCPIODLL_CLASS_DECL CTcpIo   
{
public:

    CTcpIo();
    virtual ~CTcpIo();
	/// start server and client
	BOOL Start(USHORT uport);

	/// only start server
    BOOL StartServer(USHORT uport);

	/// only start client
    BOOL StartClient(USHORT svrport);

public:
	
	/// register module funtion
	BOOL RegisterModule(u32 modID, SOCKETCREATE_FUNC _create, SOCKETMSG_FUNC _msgrecv, SOCKETEXIT_FUNC _exit);
	BOOL RegisterModule(u32 modID, INetReceiver* pReceiver);
	BOOL UnRegisterModule(u32 modID);

	/// pair call and reply
	/// server or client call message and wait return 
	int Call(call_cxt& cxt);

	/// server reply client call message
	int Reply(CMemItem* sheet, char* buf1,int buf1len, char* buf2, int buf2len);
	
	/// server only transfer client message to each other
	int Route(call_cxt& cxt,CMemItem* sheet);

        /// local special
        int LocalCall(call_cxt& cxt);

        int RouteCall(call_cxt& cxt, CMemItem* sheet);
public:
private:
	CTcpServer *m_pImpl;
};

#endif 
