#ifndef _UDP_SOCKET_H_
#define _UDP_SOCKET_H_

#if !(defined(_WIN32) || defined(_WIN64))
	#include <netinet/udp.h>
#endif

#include "c_socket.h"

#define	IOC_VENDOR			0x18000000
#define	_WSAIOW(x,y)		(IOC_IN|(x)|(y))
#define	SIO_UDP_CONNRESET	_WSAIOW(IOC_VENDOR,12)

class CUDPSocket;

typedef stPER_HANDLE_DATA <CUDPSocket> UDPPER_HANDLE_DATA;
typedef stPER_HANDLE_DATA <CUDPSocket>  *LPUDPPER_HANDLE_DATA;

typedef stPER_IO_OPERATION_DATA <CUDPSocket> UDPPER_IO_OPERATION_DATA;
typedef stPER_IO_OPERATION_DATA <CUDPSocket>  *LPUDPPER_IO_OPERATION_DATA;

class CUDPSocket
{
public:
    CUDPSocket();
    CUDPSocket(HANDLE hSocket);
    virtual ~CUDPSocket();

    int Create(DWORD &dwPort);
    void Close();
    void Cleanup();

    long Read(LPUDPPER_IO_OPERATION_DATA lpPerIoData, DWORD dwFlags = 0);
    long Write(LPCSOCKADDR lpSockAddr, LPUDPPER_IO_OPERATION_DATA lpPerIoData, DWORD dwFlags = 0);

    HANDLE GetSocketHandle() { return(HANDLE)m_hSocket.get_socket();;}

    const CUDPSocket &operator=(const SOCKET &s)
    {
        m_hSocket = s;
        return *this;
    }
    operator SOCKET () { return m_hSocket.get_socket();}
    operator HANDLE () { return(HANDLE)m_hSocket.get_socket();;}


protected:

    c_socket m_hSocket;
};

#endif // _UDP_SOCKET_H_

