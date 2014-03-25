
#include "socketpool.h"
#include "ShareLock.h"

CSocketPool *CSocketPool::sm_SocketPool = NULL;
CCritSec CSocketPool::sm_SocketPoolLock;


int CNetCard::AddConn(CTCPSocket& s)
{
	int r = -1;

	m_lock.Lock();
	if (m_mapConn.end() == m_mapConn.find(s.GetSocketHandle()))
	{
		m_mapConn.insert(make_pair(s.GetSocketHandle(), s));
		r = m_mapConn.size();
		printf("netcard add conn pair socket[%d] \n", (int)s);
	} else
	{
		///ASSERT(0);
		r = -1;
	}

	m_lock.Unlock();

	return r;
}

int CNetCard::EraseConn(CTCPSocket& s)
{
	int r  = -1;

	map<HANDLE, CTCPSocket>::iterator it;

	m_lock.Lock();

	if (m_mapConn.end() == (it =  m_mapConn.find(s.GetSocketHandle())))
	{
		r = -1;
		printf("CNetCard: can not find socket[%d] \n", (int)s);
                r = m_mapConn.size();
	} else
	{
		printf("CNetCard: erase socket[%d] \n", (int)s);
		m_mapConn.erase(it);
		r = m_mapConn.size();
	}

	m_lock.Unlock();


	return r;
}

CTCPSocket CNetCard::GetOneSocket()
{
	CTCPSocket ts;
	m_lock.Lock();
	if (m_mapConn.size() > 0)
	{
		ts = m_mapConn.begin()->second;
	}

	m_lock.Unlock();

	return ts;
}

int CProcId::AddCard(CNetCard& card)
{      
        int r = -1;
        DWORD ip = card.GetIp();
	if (m_mapIp.end() != m_mapIp.find(ip))
	{
		printf("add net card [%s] has existed \n", ConvertIPtoString(ip).c_str());
	} else
	{
		m_mapIp.insert(make_pair(card.GetIp(), card));
		r = m_mapIp.size();
	}
        return r;
}

int CProcId::AddConn(DWORD ip, CTCPSocket& s)
{
        int r = -1;
        map<DWORD, CNetCard>::iterator it;
        if (m_mapIp.end() != (it = m_mapIp.find(ip)))
	{
		r = it->second.AddConn(s);
		printf("CProcId AddConn m_mapIp find ip[%s] sock[%d] \n", ConvertIPtoString(ip).c_str(), (int)s);
	} else
	{
		CNetCard card(ip);
		card.AddConn(s);

		m_mapIp.insert(make_pair(ip, card));
		printf("CProcId:AddConn m_mapIp make pair ip[%s] sock[%d] \n", ConvertIPtoString(ip).c_str(), (int)s);
	}

	for (it = m_mapIp.begin(); it != m_mapIp.end(); it++)
	{
		printf("CProcId::AddConn trival m_mapIp [%s]\n", ConvertIPtoString(it->first).c_str());
	}

        return r;
}


int CHostInfo::AddCard(CNetCard& card, DWORD procid)
{
	int r = 0;
        map<DWORD,CProcId>::iterator it;
        ASSERT(procid);

	m_lock.Lock();

        if(m_mapProcId.end() != (it = m_mapProcId.find(procid)))
        {
            it->second.AddCard(card);
        }
        else
        {
            CProcId id(procid);

            id.AddCard(card);

            m_mapProcId.insert(make_pair(procid, id));
        }

	m_lock.Unlock();

	return r;
}

int CHostInfo::AddConn(DWORD ip, CTCPSocket& s, DWORD pid)
{
	int r = -1;
	
        map<DWORD, CProcId>::iterator im;

        m_lock.Lock();
        if ((im =m_mapProcId.find(pid)) != m_mapProcId.end())
        {
             im->second.AddConn(ip, s);
        }
        else
        {
            ASSERT(0);
        }
	
	printf("CHostInfo::AddConn m_mapProcid.size [%d]\n", (int)m_mapProcId.size());
	m_lock.Unlock();

	return r;
}

