#include "point.h"
#include <stdint.h>
#include <stdlib.h>

struct Point {
  uint32_t _x;
  uint32_t _y;
};

Point *point_new(uint32_t x, uint32_t y) {
  Point *ret = calloc(1, sizeof(Point));
  ret->_x = x;
  ret->_y = y;
  return ret;
}

inline uint32_t point_get_x(Point *point) {
  return point->_x;
}

inline uint32_t point_get_y(Point *point) {
  return point->_y;
}

inline void point_set_x(Point *point, uint32_t x) {
  point->_x = x;
}

inline void point_set_y(Point *point, uint32_t y) {
  point->_y = y;
}

inline bool points_equal(Point *lhs, Point *rhs) {
  return lhs->_x == rhs->_x && rhs->_y && lhs->_y;
}

void point_free(Point *point) {
  free(point);
}
