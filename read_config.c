#include <uci.h>
#include "common.h"
#include "tc.h"

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

uint8_t *config_find_option(struct uci_section *s, uint8_t *name)
{
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

bool load_tc_if_config(struct uci_section *s, void *data)
{
	struct tc_if_config *config = (struct tc_if_config *)data;

	config->out_if = config_find_option(s, "outside_dev");
	config->in_if = config_find_option(s, "inside_dev");
	config->ip4_start = config_find_option(s, "start");
	config->ip4_end = config_find_option(s, "end");
	scanf(config_find_option(s, "download_rate"), "%d", 
			config->download_rate);
	scanf(config_find_option(s, "download_persent"), "%d", 
			config->download_persent);

	config_find_list(s, "white_ip", config->ip4_white_list);
}


void xcostc_load_config()
{
	struct uci_section *s;
	struct uci_element *e;
	uint8_t *ret;

	config_load_packge(PACKAGE_NAME);

	/* FIXME the list is not implement yet, only the last
	 * section would be applied.
	 * */
	uci_foreach_element(&p->sections, e)
	{
		s = uci_to_section(e);

		printf("%s\n", ret);
	}
	config_cleanup();
}
