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

#ifndef __UI__GAME_MESSAGE_WINDOW__H__
#define __UI__GAME_MESSAGE_WINDOW__H__

typedef struct GameMessageWindow GameMessageWindow;

GameMessageWindow *game_message_window_new(int lines, int cols, int y, int x);

void game_message_window_clear_message(GameMessageWindow *);
void game_message_window_show_message(GameMessageWindow *, const char *);
void game_message_window_draw(GameMessageWindow *);
int  game_message_window_get_lines(GameMessageWindow *);
int  game_message_window_get_cols(GameMessageWindow *);

void game_message_window_free(GameMessageWindow *);

#endif /* ifndef __UI__GAME_MESSAGE_WINDOW__H__ */

