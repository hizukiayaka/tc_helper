#ifndef _READ_CONFIG_H_
#define _READ_CONFIG_H_
#include "common.h"
#include <uci.h>

bool config_load_packge(uint8_t *package_name);

void config_cleanup();

void *config_find_interface(uint8_t *ifname); 

uint8_t *config_find_option(struct uci_section *s, uint8_t *name);

uint8_t config_find_list(struct uci_section *s, uint8_t *name, uint8_t **list);

#endif
