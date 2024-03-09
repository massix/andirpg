// AndiRPG - Name not final
// Copyright © 2024 Massimo Gengarelli
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
