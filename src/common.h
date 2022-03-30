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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <gtk/gtk.h>

void paint_round_rectangle(cairo_t *cr,
                           GdkRectangle *rect,
                           gdouble line_red,
                           gdouble line_blue,
                           gdouble line_green,
                           gdouble line_width,
                           gdouble line_alpha,
                           gdouble fill_red,
                           gdouble fill_bule,
                           gdouble fill_green,
                           gdouble fill_alpha,
                           gdouble radius,
                           gboolean line,
                           gboolean fill);

gchar *rgba_to_rgb_string(GdkRGBA *color);

#endif /* __COMMON_H__ */
