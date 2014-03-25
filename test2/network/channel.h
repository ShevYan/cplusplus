#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "netdefine.h"
#include "tcpsocket.h"
#include "ShareLock.h"

typedef map<string,string> IPMap;
typedef vector<SOCKET> SOCKETArray;

class CChannel
{
public:
	CChannel();
	~CChannel();

	SOCKET GetFreeSocket();
	BOOL AddSocket(SOCKET sokt, string str);
	BOOL RemoveSocket(SOCKET sokt);
	int GetSocketNum(){return m_SocketArray.size();}

	string GetFirstIp();
	string GetName();
	void SetName(string str);

private:

	IPMap m_strIpList;    
	SOCKETArray m_SocketArray;
	CShareLock  m_SocketArrayLock;
	
	CCritSec    m_HostNameLock;
	string	    m_HostName;    

	ULONG m_GetSocketTime;
};

#endif 

