#ifndef __NETCONFIG_H__
#define __NETCONFIG_H__

#include "socketpool.h"

class CNetConfig
{
public:
    CNetConfig();
    ~CNetConfig();

public:

   BOOL SaveXml();
   BOOL ReadXml();

public:
    DWORD m_iocp_num;
    CHostInfo m_LocalIp;

    /// every host all ip can be indexed host class
    /// eg: a - host
    ///     b - host
    ///     c - host ,host only one
    map<DWORD,CHostInfo *> m_RemoteIp;
};
#endif
