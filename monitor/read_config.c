#include "common.h"
#include <uci.h>

static struct uci_context *c = NULL;
static struct uci_package *p = NULL;

bool config_load_packge(uint8_t *package_name) {
	uint8_t **configs;

	c = uci_alloc_context();

	if (UCI_OK == uci_load(c, package_name, &p))
		return true;
	
	return false;

}

void config_cleanup() {
	uci_free_context(c);
}

void *config_find_interface(uint8_t *ifname) {
	struct uci_section *s = NULL;

	s = uci_lookup_section(c, p, ifname);

	return s;
}

uint8_t *config_find_option(struct uci_section *s, uint8_t *name){
	uint8_t *option;

	option = uci_lookup_option_string(c, s, name);	

	return option;
}

uint8_t config_find_list(struct uci_section *s, uint8_t *name, uint8_t **list){
	uint8_t length = 0;
	struct uci_option *o;
	struct uci_element *e;

	o = uci_lookup_option(c, s, name);	

	if (UCI_TYPE_LIST == o->type)
	{
		uci_foreach_element(&o->v.list, e) 
		{
			strncpy(*(list + length), e->name, B48INHEXSTRING - 1);
			(*(list + length))[B48INHEXSTRING] = '\0';
			length++;
			if (length >= CONFIG_MAX_LIST_LENGTH)
				break;
		}
	}

	return length;

}
