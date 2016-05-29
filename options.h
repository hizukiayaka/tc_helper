#ifndef _OPTIONS_H_
#define _OPTIONS_H_
#include "common.h"
#include "network.h"
#include <uci.h>

struct xcostc_mac {
	struct list_head list;
	struct ether_addr mac;
};

struct tc_if_config {
	struct list_head list;
	uint8_t *out_if;
	uint8_t *in_if;
	uint8_t ip4_start;
	uint8_t ip4_end;
	uint8_t upload_percent;
	uint8_t download_percent;
	uint32_t upload_rate;
	uint32_t download_rate;
	struct list_head white_list;
};

bool config_load_packge(uint8_t *package_name);

void config_cleanup();

void *config_find_interface(uint8_t *ifname); 

uint8_t *config_find_option(struct uci_section *s, const uint8_t *name);

uint8_t config_find_list(struct uci_section *s, uint8_t *name, uint8_t **list);

bool xcostc_load_config(void *data);
#endif
