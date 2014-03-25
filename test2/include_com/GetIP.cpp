#include<stdio.h>
#include<stdlib.h>
#include <stdbool.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<net/if.h>
#include<netinet/in.h>
#include<sys/types.h>
//#include<sys/netlink.h>
//#include<sys/rtnetlink.h>
#include <assert.h>
#include<errno.h>
#include<ifaddrs.h>
//#include<netpacket/packet.h>
#include <arpa/inet.h>
#include "GetIP.h"

void print_ip(struct ifaddrs *ifaddrs, vector <std::string> &IPList);

/** NOTE! caller must call freeifaddrs() after the use the pointer **/
int get_local_ip(struct ifaddrs **ifap, int family);

#define IS_UNSPEC(family) (family == AF_UNSPEC)
#define NO_ADDRS(ifa) (ifa->ifa_addr == NULL)
#define FAMILY_OK(ifa) (ifa->ifa_addr->sa_family == AF_INET ||\
                                ifa->ifa_addr->sa_family == AF_INET6)
#define IN_FAMILY(ifa, family) (ifa->ifa_addr->sa_family == family)
#define TEST_FLAG(ifa, flag) (ifa->ifa_flags | flag)
#define IS_LOOPBACK(ifa) (strncmp(ifa->ifa_name,"lo", 2) == 0)
#define move_ptr(ifa, tmp) do{  ifa->ifa_next = tmp->ifa_next;\
                                ifa->ifa_name = tmp->ifa_name;\
                                ifa->ifa_flags = tmp->ifa_flags;\
                                ifa->ifa_addr = tmp->ifa_addr;\
                                ifa->ifa_netmask = tmp->ifa_netmask;\
                                if (TEST_FLAG(ifa, IFF_BROADCAST)) \
                                        ifa->ifa_broadaddr = tmp->ifa_broadaddr;\
                                else if (TEST_FLAG(ifa, IFF_POINTOPOINT))\
                                        ifa->ifa_dstaddr = tmp->ifa_dstaddr;\
                                } while (0)


int CGetIP::GetIPV4(vector <std::string> &IPList)
{
	int r = -1;

	struct ifaddrs *ifa, *ifaddrs;
	struct ifaddrs *ifb, *ifc;
	IPList.clear();
	r = get_local_ip(&ifa, AF_INET);
	if (0 != r)
	{
		return r;
	}

	print_ip(ifa, IPList);

	return r;
}

int CGetIP::GetIPV6(vector <std::string> &IPList)
{
	int r = -1;
	
	IPList.clear();
	struct ifaddrs *ifa, *ifaddrs;
	struct ifaddrs *ifb, *ifc;

	r = get_local_ip(&ifa, AF_INET6);
	if (0 != r)
	{
		return r;
	}

	print_ip(ifa, IPList);

	return r;
}



//int main ()
//{
//    struct ifaddrs *ifa, *ifaddrs;
//    struct ifaddrs *ifb, *ifc;
//
//
//    printf("AF_INET\n");
//    get_local_ip(&ifa, AF_INET);
//    print_ip(ifa);
//    printf("AF_INET6\n");
//    get_local_ip(&ifb, AF_INET6);
//    print_ip(ifb);
//    printf("AF_UNSPEC\n");
//    get_local_ip(&ifc, AF_UNSPEC);
//    print_ip(ifc);
//
//
//}

void
print_ip(struct ifaddrs *ifaddrs, vector <std::string> &IPList)
{
	struct ifaddrs *ifa;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	char buf[INET6_ADDRSTRLEN];


	for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL) continue;
		if ((ifa->ifa_flags & IFF_UP) == 0)	continue;

		if (ifa->ifa_addr->sa_family == AF_INET)
		{
			sin = (struct sockaddr_in *)(ifa->ifa_addr);
			if (inet_ntop(ifa->ifa_addr->sa_family, (void *)&(sin->sin_addr), buf, sizeof(buf)) == NULL)
			{
				
				printf("%s: inet_ntop failed!\n", ifa->ifa_name);
			}
			else
			{
				std::string strTmp(buf);
				IPList.push_back(strTmp);
				//printf("%s: %s\n", ifa->ifa_name, buf);
			}
		}
		else if (ifa->ifa_addr->sa_family == AF_INET6)
		{
			sin6 = (struct sockaddr_in6 *)(ifa->ifa_addr);
			if (inet_ntop(ifa->ifa_addr->sa_family, (void *)&(sin6->sin6_addr), buf, sizeof(buf)) == NULL)
			{
				printf("%s: inet_ntop failed!\n", ifa->ifa_name);
			}
			else
			{
				std::string strTmp(buf);
				IPList.push_back(strTmp);
				//printf("%s: %s\n", ifa->ifa_name, buf);
			}
		}
	}
}

int get_local_ip(struct ifaddrs **ifap, int family)
{
	int n;
	bool change = 0;
	char *name;
	struct ifaddrs *ifa;
	struct ifaddrs *tmp;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	char buf[INET6_ADDRSTRLEN];

	n = getifaddrs(&ifa);
	if (n != 0)
		return -1;

	*ifap = ifa;
	for (ifa; (tmp = ifa) != NULL; ifa = ifa->ifa_next)
	{

		while (tmp && (IS_LOOPBACK(tmp) ||
					   NO_ADDRS(tmp) || (!TEST_FLAG(tmp, IFF_UP))||
					   (FAMILY_OK(tmp) && (!IN_FAMILY(tmp, family)) && (!IS_UNSPEC(family))) ||
					   !FAMILY_OK(tmp)))
		{
			change = true;
			tmp = tmp->ifa_next;
		}
		if (change)
		{
			if (tmp)
				move_ptr(ifa, tmp);
			else
				memset(ifa, 0, sizeof (struct ifaddrs));
			/** An alternative way is use these instead:
			ifa->ifa_next = NULL;
			ifa->ifa_name = NULL;
			ifa->ifa_addr = NULL;
			ifa->ifa_netmask = NULL;
			if (TEST_FLAG(ifa, IFF_BROADCAST))
					ifa->ifa_broadaddr = NULL;
			else if (TEST_FLAG(ifa, IFF_POINTOPOINT))
					ifa->ifa_dstaddr = NULL;
			**/

			change = false;
		}

	}

	/** I think even *ifap now is NULL after check, we should not free the space either.
		Since user who called this routine will call freeifaddrs() too, The space will be free safely . **/
	return 0;
}

