#ifndef __POINT__H__
#define __POINT__H__

#include <stdint.h>
typedef struct Point Point;

Point *point_new(uint32_t x, uint32_t y);

uint32_t point_get_x(Point *);
uint32_t point_get_y(Point *);
void     point_set_x(Point *, uint32_t);
void     point_set_y(Point *, uint32_t);

void point_free(Point *);

#endif
