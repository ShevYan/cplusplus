
#include "c_socket.h"
#include <errno.h>

#include "portdef.h"

#if defined(_WIN32) || defined(_WIN64)
	#include "shlwapi.h"
	#include "ws2tcpip.h"
	#pragma comment(lib, "ws2_32.lib")
#endif

c_socket::c_socket()
{
    m_n_type = 0;
    m_n_af = 0;
    m_n_socket = SOCKET_NULL;
    m_n_last_error = 0;
}

c_socket::c_socket(const c_socket&sock)
{
    m_n_type = sock.m_n_type;
    m_n_af = sock.m_n_af;
    m_n_socket = sock.m_n_socket;
    m_n_last_error = 0;
}

c_socket::~c_socket()
{
    //if (m_n_socket != SOCKET_NULL)
    //{
    //	socket_close();

    //	m_n_socket == SOCKET_NULL;
    //}
}

bool 
c_socket::socket_create(const int n_af, const int n_type, const int n_protocol)
{
    m_n_socket = socket(n_af, n_type, n_protocol);

    if (INVALID_SOCKET == m_n_socket)
    {
        perror("c_socket::socket_create, socket()");

        m_n_last_error = errno;

        return false;
    }

    m_n_af = n_af;
    m_n_type = n_type; 

    return true;
}

const c_socket &
c_socket::operator=(const SOCKET &sock)
{
    m_n_socket = sock;

    return *this;
}

bool 
c_socket::socket_accept(c_socket &accept_socket, struct sockaddr *p_addr, int &n_addr_len)
{
    if (m_n_socket == SOCKET_NULL || m_n_type != SOCK_STREAM)
    {
        m_n_last_error = INVALID_SOCKET;

        return false;
    }
#if (defined(_WIN32) || defined(_WIN64))
	accept_socket = WSAAccept(m_n_socket, (sockaddr*)p_addr, (socklen_t*)&n_addr_len, NULL, 0);
#else
    accept_socket = accept(m_n_socket, (sockaddr*)p_addr, (socklen_t*)&n_addr_len);
#endif

    if (INVALID_SOCKET == accept_socket.get_socket())
    {
        m_n_last_error = errno;

        return false;
    }

    accept_socket.set_af(m_n_af);
    accept_socket.set_type(m_n_type);

    return true;
}

bool 
c_socket::socket_close()
{
    if (m_n_socket == SOCKET_NULL)
    {
        return true;
    }

    shutdown(m_n_socket, SD_BOTH);

    if (CLOSESOCKET(m_n_socket) != 0)
    {
        m_n_last_error = errno;

        return false;
    }

    return true;
}

bool 
c_socket::socket_set_opt(const int n_level, const int n_opt_name, const char *p_opt_val, const int n_opt_len)
{
    if (m_n_socket == SOCKET_NULL || p_opt_val == NULL || n_opt_len == 0)
    {
        return true;
    }

    if (setsockopt(m_n_socket, n_level, n_opt_name, p_opt_val, n_opt_len) != 0)
    {
        m_n_last_error = errno;

        return false;
    }

    return true;
}

bool 
c_socket::socket_connect(const struct sockaddr *p_addr)
{
    if (m_n_type != SOCK_STREAM)
    {
        return true;
    }

    if (m_n_socket == SOCKET_NULL || p_addr == NULL)
    {
        m_n_last_error = INVALID_SOCKET;

        return false;
    }

    if (connect(m_n_socket, p_addr, (socklen_t)sizeof(sockaddr_in)) != 0 )
    {
        m_n_last_error = errno;

        printf("_________connect error [%d]", m_n_last_error);
        return false;
    }

    return true;
}

