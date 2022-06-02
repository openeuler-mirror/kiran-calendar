/**
* @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd. 
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; If not, see <http: //www.gnu.org/licenses/>. 
*
* Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
*/

#include <math.h>
#include "common.h"

void
paint_round_rectangle (cairo_t       *cr,
                       GdkRectangle  *rect,
                       gdouble        line_red,
                       gdouble        line_blue,
                       gdouble        line_green,
                       gdouble        line_width,
                       gdouble        line_alpha,
                       gdouble        fill_red,
                       gdouble        fill_bule,
                       gdouble        fill_green,
                       gdouble        fill_alpha,
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
        cairo_set_source_rgba (cr, fill_red, fill_bule, fill_green, fill_alpha);
        if (line)
            cairo_fill_preserve (cr);
        else
            cairo_fill (cr);
    }

    if (line)
    {
        cairo_set_source_rgba (cr, line_red, line_blue, line_green, line_alpha);
        cairo_set_line_width (cr, line_width);
        cairo_stroke (cr);
    }
}

gchar *
rgba_to_rgb_string (GdkRGBA *color)
{
    int r, g, b;

    r =  color->red * 255;
    g =  color->green * 255;
    b =  color->blue * 255;

    return g_strdup_printf ("#%02x%02x%02x", r, g, b);
}
