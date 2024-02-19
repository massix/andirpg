#ifndef __UI__PLAYER_WINDOW__H__
#define __UI__PLAYER_WINDOW__H__

#include "entity.h"
#include "ui_point.h"
#include <ncurses.h>

typedef struct PlayerWindow PlayerWindow;

PlayerWindow *player_window_new(Entity *, int lines, int cols, int x, int y);
WINDOW       *player_window_get_ncurses_window(PlayerWindow *);
void          player_window_draw(PlayerWindow *);
UiPoint      *player_window_get_ui_point(PlayerWindow *);
int           player_window_get_lines(PlayerWindow *);
int           player_window_get_cols(PlayerWindow *);

void player_window_free(PlayerWindow *);

#endif /* ifndef __UI__PLAYER_WINDOW__H__ */
