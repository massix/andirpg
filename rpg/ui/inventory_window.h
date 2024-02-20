#ifndef __UI__INVENTORY_WINDOW__H__
#define __UI__INVENTORY_WINDOW__H__

#include "entity.h"
typedef struct InventoryWindow InventoryWindow;

InventoryWindow *inventory_window_new(Entity *, int lines, int cols, int x, int y);

int  inventory_window_get_cols(InventoryWindow *);
int  inventory_window_get_lines(InventoryWindow *);
void inventory_window_draw(InventoryWindow *);

void inventory_window_free(InventoryWindow *);

#endif /* ifndef __UI__INVENTORY_WINDOW__H__ */
