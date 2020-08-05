#include <math.h>
#include "common.h"

void
paint_round_rectangle (cairo_t       *cr,
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
                       gboolean       fill)
{
    gdouble degrees = M_PI / 180.0;

    cairo_new_sub_path (cr);
    cairo_arc (cr, rect->x + rect->width - radius, rect->y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc (cr, rect->x + rect->width - radius, rect->y + rect->height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc (cr, rect->x + radius, rect->y + rect->height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc (cr, rect->x + radius, rect->y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);

    if (fill)
    {
        cairo_set_source_rgba (cr, fill_red, fill_bule, fill_green, alpha);
        if (line)
            cairo_fill_preserve (cr);
        else
            cairo_fill (cr);
    }

    if (line)
    {
        cairo_set_source_rgba (cr, line_red, line_blue, line_green, 1.0);
        cairo_set_line_width (cr, line_width);
        cairo_stroke (cr);
    }
}