int CHostInfo::EraseConn(DWORD ip, CTCPSocket& s)
{
	int r = -1;
	map<DWORD, CNetCard>::iterator it;
        map<DWORD, CProcId>::iterator im;

	m_lock.Lock();
        for (im = m_mapProcId.begin(); im != m_mapProcId.end(); im++)
        {
            if (im->second.m_mapIp.end() != (it = im->second.m_mapIp.find(ip)))
            {
                    r = it->second.EraseConn(s);
                    /// net card conn size is zero  must delete net card
                    if (0 == r)
                    {
                            im->second.m_mapIp.erase(it);
                    }

                    r = im->second.m_mapIp.size();
                    if (r == 0)
                    {
                        printf("CHostInof erase all client[%s] process[%d] reg info________________________ \n",
                               ConvertIPtoString(ip).c_str(), im->first);

                        m_mapProcId.erase(im);

                        break;
                    }
            } else
            {
                    printf("CHostInfo::EraseConn can not find ip[%s] \n", ConvertIPtoString(ip).c_str());
            }
        }

        r =  m_mapProcId.size();
	printf("CHostInfo::EraseConn m_mapIp.size [%d]\n", (int)m_mapProcId.size());
	m_lock.Unlock();

	return r;
}

int CHostInfo::LoadFromMem(byte *buff, int len)
{
	ASSERT(buff);

	byte *head = buff;

	m_hostnamelen = *(u16 *)buff;
	buff += sizeof(u16);

	if (m_hostnamelen > 0)
	{
		m_hostname = (char *)buff;
	}

	buff += m_hostnamelen;

	m_identify = *(u64 *)buff;
	buff += sizeof(u64);

	m_mapsize = *(u8 *)buff;
	buff += sizeof(u8);

        ASSERT(m_mapsize == 1);
	for (int i = 0; i < m_mapsize; i++)
	{
                DWORD pid = *(DWORD*)buff;
                buff += sizeof(DWORD);

                CProcId pinf(pid);

                DWORD ipnum = *(DWORD*)buff;
                buff += sizeof(DWORD);

                for (int j = 0; j < ipnum; j++)
                {
                    DWORD ip = *(DWORD *)buff;
                    buff += sizeof(DWORD);

                    ASSERT(0 != ip);

                    CNetCard card(ip);

                    printf("CHostInfo::LoadFromMem Add host[%llx] ip[%s] \n", m_identify,ConvertIPtoString(ip).c_str());
                    if (pinf.m_mapIp.end() != pinf.m_mapIp.find(ip))
                    {
                            printf("client reg multiple netcard has exist!\n");

                            ASSERT(0);
                    } else
                    {
                            pinf.m_mapIp.insert(make_pair(ip, card));
                    }
                }

                m_mapProcId.insert(make_pair(pid,pinf));
	}

	ASSERT((buff - head) == len);

	return 0;
}

int CHostInfo::SaveToMem(CMemItem *pItem)
{
	ASSERT(pItem);

	byte *buff = pItem->GetBufferPtr();

	m_hostnamelen = m_hostname.size() + 1;
	*(u16 *)buff = m_hostnamelen;
	buff += sizeof(u16);

	if (m_hostnamelen > 0)
	{
		memcpy(buff, m_hostname.c_str(), m_hostnamelen);
	}

	buff += m_hostnamelen;

	*(u64 *)buff = m_identify;
	buff += sizeof(u64);

	m_mapsize = m_mapProcId.size();

	*(u8 *)buff = m_mapsize;
	buff += sizeof(u8);

        ASSERT(m_mapsize == 1);

	map<DWORD, CProcId>::iterator it;
        map<DWORD, CNetCard>::iterator im;

	for (it = m_mapProcId.begin(); it != m_mapProcId.end(); it++)
	{
                CProcId pid = it->second;

		*(DWORD *)buff = pid.m_dwProcId;
                buff += sizeof(DWORD);

                *(DWORD *)buff = pid.m_mapIp.size();
                buff += sizeof(DWORD);

               for( im = pid.m_mapIp.begin(); im != pid.m_mapIp.end(); im++)
               {
                   	*(DWORD *)buff = im->first;
                        buff += sizeof(DWORD);
               }
	}

	pItem->SetDataRealSize(buff - pItem->GetBufferPtr());

	return 0;
}

CTCPSocket CHostInfo::GetOneSocket(DWORD &pid)
{
	CNetCard nc;
	CTCPSocket ts;
        map<DWORD,CProcId>::iterator it;

	m_lock.Lock();
	if (m_mapProcId.size() > 0)
	{
            if ((it = m_mapProcId.find(pid)) != m_mapProcId.end())
            {
                    nc = it->second.m_mapIp.begin()->second;
                    ts = nc.GetOneSocket();
            }
	}

	m_lock.Unlock();

	return ts;
}

