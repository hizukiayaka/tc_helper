#include "common.h"
#include "read_config.h"

int main(int argc, char **argv) {
	uint8_t *value;
	void *interface = NULL;
	uint8_t **list;
	uint8_t length;

	if(!config_load_packge(PACKAGE_NAME))
		exit(1);

	interface = config_find_interface("wan");	

	if(NULL == interface)
		exit(1);

	value = config_find_option(interface, "start");

	printf("%s\n", value);
	
	list = malloc(CONFIG_MAX_LIST_LENGTH);
	if (NULL == list)
		exit(1);

	for (uint8_t i = 0; i < CONFIG_MAX_LIST_LENGTH; i++) {
		uint8_t *ptr;

		ptr = malloc(B48INHEXSTRING);
		*(list + i) = ptr;
	}

	length = config_find_list(interface, "white_ip", list);

	for (uint8_t i = 0; i < length; i++) {
		printf("%s\n", *(list + i));
	}

	for (uint8_t i = 0; i < length; i++) {
		free(*(list + i));
	}

	free(list);


	config_cleanup();

	exit(0);
}
