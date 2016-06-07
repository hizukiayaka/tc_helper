#include "common.h"
#include "network.h"

#define _GNU_SOURCE		/* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>

bool network_ipv4_subnet
(uint8_t *if_name, uint32_t *subnet, uint32_t *netsize)
{
	struct ifaddrs *ifaddr, *ifa;
	struct sockaddr_in *ipv4_addr, *ipv4_mask;
	uint8_t host[NI_MAXHOST];
	uint8_t netmask[NI_MAXHOST];
	int32_t family, prefix;
	struct in_addr mask;
	int32_t ret;
	uint8_t *classes_netamsk[] = {
		"255.255.255.0",
		"255.255.0.0",
		"255.0.0.0",
		/* FIXME can't work with network larger than /8 */
	};

	if (NULL == if_name)
		return -1;

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;
		if (family == AF_INET) {
			if (0 != strcmp(ifa->ifa_name, if_name))
				continue;
			ipv4_addr = (struct sockaddr_in *)ifa->ifa_addr;
			ipv4_mask = (struct sockaddr_in *)ifa->ifa_netmask;

			inet_aton("255.255.255.255", &mask);
			*netsize = mask.s_addr -  ipv4_mask->sin_addr.s_addr;
			*netsize = ntohl(*netsize) + 1;
			
			prefix = ipv4_addr->sin_addr.s_addr
				& (ipv4_mask->sin_addr.s_addr);
	
			for (uint8_t i = 0; 
				i < sizeof(classes_netamsk) / sizeof(uint8_t *);
				i++)
			{
				inet_aton(classes_netamsk[i], &mask);
				*subnet = ipv4_mask->sin_addr.s_addr & 
					(~mask.s_addr);
				if (0 != *subnet) {
					/* always be network order !*/
					*subnet = prefix & (~mask.s_addr);
					*subnet /= pow(256, (3 - i));
					break;
				}
			}

			freeifaddrs(ifaddr);

			return true;

		}
	}
	freeifaddrs(ifaddr);

	return false;

}

bool network_ipv4_prefix(uint8_t *if_name, uint8_t **ipv4_prefix){
	struct ifaddrs *ifaddr, *ifa;
	struct in_addr addr;
	struct sockaddr_in *ipv4_addr;
	struct sockaddr_in *ipv4_mask;
	uint8_t host[NI_MAXHOST];
	uint8_t netmask[NI_MAXHOST];
	int32_t family;
	int32_t subnet;
	struct in_addr mask;
	int32_t ret;

	if (NULL == if_name)
		return false;
	if (NULL == ipv4_prefix)
		return false;

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;
		if (family == AF_INET) {
			ret = strcmp(ifa->ifa_name, if_name);
			if(0 != ret)
				continue;
			ipv4_addr = (struct sockaddr_in *)ifa->ifa_addr;
			ipv4_mask = (struct sockaddr_in *)ifa->ifa_netmask;

			addr.s_addr = ipv4_addr->sin_addr.s_addr 
				& ipv4_mask->sin_addr.s_addr;
			
			*ipv4_prefix = inet_ntoa(addr);
			freeifaddrs(ifaddr);

			return true;
		}
	}
	freeifaddrs(ifaddr);

	return false;

}