void CHostInfo::GetAllHostSocket(vector<CTCPSocket>& vec_out, vector<DWORD>& priority,DWORD &pid)
{
	int i = 0;
	CTCPSocket s;
	BOOL bFind = FALSE;
	map<DWORD, CNetCard>::iterator it;
        map<DWORD, CProcId>::iterator im;

	for (i = 0; i < (int)priority.size(); i++)
	{
		s.Reset();
		s = GetIpSocket(priority[i], pid);

		if (s.GetSocketHandle())
		{
			vec_out.push_back(s);
		}
	}

	m_lock.Lock();
        if ((im = m_mapProcId.find(pid)) != m_mapProcId.end())
        {

            for (it = im->second.m_mapIp.begin(); it != im->second.m_mapIp.end(); it++)
            {
                    bFind = FALSE;
                    s.Reset();

                    for (i = 0; i < (int)priority.size(); i++)
                    {
                            if (it->second.GetIp() == priority[i])
                            {
                                    bFind = TRUE;
                                    break;
                            }
                    }

                    if (!bFind)
                    {
                            s = it->second.GetOneSocket();
                            if (s.GetSocketHandle())
                            {
                                    vec_out.push_back(s);
                            }
                    }
            }
        }

	m_lock.Unlock();
}

void CHostInfo::GetAllHostSocket(vector<CTCPSocket>& vec_out,DWORD &pid)
{
	int i = 0;
	CTCPSocket s;
	map<DWORD, CNetCard>::iterator it;
        map<DWORD, CProcId>::iterator im;

	m_lock.Lock();

        if ((im = m_mapProcId.find(pid)) != m_mapProcId.end())
        {
            for (it = im->second.m_mapIp.begin(); it != im->second.m_mapIp.end(); it++)
            {
                    s.Reset();

                    s = it->second.GetOneSocket();

                    if (s.GetSocketHandle())
                    {
                            vec_out.push_back(s);
                    }
            }
        }

	m_lock.Unlock();
}

CTCPSocket CHostInfo::GetIpSocket(DWORD ip, DWORD& pid)
{
	CTCPSocket ts;
	CNetCard nc;

	map<DWORD, CNetCard>::iterator it;
        map<DWORD, CProcId>::iterator im;
	m_lock.Lock();
        if((im = m_mapProcId.find(pid)) != m_mapProcId.end())
        {
            if (im->second.m_mapIp.end() != (it = im->second.m_mapIp.find(ip)))
            {
                    nc = it->second;
                    ts = nc.GetOneSocket();
            }
        }

	m_lock.Unlock();

	return ts;
}

CSocketPool::CSocketPool()
{
	m_SockeChanneltMap.clear();
}

CSocketPool::~CSocketPool()
{
	//	lock
	m_SockeChanneltMapLock.OwnerLock();

	CSOCKET_CHANNELTMAP::iterator it = m_SockeChanneltMap.begin();

	while (it != m_SockeChanneltMap.end())
	{
		_SafeDeletePtr(it->second);
		m_SockeChanneltMap.erase(it++);
	}

	m_SockeChanneltMapLock.LeaveLock();
}

CSocketPool* CSocketPool::GetInstance()
{
	if (sm_SocketPool == NULL)
	{
		sm_SocketPoolLock.Lock();

		if (sm_SocketPool == NULL)
		{
			sm_SocketPool = new  CSocketPool;
		}

		sm_SocketPoolLock.Unlock();
	}

	return sm_SocketPool;
}

void CSocketPool::DestoryInstance()
{
	if (sm_SocketPool != NULL)
	{
		_SafeDeletePtr(sm_SocketPool);
	}
}

BOOL CSocketPool::AddSocket(CTCPSocket *sokt, CChannel *chl)
{
	BOOL br = FALSE;
	// lock
	m_SockeChanneltMapLock.OwnerLock();

	CSOCKET_CHANNELTMAP::iterator it = m_SockeChanneltMap.find(sokt->GetSocketHandle());

	if (it == m_SockeChanneltMap.end())
	{
		m_SockeChanneltMap[sokt->GetSocketHandle()] = chl;
		br = true;
	} else
	{
		br = false;
	}

	m_SockeChanneltMapLock.LeaveLock();

	return br;
}

BOOL CSocketPool::RemoveSocket(CTCPSocket *sokt)
{
	BOOL br = false;

	m_SockeChanneltMapLock.OwnerLock();

	CSOCKET_CHANNELTMAP::iterator it = m_SockeChanneltMap.find(sokt->GetSocketHandle());

	if (it != m_SockeChanneltMap.end())
	{
		m_SockeChanneltMap.erase(it);
		br = true;
	} else
	{
		br = false;
	}

	m_SockeChanneltMapLock.LeaveLock();

	return br;
}
