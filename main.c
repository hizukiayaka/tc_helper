#include "common.h"
#include "options.h"
#include "tc.h"

int main(int argc, char **argv)
{
	LIST_HEAD(config_list);
	xcostc_load_config(&config_list);
}
