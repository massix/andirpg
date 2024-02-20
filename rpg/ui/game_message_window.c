// andirpg -- description to be modified
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

#include "game_message_window.h"
#include "boxed_window.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct GameMessageWindow {
  BoxedWindow *_window;
  char        *_message;
  uint32_t     _message_size;
  uint32_t     _max_redraws;
  uint32_t     _actual_redraws;
};

GameMessageWindow *game_message_window_new(int lines, int cols, int y, int x) {
  GameMessageWindow *ret = calloc(1, sizeof(GameMessageWindow));

  struct BoxedWindowOptions options;
  boxed_window_options_default(&options);

  ret->_window = boxed_window_new(&options, "Game Message", lines, cols, y, x, nullptr);
  ret->_max_redraws = 10;

  return ret;
}

void game_message_window_clear_message(GameMessageWindow *gmw) {
  if (gmw->_message != nullptr) {
    free(gmw->_message);
    gmw->_message_size = 0;
    gmw->_message = nullptr;
  }

  gmw->_actual_redraws = 0;
}

void game_message_window_show_message(GameMessageWindow *gmw, const char *msg) {
  game_message_window_clear_message(gmw);
  gmw->_message = strdup(msg);
  gmw->_message_size = strlen(gmw->_message);
}

void game_message_window_draw(GameMessageWindow *gmw) {
  boxed_window_draw(gmw->_window);

  WINDOW *target = boxed_window_get_inner_window(gmw->_window);

  if (gmw->_actual_redraws > gmw->_max_redraws) {
    game_message_window_clear_message(gmw);
  }

  wclear(target);
  if (gmw->_message == nullptr) {
    mvwprintw(target, 0, 0, "No messages to show, yet...");
  } else {
    mvwprintw(target, 0, 0, "%-10s", gmw->_message);
  }

  gmw->_actual_redraws++;

  wrefresh(target);
}

inline int game_message_window_get_cols(GameMessageWindow *gmw) {
  return boxed_window_get_cols(gmw->_window);
}

inline int game_message_window_get_lines(GameMessageWindow *gmw) {
  return boxed_window_get_lines(gmw->_window);
}

void game_message_window_free(GameMessageWindow *gmw) {
  boxed_window_free(gmw->_window);
  if (gmw->_message != nullptr) {
    free(gmw->_message);
  }

  free(gmw);
}
