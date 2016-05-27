#ifndef _NETWORK_H_
#define _NETWORK_H_
#include <netinet/ether.h>
#include <net/ethernet.h>

int32_t network_ipv4_subnet(char *if_name);

bool network_ipv4_prefix
(uint8_t *if_name, uint8_t **ipv4_prefix);
#endif
