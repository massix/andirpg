#ifndef __UI__BOXED_WINDOW__H__
#define __UI__BOXED_WINDOW__H__

#include <ncurses.h>
#include <stdint.h>

typedef struct BoxedWindow BoxedWindow;

struct BoxedWindowOptions {
  uint8_t padding;

  // Edges
  char edge_tl; // char to use to draw top-left edge
  char edge_tr;
  char edge_bl;
  char edge_br;

  // Lines
  char line_h;
  char line_v;
};

// WINDOW is optional to create a subwin
BoxedWindow *boxed_window_new(const struct BoxedWindowOptions *options, const char *title, int lines, int cols, int y, int x,
                              WINDOW *parent);

WINDOW *boxed_window_get_inner_window(BoxedWindow *);

int boxed_window_get_cols(BoxedWindow *);
int boxed_window_get_lines(BoxedWindow *);

void boxed_window_options_default(struct BoxedWindowOptions *);

void boxed_window_move(BoxedWindow *, int y, int x);
void boxed_window_restore(BoxedWindow *);
void boxed_window_draw(BoxedWindow *);
void boxed_window_clear(BoxedWindow *);
void boxed_window_set_draw_function(BoxedWindow *, void (*draw_function)(void *, WINDOW *), void *);

void boxed_window_free(BoxedWindow *);

#endif /* ifndef __UI__BOXED_WINDOW__H__ */
