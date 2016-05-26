#include <uci.h>
#include "common.h"
#include "options.h"

static struct uci_context *c = NULL;
static struct uci_package *p = NULL;


static bool
put_value(void *ptr, void *val, int elem_size, bool is_list)
{
	void *copy;

	if (is_list)
	{
		copy = malloc(elem_size);

		if (!copy)
			return false;

		memcpy(copy, val, elem_size);
		list_add_tail((struct list_head *)copy, (struct list_head *)ptr);
		return true;
	}

	memcpy(ptr, val, elem_size);
	return false;
}

static bool
parse_enum(void *ptr, const char *val, const char **values, int min, int max)
{
	int i, l = strlen(val);

	if (l > 0)
	{
		for (i = 0; i <= (max - min); i++)
		{
			if (!strncasecmp(val, values[i], l))
			{
				*((int *)ptr) = min + i;
				return true;
			}
		}
	}

	return false;
}

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

bool parse_mac(void *ptr, const char *val, bool is_list)
{
	struct xcostc_mac addr = {};
	struct ether_addr *mac;

	if ((mac = ether_aton(val)) != NULL)
	{
		addr.mac = *mac;
		put_value(ptr, &addr, sizeof(addr), is_list);
		return true;
	}

	return false;
}


uint8_t config_load_mac_list(struct uci_section *s, uint8_t *name, void *list){
	uint8_t length = 0;
	struct uci_option *o;
	struct uci_element *e;

	o = uci_lookup_option(c, s, name);	

	if (UCI_TYPE_LIST == o->type)
	{
		uci_foreach_element(&o->v.list, e) 
		{
			if(parse_mac(list, e->name, true))
				length++;
		}
	}

	return length;
}

bool load_tc_if_config(struct uci_section *s, void *data)
{
	struct tc_if_config *config = (struct tc_if_config *)data;

	config->out_if = config_find_option(s, "outside_dev");
	config->in_if = config_find_option(s, "inside_dev");
	scanf(config_find_option(s, "start"), "%d", 
		config->ip4_start);
	scanf(config_find_option(s, "end"), "%d", 
		config->ip4_end);
	scanf(config_find_option(s, "download_rate"), "%d", 
		config->download_rate);
	scanf(config_find_option(s, "download_percent"), "%d", 
		config->download_percent);

	config_load_mac_list(s, "white_ip", config->ip4_white_list);

	if (NULL == config->out_if || NULL == config->in_if
	   || 0 >= config->ip4_start || 0 >= config->ip4_end)
		return false;

	if (0 >= config->download_rate)
		config->download_rate = 100;
	if (0 >= config->upload_rate)
		config->upload_rate = 100;

	return true;
}


bool xcostc_load_config(void *data)
{
	uint8_t ret;
	struct uci_section *s;
	struct uci_element *e;
	struct tc_if_config *config;

	if (!config_load_packge(PACKAGE_NAME))
		return false;

	/* FIXME the list is not implement yet, only the last
	 * section would be applied.
	 * */
	uci_foreach_element(&p->sections, e)
	{
		s = uci_to_section(e);

		config = malloc(sizeof(struct tc_if_config));
		memset(config, 0, sizeof(struct tc_if_config));
		if (load_tc_if_config(s, config))
			put_value(data, config, sizeof(config), true);
		else
			free(config);
	}
	config_cleanup();
}
