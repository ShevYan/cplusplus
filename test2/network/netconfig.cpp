
#include "netconfig.h"
#if (defined(_WIN32) || defined(_WIN64))
#include "winerror.h"
#else
#include "win_error.h"
#endif
#include "Markup.h"

#if defined(UNICODE) || defined(_UNICODE)
#define tstring std::wstring 
#else
#define tstring std::string 
#endif

CNetConfig::CNetConfig()
{

}

CNetConfig::~CNetConfig()
{

}

BOOL CNetConfig::SaveXml()
{
   TCHAR s[64];
   CMarkup xml;
   DWORD dwIP = 0;
 /*
   xml.SetDoc(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"));
   xml.AddElem(_T("finet"));
   xml.IntoElem();
   

   memset(s, 0 , 64);
#if defined(UNICODE) || defined(_UNICODE)

   swprintf(s, _T("%d") ,m_iocp_num);
#else
;
   sprintf(s, "%d" ,m_iocp_num);
#endif
   xml.AddElem(_T("iocpnum"), s);


   xml.AddElem(_T("local"));
   xml.IntoElem();
   
   xml.AddElem(_T("host"));
   xml.IntoElem();
   
   map<DWORD,CNetCard>::iterator it;
   ASSERT(m_LocalIp.m_mapProcId.size() == 1);

   CProcId pid = m_LocalIp.m_mapProcId.begin().second;

   for (it = pid.m_mapIp.begin(); it != pid.m_mapIp.end(); it++)
   {
       tstring sip = ConvertIPtoString(it->first).c_str();
       xml.AddElem(_T("ip"), sip.c_str());
   }

   xml.OutOfElem();
   xml.OutOfElem();

   xml.AddElem(_T("remote"));
   xml.IntoElem();

   map<DWORD,CHostInfo *>::iterator im;

   for (im = m_RemoteIp.begin(); im != m_RemoteIp.end(); im++)
   {
       xml.AddElem(_T("host"));
       xml.IntoElem();

       map<DWORD, CNetCard>::iterator it;
       for (it = im->second->m_mapIp.begin(); it != im->second->m_mapIp.end(); it++)
       {
		   dwIP = it->first;
		   tstring sip = ConvertIPtoString(dwIP).c_str();
		   xml.AddElem(_T("ip"), sip.c_str());
       }

       xml.OutOfElem();
   }
   xml.OutOfElem();
   		
   xml.OutOfElem();		
   */     
   return xml.Save(_T("finet.xml"));
}

BOOL CNetConfig::ReadXml()
{
    CMarkup xml;
    tstring strTmp;

    if (!xml.Load(_T("finet.xml"))) 
    {
	printf("load finet xml failed \n");
	return FALSE;
    }

    if (xml.FindChildElem(_T("iocpnum")))
    {
	xml.IntoElem();
	strTmp = xml.GetData();
	if (!strTmp.empty())
	{
#if (defined(_WIN32) || defined(_WIN64))
		char tmp[64];
		memset(tmp,0,64);

		UTF16ConvertANSI((WCHAR*)strTmp.c_str(), strTmp.length(), tmp, 64);
	    m_iocp_num = atoi(tmp);
#else
	    m_iocp_num = atoi(strTmp.c_str());
#endif
	}
	xml.OutOfElem();
    }

    if (xml.FindChildElem(_T("local")))
    {
	xml.IntoElem();

	CHostInfo hs;

	if (xml.FindChildElem(_T("host")))
	{
	    xml.IntoElem();
	    while (xml.FindChildElem(_T("ip")))
	    {
			xml.IntoElem();
			
			strTmp = xml.GetData();
			CStdString aa = strTmp.c_str();
			CNetCard nc(ConvertIPtoDWORD(aa));
			hs.AddCard(nc,FiGetPid());

			xml.OutOfElem();
	    }
	    xml.OutOfElem();
	}
	
        CProcId cid = hs.m_mapProcId.begin()->second;
	hs.m_identify = cid.m_mapIp.begin()->first;

	m_LocalIp = hs;

	xml.OutOfElem();
    }

    if (xml.FindChildElem(_T("remote")))
    {
	xml.IntoElem();

	while (xml.FindChildElem(_T("host")))
	{
	    CHostInfo *hs = new CHostInfo;

	    xml.IntoElem();
	    while (xml.FindChildElem(_T("ip")))
	    {
		xml.IntoElem();
		
		strTmp = xml.GetData();
		CStdString bb = strTmp.c_str();

		CNetCard nc(ConvertIPtoDWORD(bb));
		hs->AddCard(nc, FiGetPid());

		xml.OutOfElem();

		/// all remote ip can be indexed by ip (ip to all ip (hostinfo class)) 
		m_RemoteIp.insert(make_pair(nc.GetIp(),hs));
	    }
	    xml.OutOfElem();

	    
	}
	xml.OutOfElem();
    }

    return TRUE;
}

