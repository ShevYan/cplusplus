
#ifndef __C_SOCKET_H_
#define __C_SOCKET_H_

#include "BaseType.h"

#include <stdio.h>
#include <stdlib.h>

#if !(defined(_WIN32) || defined(_WIN64))
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "netdefine.h"

using namespace std;

#if !(defined(_WIN32) || defined(_WIN64))
typedef uintptr_t SOCKET;
#endif

#define SOCKET_NULL			     0

#define SOCKET_TYPE_NOT_MATCH   -2
#define INVALID_PARAMETER       -3

#define SD_RECEIVE				0x00
#define SD_SEND					0x01
#define	SD_BOTH					0x02

typedef struct sockaddr *LPSOCKADDR;
typedef const struct sockaddr *LPCSOCKADDR;

class c_socket
{
public:
    c_socket();
    virtual ~c_socket();
    c_socket(const c_socket&sock);

public:
    bool socket_create(const int n_af, const int n_type, const int n_protocol);
    bool socket_accept(c_socket &accept_socket, struct sockaddr *p_addr, int &n_addr_len);
    bool socket_getport(unsigned short &nport);
public:
    bool socket_close();
    bool socket_set_opt(const int n_level, const int n_opt_name, const char *p_opt_val, const int n_opt_len);

public:
    bool socket_connect(const struct sockaddr *p_addr);
    bool udp_socket_bind(const unsigned int n_ip, unsigned short &n_port);
    
    bool tcp_bind(const unsigned int n_ip, unsigned short n_port);
    bool tcp_socket_bind(const unsigned int n_ip, unsigned short &n_port);
    bool socket_listen(const int n_wait_queue_size);

public:
    int socket_send(const char *p_send_buf, const int n_send_len, const int n_flags);
    int socket_send_to(const char *p_send_buf, const int n_send_len, const int n_flags, const struct sockaddr_in *p_addr, const int p_addr_len);
    int socket_recv(char *p_recv_buf, const int n_recv_len, const int n_flags);
    int socket_recv_from(char *p_recv_buf, const int n_recv_len, const int n_flags, struct sockaddr_in *p_addr, int *p_addr_len);

public: 
    const c_socket &operator=(const SOCKET &sock);

public:
    int get_last_error();
    SOCKET get_socket();
    void zero_socket();
    bool socket_valid();

public:
    bool set_af(int n_af = AF_INET);
    bool set_type(int n_type = SOCK_STREAM);

private:
    int     m_n_type;
    int     m_n_af;
    int     m_n_last_error;
    SOCKET  m_n_socket;
};

#endif
