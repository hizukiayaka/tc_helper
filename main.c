#include "common.h"
#include "options.h"
#include "tc.h"
#include <unistd.h>

void start()
{
	struct tc_if_config *data, *tmp;
	struct list_head *cur, *nex_tmp;

	PDEBUG("Loading configure\n");
	LIST_HEAD(config_list);
	xcostc_load_config(&config_list);

	list_for_each_entry_safe(data, tmp, &config_list, list)
	{
		tc_init(data);
		tc_add_exception(data);
	}

	PDEBUG("clean up\n");
	/* Free the allocated memory*/
	list_for_each_entry_safe(data, tmp, &config_list, list)
	{
		struct xcostc_mac *addr, *addr_tmp;
		list_for_each_entry_safe(addr, addr_tmp, 
			&data->white_list, list) {
			free(addr);
		}
	}
	list_for_each_safe(cur, nex_tmp, &config_list)
	{
		free(cur);	
	}

	config_cleanup();
}

void stop()
{
	struct tc_if_config *data, *tmp;
	struct list_head *cur, *nex_tmp;

	PDEBUG("Loading configure\n");
	LIST_HEAD(config_list);
	xcostc_load_config(&config_list);

	list_for_each_entry_safe(data, tmp, &config_list, list)
	{
		tc_stop(data);
	}

	PDEBUG("clean up\n");
	/* Free the allocated memory*/
	list_for_each_entry_safe(data, tmp, &config_list, list)
	{
		struct xcostc_mac *addr, *addr_tmp;
		list_for_each_entry_safe(addr, addr_tmp, 
			&data->white_list, list) {
			free(addr);
		}
	}
	list_for_each_safe(cur, nex_tmp, &config_list)
	{
		free(cur);	
	}

	config_cleanup();

}

static int32_t 
usage(void)
{
	fprintf(stderr, "xcostc {start|stop|restart}");
	return 1;
}

int main(int argc, char **argv)
{
	int32_t ch;

	while ((ch = getopt(argc, argv, "qh")) != -1)
	{
		switch (ch)
		{
			case 'q':
				if (freopen("/dev/null", "w", stderr)) {}
				break;
			case 'h':
				usage();
				goto out;
				break;
		}
	}

	if (optind >= argc)
	{
		usage();
		goto out;
	}

	if (!strcmp(argv[optind], "start")) {
		start();
	}
	else if (!strcmp(argv[optind], "stop")) {
		stop();
	}
	else if (!strcmp(argv[optind], "restart")) {
		stop();
		start();
	}

out:
	return 0;
}
