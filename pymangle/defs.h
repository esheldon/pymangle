#ifndef _MANGLE_DEFINITIONS_H
#define _MANGLE_DEFINITIONS_H

#define _MANGLE_SMALL_BUFFSIZE 25
#define _MANGLE_LARGE_BUFFSIZE 255
#define D2R  0.017453292519943295
#define R2D  57.295779513082323

#include <stdint.h>
typedef int64_t int64;

#define wlog(...) fprintf(stderr, __VA_ARGS__)

#endif
