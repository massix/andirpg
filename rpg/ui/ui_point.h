#ifndef __UI__POINT__H__
#define __UI__POINT__H__

typedef struct UiPoint UiPoint;

UiPoint *ui_point_new(int x, int y);
int      ui_point_get_x(UiPoint *);
int      ui_point_get_y(UiPoint *);
void     ui_point_free(UiPoint *);

#endif /* ifndef __UI__POINT__H__ */
