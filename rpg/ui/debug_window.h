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

#ifndef __UI__DEBUG_WINDOW__H__
#define __UI__DEBUG_WINDOW__H__

#include "engine.h"
typedef struct DebugWindow DebugWindow;

DebugWindow *debug_window_new(Engine *, int lines, int cols, int y, int x);

bool debug_window_is_visible(DebugWindow *);
void debug_window_show(DebugWindow *);
void debug_window_hide(DebugWindow *);

void debug_window_draw(DebugWindow *);
void debug_window_free(DebugWindow *);

#endif /* ifndef __UI__DEBUG_WINDOW__H__ */
