#include "common.h"
#include "options.h"
#include "tc.h"

int main(int argc, char **argv)
{
	struct tc_if_config *data, *tmp;
	struct list_head *cur, *nex_tmp;
	LIST_HEAD(config_list);
	xcostc_load_config(&config_list);

	list_for_each_entry_safe(data, tmp, &config_list, list)
	{
		tc_init(data);
		tc_add_exception(data);
	}
	list_for_each_safe(cur, nex_tmp, &config_list)
	{
		/* FIXME free the sub-list*/
		free(cur);	
	}

	config_cleanup();
}
