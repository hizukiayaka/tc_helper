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

int32_t network_ipv4_subnet(char *if_name){
	struct ifaddrs *ifaddr, *ifa;
	struct sockaddr_in *ipv4_addr;
	uint8_t host[NI_MAXHOST];
	uint8_t netmask[NI_MAXHOST];
	int32_t family;
	int32_t subnet;
	struct in_addr mask;
	int32_t ret;

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
			ret = strcmp(ifa->ifa_name, if_name);
			if(0 != ret)
				continue;
			/* FIXME only work for the /24 netmask */
			inet_aton("255.255.0.255", &mask);
			ipv4_addr = (struct sockaddr_in *)ifa->ifa_addr;
			
			subnet = ipv4_addr->sin_addr.s_addr 
				& (~mask.s_addr);

			subnet = subnet >> 16;

			freeifaddrs(ifaddr);

			return subnet;

		}
	}
	freeifaddrs(ifaddr);

	return -1;

}

bool network_ipv4_prefix(uint8_t *if_name, uint8_t *ipv4_prefix){
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

			
			ipv4_prefix = inet_ntoa(addr);
			freeifaddrs(ifaddr);

			return true;
		}
	}
	freeifaddrs(ifaddr);

	return false;

}
