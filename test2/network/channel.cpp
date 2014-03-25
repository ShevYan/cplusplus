#include "channel.h"

CChannel::CChannel()
{
    m_GetSocketTime = 0;
}

CChannel::~CChannel()
{

}

SOCKET CChannel::GetFreeSocket()
{
    m_SocketArrayLock.ShareLock();

    if (m_SocketArray.size() != 0)
    {
        SOCKET tem;
        tem = m_SocketArray.at(m_GetSocketTime++%m_SocketArray.size());
        m_SocketArrayLock.LeaveLock();
        return tem;
    }
    else
    {
        MASTRACE(("[fail]get free socket"));
    }

    m_SocketArrayLock.LeaveLock();

    return 0;
}

BOOL CChannel::AddSocket(SOCKET sokt, string strIp)
{
    BOOL br = false;

    m_SocketArrayLock.OwnerLock();
    m_SocketArray.push_back(sokt);
    m_SocketArrayLock.LeaveLock();

    br = true;

    IPMap::iterator it = m_strIpList.find(strIp);
    if (it == m_strIpList.end())
    {
        m_strIpList[strIp] = strIp;
    }
    return br;
}
BOOL CChannel::RemoveSocket(SOCKET sokt)
{
    BOOL br =false;

    m_SocketArrayLock.OwnerLock();

    SOCKETArray::iterator it = m_SocketArray.begin();

    while (it != m_SocketArray.end())
    {
        if (*it == sokt)
        {
            m_SocketArray.erase(it++);
            break;
        }
        else
        {
            it ++;
        }
    }

    m_SocketArrayLock.LeaveLock();

    return br;
}

string CChannel::GetFirstIp()
{
    IPMap::iterator it = m_strIpList.begin();
    if (it != m_strIpList.end())
    {
        return it->first;
    }
    return "";
}

string CChannel::GetName()
{
    return m_HostName;
}

void CChannel::SetName(string str)
{
    m_HostNameLock.Lock(); 
    m_HostName = str; 
    m_HostNameLock.Unlock();
}
