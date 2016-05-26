#include "tc.h"
#include "network.h"

#define EXEC_STRING(template, args...) \
	do {uint8_t buf[200]; \
	snprintf(buf, sizeof(buf), template, ## args); \
	return system(buf); \
	} \
	while(0)

uint8_t tc_init(void *data)
{
	struct tc_if_config *config = (struct tc_if_config *)data;
	uint16_t rate;
	uint16_t netmask;
	uint16_t subnet;
	uint8_t *subnet4_addr;

	/* FIXME caluate netmask and subnet here */

	/* The class id and handle can't be zero and 1 as the defualt class*/
	subnet = network_ipv4_subnet(config->in_if);
	subnet += 2;

	if (0 != network_ipv4_prefix(config->in_if, subnet4_addr))
		return -1;

	EXEC_STRING("tc tc qdisc del dev %s root", config->out_if);
	EXEC_STRING("tc qdisc add dev %s root handle 1 hfsc default %d",
		config->out_if, subnet);

	rate = config->upload_rate * config->upload_persent;
	EXEC_STRING("tc class add dev %s parent 1: classid 1:%d  hfsc sc rate %dkbit ul rate %dkbit",
		config->out_if, subnet, rate, rate);
	EXEC_STRING("tc qdisc add dev %s parent 1:%d handle %d: fq_codel",
		config->out_if, subnet, subnet);
	EXEC_STRING("tc filter add dev %s parent %d: handle %d protocol all flow hash keys nfct-src divisor 256",
		config->out_if, subnet, subnet);

	EXEC_STRING("tc class add dev %s  parent 1: classid 1:1 hfsc sc rate %dkbit ul rate %dkbit",
		config->out_if, config->upload_rate, config->upload_rate);
	EXEC_STRING("tc filter add dev %s parent 1: protocol arp prio 2 u32 match u32 0 0 flowid 1:1",
		config->out_if);

	EXEC_STRING("tc tc qdisc del dev %s root", config->in_if);
	EXEC_STRING("tc qdisc add dev %s root handle 1 hfsc default %d",
		config->in_if, subnet);

	rate = config->download_rate * config->download_persent;
	EXEC_STRING("tc class add dev %s parent 1: classid 1:%d  hfsc sc rate %dkbit ul rate %dkbit",
		config->in_if, subnet, rate, rate);
	EXEC_STRING("tc qdisc add dev %s parent 1:%d handle %d: fq_codel",
		config->in_if, subnet, subnet);
	EXEC_STRING("tc filter add dev %s parent %d: handle %d protocol all flow map key dst-%s divisor 256",
		config->in_if, subnet, subnet4_addr);

	EXEC_STRING("tc class add dev %s  parent 1: classid 1:1 hfsc sc rate %dkbit ul rate %dkbit",
		config->in_if, config->download_rate, config->download_rate);
	EXEC_STRING("tc filter add dev %s parent 1: protocol arp prio 2 u32 match u32 0 0 flowid 1:1",
		config->in_if);

}

int8_t tc_add_expection(void *data, void *list)
{
	struct tc_if_config *config = (struct tc_if_config *)data;
}
