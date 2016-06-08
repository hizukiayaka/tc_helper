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
		list_add_tail((struct list_head *)copy, 
				(struct list_head *)ptr);
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

/* If you don't duplicate the option strings returned from
 * API, you shouldn't call this function when you still use
 * those variables
 */
void config_cleanup() {
	uci_free_context(c);
}

uint8_t *config_find_option(struct uci_section *s, const uint8_t *name)
{
	return (uint8_t *)uci_lookup_option_string(c, s, name);	
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


static uint8_t config_load_mac_list(struct uci_section *s, uint8_t *name, void *list){
	uint8_t length = 0;
	struct uci_option *o;
	struct uci_element *e;

	o = uci_lookup_option(c, s, name);	
	if (NULL == o)
		return 0;

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

static bool load_tc_if_config(struct uci_section *s, void *data)
{
	struct tc_if_config *config = (struct tc_if_config *)data;
	uint8_t *ret = NULL;

	config->out_if = config_find_option(s, "outside_dev");
	config->in_if = config_find_option(s, "inside_dev");

	ret = config_find_option(s, "start");
	if(NULL != ret)
		sscanf(ret, "%d", &config->ip4_start);

	ret = config_find_option(s, "end");
	if(NULL != ret)
		sscanf(ret, "%d", &config->ip4_end);

	ret = config_find_option(s, "download_rate");
	if(NULL != ret)
		sscanf(ret, "%d", &config->download_rate);

	ret = config_find_option(s, "download_percent");
	if(NULL != ret)
		sscanf(ret, "%d", &config->download_percent);

	ret = config_find_option(s, "upload_rate");
	if(NULL != ret)
		sscanf(ret, "%d", &config->upload_rate);

	ret = config_find_option(s, "upload_percent");
	if(NULL != ret)
		sscanf(ret, "%d", &config->upload_percent);


	INIT_LIST_HEAD(&config->white_list);
	config_load_mac_list(s, "white_mac", &config->white_list);

	if (NULL == config->out_if || NULL == config->in_if)
		return false;

	if (0 >= config->download_rate)
		config->download_rate = 1000;
	if (0 >= config->upload_rate)
		config->upload_rate = 1000;
	if (0 >= config->download_percent)
		config->download_percent = 100;
	if (0 >= config->upload_percent)
		config->upload_percent = 100;

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

	uci_foreach_element(&p->sections, e)
	{
		s = uci_to_section(e);
		config = malloc(sizeof(struct tc_if_config));
		memset(config, 0, sizeof(struct tc_if_config));

		if (load_tc_if_config(s, config)) 
			list_add_tail((struct list_head *)config, (struct list_head *)data);
		else
			free(config); 
	}
}
