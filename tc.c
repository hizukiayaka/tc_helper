#include "tc.h"
#include "network.h"
#include "options.h"

#define EXEC_STRING(template, args...) \
	do {uint8_t buf[200]; uint8_t ret;\
	snprintf(buf, sizeof(buf), template, ## args); \
	ret = system(buf); \
	PDEBUG("%s return %d\n", buf, ret); \
	} \
	while(0)

int8_t tc_init(void *data)
{
	struct tc_if_config *config = (struct tc_if_config *)data;
	uint32_t rate;
	uint16_t netmask;
	uint16_t subnet;
	uint8_t *subnet4_addr;

	/* FIXME caluate netmask and subnet here */

	/* The class id and handle can't be zero and 1 as the defualt class*/
	subnet = network_ipv4_subnet(config->in_if);
	subnet += 2;

	if (!network_ipv4_prefix(config->in_if, &subnet4_addr))
		return -1;

	EXEC_STRING("tc qdisc del dev %s root", config->out_if);
	EXEC_STRING("tc qdisc add dev %s root handle 1 hfsc default %d",
		config->out_if, subnet);

	rate = config->upload_rate * config->upload_percent / 100;
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

	EXEC_STRING("tc qdisc del dev %s root", config->in_if);
	EXEC_STRING("tc qdisc add dev %s root handle 1 hfsc default %d",
		config->in_if, subnet);

	rate = config->download_rate * config->download_percent / 100;
	EXEC_STRING("tc class add dev %s parent 1: classid 1:%d  hfsc sc rate %dkbit ul rate %dkbit",
		config->in_if, subnet, rate, rate);
	EXEC_STRING("tc qdisc add dev %s parent 1:%d handle %d: fq_codel",
		config->in_if, subnet, subnet);
	EXEC_STRING("tc filter add dev %s parent %d: handle %d protocol all flow map key dst addend -%s divisor 256",
		config->in_if, subnet, subnet, subnet4_addr);

	EXEC_STRING("tc class add dev %s  parent 1: classid 1:1 hfsc sc rate %dkbit ul rate %dkbit",
		config->in_if, config->download_rate, config->download_rate);
	EXEC_STRING("tc filter add dev %s parent 1: protocol arp prio 1 u32 match u32 0 0 flowid 1:1",
		config->in_if);

	return 0;

}

int8_t tc_add_exception(void *data)
{
	struct tc_if_config *config = (struct tc_if_config *)data;

	struct ether_addr *mac;
	struct xcostc_mac *ptr, *tmp;
	struct list_head *head;

	
	/* FIXME this way only work for the download traffic control,
	 * as we do that in the lan side
	 */
	list_for_each_entry_safe(ptr, tmp, &config->white_list, list) {
		mac = &(ptr->mac);
		EXEC_STRING(
		"tc filter add dev %s parent 1: protocol ip prio 3 u32 match u16 0x0800 0xFFFF at -2  \
		match u32 0x%X%X%X%X 0xFFFFFFFF at -12 match u16 0x%X%X 0xFFFF at -14 flowid 1:1",
		config->in_if, mac->ether_addr_octet[2], mac->ether_addr_octet[3],
		mac->ether_addr_octet[4], mac->ether_addr_octet[5], 
		mac->ether_addr_octet[0], mac->ether_addr_octet[1]);
	}

	return 0;

}
