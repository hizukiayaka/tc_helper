#ifndef _TC_H_
#define _TC_H_
#include "common.h"

struct tc_if_config {
	uint8_t *out_if;
	uint8_t *in_if;
	uint8_t *ip4_start;
	uint8_t *ip4_end;
	uint8_t **ip4_white_list;
	uint8_t download_persent;
	uint8_t upload_persent;
	uint16_t download_rate;
	uint16_t upload_rate;
};

#endif
