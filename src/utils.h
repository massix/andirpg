#ifndef __UTILS__H__
#define __UTILS__H__

#include <stdint.h>
#include <stdio.h>

bool     strings_equal(const char *, const char *);
uint32_t min(uint32_t, uint32_t);
uint32_t max(uint32_t, uint32_t);
int64_t  file_size(FILE *);

#endif

