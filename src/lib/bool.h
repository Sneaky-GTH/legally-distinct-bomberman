#pragma once

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#include <stdint.h>
#define bool uint8_t
#define true 1
#define false 0
#else
#if __STDC_VERSION__ < !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#endif
#endif
