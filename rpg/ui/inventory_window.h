#ifndef __UI__INVENTORY_WINDOW__H__
#define __UI__INVENTORY_WINDOW__H__

#include "entity.h"
#include "ui_point.h"
typedef struct InventoryWindow InventoryWindow;

InventoryWindow *inventory_window_new(Entity *, int lines, int cols, int x, int y);

int      inventory_window_get_cols(InventoryWindow *);
int      inventory_window_get_lines(InventoryWindow *);
void     inventory_window_draw(InventoryWindow *);
UiPoint *inventory_window_get_ui_point(InventoryWindow *);

void inventory_window_free(InventoryWindow *);

#endif /* ifndef __UI__INVENTORY_WINDOW__H__ */
