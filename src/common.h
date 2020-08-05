#ifndef __COMMON_H__
#define __COMMON_H__

#include <gtk/gtk.h>

void paint_round_rectangle (cairo_t       *cr,
                            GdkRectangle  *rect,
                            gdouble        line_red,
                            gdouble        line_blue,
                            gdouble        line_green,
                            gdouble        line_width,
                            gdouble        fill_red,
                            gdouble        fill_bule,
                            gdouble        fill_green,
                            gdouble        alpha,
                            gdouble        radius,
                            gboolean       line,
                            gboolean       fill);

#endif /* __COMMON_H__ */
