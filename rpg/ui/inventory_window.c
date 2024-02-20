// andirpg
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

#include "inventory_window.h"
#include "boxed_window.h"
#include "entity.h"
#include "logger.h"
#include <ncurses.h>
#include <stdlib.h>

struct InventoryWindow {
  Entity      *_entity;
  BoxedWindow *_window;
};

InventoryWindow *inventory_window_new(Entity *entity, int lines, int cols, int x, int y) {
  InventoryWindow *ret = calloc(1, sizeof(InventoryWindow));
  LOG_DEBUG("Creating inventory window %dx%d", cols, lines);

  ret->_entity = entity;
  struct BoxedWindowOptions options;
  boxed_window_options_default(&options);

  ret->_window = boxed_window_new(&options, "Inventory", lines, cols, y, x, nullptr);

  return ret;
}

void inventory_window_draw_inner(InventoryWindow *ivw) {
  WINDOW *target = boxed_window_get_inner_window(ivw->_window);

  wclear(target);
  mvwprintw(target, 0, 0, "No items");
  wrefresh(target);
}

void inventory_window_draw(InventoryWindow *ivw) {
  boxed_window_draw(ivw->_window);
  inventory_window_draw_inner(ivw);
}

void inventory_window_free(InventoryWindow *ivw) {
  boxed_window_free(ivw->_window);
  free(ivw);
}
