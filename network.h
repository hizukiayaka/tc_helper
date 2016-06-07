#ifndef _NETWORK_H_
#define _NETWORK_H_
#include <netinet/ether.h>
#include <net/ethernet.h>
bool network_ipv4_subnet
(uint8_t *if_name, uint32_t *subnet, uint32_t *netsize);

bool network_ipv4_prefix
(uint8_t *if_name, uint8_t **ipv4_prefix);
#endif