bool 
c_socket::udp_socket_bind(const unsigned int n_ip, unsigned short &n_port)
{
    unsigned short i = 0;

    if (m_n_socket == SOCKET_NULL || n_port == 0)
    {
        m_n_last_error = INVALID_SOCKET;

        return false;
    }

    struct sockaddr_in svr_addr;
    struct sockaddr_in rand_addr;

    memset(&rand_addr,0,sizeof(sockaddr_in));
    memset(&svr_addr, 0, sizeof(sockaddr_in));

    svr_addr.sin_family = m_n_af;
    svr_addr.sin_addr.s_addr = n_ip;

    // rand port 
    //svr_addr.sin_port = htons(n_port);
    //svr_addr.sin_port = 0;
    // linux udp port range [5621,5621+40-1] 

    for (i = FIMAS_UDPBASE_PORT; i < FIMAS_UDPBASE_PORT + FIMAS_UDPPORT_RANGE; i++)
    {
        svr_addr.sin_port = htons(i);

        if (bind(m_n_socket, (sockaddr*)&svr_addr, (socklen_t)sizeof(sockaddr_in)) != 0)
        {
            m_n_last_error = errno;
            printf("bind UDP port [%d] failed [%s]\n",i, strerror(m_n_last_error));
            continue;
        }
        else
        {
            printf("bind UDP port [%d] success\n",i);
            break;
        }
    }

    if ( i >= FIMAS_UDPBASE_PORT + FIMAS_UDPPORT_RANGE)
    {
        return false;
    }

    int len = sizeof(rand_addr);
    if (getsockname(m_n_socket,(sockaddr*)&rand_addr,( socklen_t *)&len )!= 0)
    {
        m_n_last_error = errno;
        return false;
    }

    int nRankPort = 0;
    nRankPort = ntohs(rand_addr.sin_port);

    n_port = nRankPort;
    //SUPER_TRACE("return rank port [%d]",nRankPort);

    return true;
}

bool c_socket::tcp_bind(const unsigned int n_ip, unsigned short n_port)
{
    unsigned short i = 0;
    if (m_n_socket == SOCKET_NULL)
    {
	m_n_last_error = INVALID_SOCKET;

	return false;
    }

    struct sockaddr_in svr_addr;
    struct sockaddr_in rand_addr;

    memset(&rand_addr, 0, sizeof(sockaddr_in));
    memset(&svr_addr, 0, sizeof(sockaddr_in));

    svr_addr.sin_family = m_n_af;
    svr_addr.sin_addr.s_addr = n_ip;
    svr_addr.sin_port = htons(n_port);

    if (bind(m_n_socket, (sockaddr *)&svr_addr, (socklen_t)sizeof(sockaddr_in)) != 0)
    {
	m_n_last_error = errno;

	printf("bind error<%d>. please check network cable or profile.\n", errno);

	return false;
    }

    return true;
}

bool 
c_socket::tcp_socket_bind(const unsigned int n_ip, unsigned short &n_port)
{
    unsigned short i = 0;
    if (m_n_socket == SOCKET_NULL)
    {
        m_n_last_error = INVALID_SOCKET;

        return false;
    }

    struct sockaddr_in svr_addr;
    struct sockaddr_in rand_addr;

    memset(&rand_addr,0,sizeof(sockaddr_in));
    memset(&svr_addr, 0, sizeof(sockaddr_in));

    svr_addr.sin_family = m_n_af;
    svr_addr.sin_addr.s_addr = n_ip;

    // rand port 
   // for (i = FIMAS_TCPBASE_PORT; i < FIMAS_TCPBASE_PORT+FIMAS_TCPPORT_RANGE; i++)
    {
        svr_addr.sin_port = htons(n_port);

        if (bind(m_n_socket, (sockaddr*)&svr_addr, (socklen_t)sizeof(sockaddr_in)) != 0)
        {
            printf("bind TCP port [%d] failed [%s]\n",n_port, strerror(m_n_last_error));
            m_n_last_error = errno; 
            return false;
        }
        else
        {
            printf("bind TCP port [%d] success\n",n_port);
         //   break;
        }
    }

   // if (i >= FIMAS_TCPBASE_PORT+FIMAS_TCPPORT_RANGE)
   // {
   //     return false;
    //}

    int len = sizeof(rand_addr);
    if (getsockname(m_n_socket,(sockaddr*)&rand_addr,( socklen_t *)&len )!= 0)
    {
        m_n_last_error = errno;
        return false;
    }

    int nRankPort = 0;
    nRankPort = ntohs(rand_addr.sin_port);

    n_port = nRankPort;
    //SUPER_TRACE("return rank port [%d]",nRankPort);

    return true;
}

bool 
c_socket::socket_listen(const int n_wait_queue_size)
{
    if (m_n_type != SOCK_STREAM)
    {
        return true;
    }

    if (m_n_socket == SOCKET_NULL)
    {
        m_n_last_error = INVALID_SOCKET;

        return false;
    }

    if (listen(m_n_socket, n_wait_queue_size) != 0)
    {
        m_n_last_error = errno;

        return false;
    }

    return true;
}

