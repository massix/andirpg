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

#include "boxed_window.h"
#include "logger.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct BoxedWindow {
  WINDOW *_main_window;
  WINDOW *_inner_window;
  char   *_title;

  int _lines;
  int _cols;
  int _x;
  int _y;

  struct BoxedWindowOptions *_options;
  void (*_inner_draw)(void *, WINDOW *);
  void *_draw_param;
};

void boxed_window_options_default(struct BoxedWindowOptions *bwo) {
  bwo->padding = 2;

  bwo->edge_br = '+';
  bwo->edge_tr = '+';
  bwo->edge_bl = '+';
  bwo->edge_tl = '+';

  bwo->line_v = '|';
  bwo->line_h = '-';
}

void boxed_window_options_clone(const struct BoxedWindowOptions *origin, struct BoxedWindowOptions **target) {
  *target = calloc(1, sizeof(struct BoxedWindowOptions));

  (*target)->padding = origin->padding;

  // Edges
  (*target)->edge_tl = origin->edge_tl;
  (*target)->edge_bl = origin->edge_bl;
  (*target)->edge_br = origin->edge_br;
  (*target)->edge_tr = origin->edge_tr;

  // Lines
  (*target)->line_h = origin->line_h;
  (*target)->line_v = origin->line_v;
}

BoxedWindow *boxed_window_new(const struct BoxedWindowOptions *options, const char *title, int lines, int cols, int y, int x,
                              WINDOW *parent) {
  BoxedWindow *ret = calloc(1, sizeof(BoxedWindow));
  boxed_window_options_clone(options, &ret->_options);

  ret->_inner_draw = nullptr;
  ret->_draw_param = nullptr;
  ret->_title = strdup(title);
  ret->_lines = lines;
  ret->_cols = cols;
  ret->_x = x;
  ret->_y = y;

  ret->_main_window = subwin(parent == nullptr ? stdscr : parent, ret->_lines, ret->_cols, ret->_y, ret->_x);

  int pad = ret->_options->padding * 2;
  ret->_inner_window =
    subwin(ret->_main_window, ret->_lines - pad, ret->_cols - pad, ret->_y + ret->_options->padding, ret->_x + ret->_options->padding);

  return ret;
}

inline int boxed_window_get_cols(BoxedWindow *bxw) {
  return bxw->_cols;
}

inline int boxed_window_get_lines(BoxedWindow *bxw) {
  return bxw->_lines;
}

void boxed_window_draw(BoxedWindow *bxw) {
  LOG_DEBUG("Drawing boxed window", 0);
  int max_x;
  int max_y;
  int cur_x = 0;
  int cur_y = 0;
  getmaxyx(bxw->_main_window, max_y, max_x);
  wclear(bxw->_main_window);

  // Draw the four edges
  mvwprintw(bxw->_main_window, 0, 0, "%c", bxw->_options->edge_tl);
  mvwprintw(bxw->_main_window, max_y - 1, 0, "%c", bxw->_options->edge_bl);
  mvwprintw(bxw->_main_window, 0, max_x - 1, "%c", bxw->_options->edge_tr);
  mvwprintw(bxw->_main_window, max_y - 1, max_x - 1, "%c", bxw->_options->edge_br);

  // Draw the two horizontal lines (top-left -> top-right and bottom-left -> bottom-right)
  for (cur_x = 1; cur_x < max_x - 1; cur_x++) {
    mvwprintw(bxw->_main_window, 0, cur_x, "%c", bxw->_options->line_h);
    mvwprintw(bxw->_main_window, max_y - 1, cur_x, "%c", bxw->_options->line_h);
  }

  // Draw the two vertical lines (top-left -> bottom-left and top-right -> bottom-right)
  for (cur_y = 1; cur_y < max_y - 1; cur_y++) {
    mvwprintw(bxw->_main_window, cur_y, 0, "%c", bxw->_options->line_v);
    mvwprintw(bxw->_main_window, cur_y, max_x - 1, "%c", bxw->_options->line_v);
  }

  // Draw the title
  uint32_t title_length = 0;
  char    *title = calloc(strlen(bxw->_title) + 50, sizeof(char));
  sprintf(title, "[ %s ]", bxw->_title);
  title_length = strlen(title);

  mvwprintw(bxw->_main_window, 0, max_x - title_length - 3, "%s", title);

  wrefresh(bxw->_main_window);

  // Draw the inner window (if any)
  if (bxw->_inner_draw != nullptr) {
    LOG_DEBUG("Drawing inner window", 0);

    wclear(bxw->_inner_window);
    bxw->_inner_draw(bxw->_draw_param, bxw->_inner_window);
    wrefresh(bxw->_inner_window);
  }
}

inline WINDOW *boxed_window_get_inner_window(BoxedWindow *bxw) {
  return bxw->_inner_window;
}

void boxed_window_set_draw_function(BoxedWindow *bxw, void (*draw_function)(void *, WINDOW *), void *param) {
  bxw->_inner_draw = draw_function;
  bxw->_draw_param = param;
}

void boxed_window_free(BoxedWindow *bxw) {
  delwin(bxw->_inner_window);
  delwin(bxw->_main_window);
  free(bxw->_options);
  free(bxw->_title);
  free(bxw);
}

