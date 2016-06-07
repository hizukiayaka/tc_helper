#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "libubox/list.h"

#ifdef XCOSTC_DEBUG
#define PDEBUG(fmt, args...)	fprintf(stderr, fmt, ## args)
#else
#define PDEBUG(fmt, args...)
#endif

#define PACKAGE_NAME	"xcostc"

#define CONFIG_MAX_LIST_LENGTH 24
/* Can't larger than 24, I wonder what cause the problem*/
#define B48INHEXSTRING 	18

#endif