int
c_socket::socket_send(const char *p_send_buf, const int n_send_len, const int n_flags)
{
    if (m_n_socket == SOCKET_NULL)
    {
	m_n_last_error = SOCKET_ERROR;

	return SOCKET_ERROR;
    }

    if (p_send_buf == NULL || n_send_len == 0)
    {
	return 0;
    }

    int n_has_send_byte = 0;
    int n_cur_send_byte = 0;

    while (n_has_send_byte < n_send_len)
    {
	n_cur_send_byte = send(m_n_socket, p_send_buf + n_has_send_byte, n_send_len - n_has_send_byte, n_flags);

	if (n_cur_send_byte <= 0)
	{
	    m_n_last_error = errno;
	    if (m_n_last_error == EAGAIN)
	    {
		printf("=========eagain send error[%d] n_cur_send_byte[%d]\n", m_n_last_error, n_cur_send_byte);
		fd_set readfds;

		FD_ZERO(&readfds);
		FD_SET(m_n_socket, &readfds);
		struct timeval time;
		time.tv_usec = 300;
		time.tv_sec  = 0;
		select(m_n_socket+1, &readfds, NULL, NULL, &time);
		continue;
	    }
	    printf("send error [%d] str[%s]\n", errno, strerror(errno));
	    return SOCKET_ERROR;
	}

	n_has_send_byte += n_cur_send_byte;
    }
    //printf("socket [%d] send msgsize[%d]\n",m_n_socket, n_has_send_byte);
    return n_has_send_byte;
}

int 
c_socket::socket_send_to(const char *p_send_buf, const int n_send_len, const int n_flags, const struct sockaddr_in *p_addr, const int p_addr_len)
{
    if (m_n_socket == SOCKET_NULL)
    {
        m_n_last_error = SOCKET_ERROR;

        return SOCKET_ERROR;
    }

    if (p_send_buf == NULL || n_send_len == 0)
    {
        return 0;
    }

    int n_has_send_byte = 0;
    int n_cur_send_byte = 0;

    while (n_has_send_byte < n_send_len)
    {
        n_cur_send_byte = sendto(m_n_socket, p_send_buf+n_has_send_byte, n_send_len-n_has_send_byte, n_flags, (sockaddr*)p_addr, (socklen_t)p_addr_len);

        if (n_cur_send_byte <= 0)
        {
            m_n_last_error = errno;

            return SOCKET_ERROR;
        }

        n_has_send_byte += n_cur_send_byte;
    }

    return n_has_send_byte;
}

int 
c_socket::socket_recv(char *p_recv_buf, const int n_recv_len, const int n_flags)
{
    if (m_n_socket == SOCKET_NULL)
    {
        m_n_last_error = SOCKET_ERROR;

        return SOCKET_ERROR;
    }

    if (p_recv_buf == NULL || n_recv_len == 0)
    {
        return 0;
    }

    return recv(m_n_socket, p_recv_buf, n_recv_len, n_flags);

}

int 
c_socket::socket_recv_from(char *p_recv_buf, const int n_recv_len, const int n_flags, struct sockaddr_in *p_addr, int *p_addr_len)
{
    if (m_n_socket == SOCKET_NULL)
    {
        m_n_last_error = SOCKET_ERROR;

        return SOCKET_ERROR;
    }

    if (p_recv_buf == NULL || n_recv_len == 0)
    {
        return 0;
    }

    return recvfrom(m_n_socket, p_recv_buf, n_recv_len, n_flags, (sockaddr*)p_addr, (socklen_t*)p_addr_len);
}

SOCKET 
c_socket::get_socket()
{
    return m_n_socket;
}

void 
c_socket::zero_socket()
{
    m_n_socket = SOCKET_NULL;
}

int 
c_socket::get_last_error()
{
    return m_n_last_error;
}

bool 
c_socket::socket_valid()
{
    return m_n_socket == SOCKET_NULL ? false : true;
}

bool 
c_socket::set_af(int n_af)
{
    if (AF_UNSPEC < n_af && n_af < AF_MAX)
    {
        m_n_af = n_af;
    }
    else
    {
        m_n_last_error = INVALID_PARAMETER;

        return false;
    }

    return true;
}

bool 
c_socket::set_type(int n_type)
{
    if (SOCK_STREAM < n_type || n_type < SOCK_SEQPACKET)
    {
        m_n_type = n_type;
    }
    else
    {
        m_n_last_error = INVALID_PARAMETER;

        return false;
    }

    return true;
}

bool 
c_socket::socket_getport(unsigned short &nport)
{
    struct sockaddr_in rand_addr;
    memset(&rand_addr,0,sizeof(sockaddr_in));

    if (m_n_socket == SOCKET_NULL)
    {
        return false;
    }

    int len = sizeof(rand_addr);
    if (getsockname(m_n_socket,(sockaddr*)&rand_addr,( socklen_t *)&len )!= 0)
    {
        m_n_last_error = errno;
        return false;
    }

    nport = ntohs(rand_addr.sin_port);

    return nport  == 0 ? false : true;
}
