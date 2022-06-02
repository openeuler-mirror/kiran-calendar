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

#include "kiran-calendar.h"
#include <gtk/gtk.h>

#define CALENDA_WIDTH 304
#define CALENDA_HEIGHT 495

static void tran_setup(GtkWidget *window)
{
    GdkScreen *screen;
    GdkVisual *visual;

    gtk_widget_set_app_paintable(window, TRUE);
    screen = gdk_screen_get_default();
    visual = gdk_screen_get_rgba_visual(screen);

    if (visual != NULL && gdk_screen_is_composited(screen))
    {
        gtk_widget_set_visual(window, visual);
    }
}

int main(int argc,
         char **argv)
{
    GtkWidget *window;
    GtkWidget *calendar;
    int x, y;
    GdkDisplay *display;
    GdkRectangle monitor;
    GError *error = NULL;
    GtkCssProvider *provider;
    GdkScreen *screen;
    GFile *css_fp;

    if (!gtk_init_with_args(&argc, &argv, NULL, NULL, NULL, &error))
    {
        fprintf(stderr, "%s", error->message);
        g_error_free(error);
        exit(1);
    }

    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen,
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
    css_fp = g_file_new_for_path("/usr/share/kiran-calendar/kiran-calendar.css");
    gtk_css_provider_load_from_file(provider, css_fp, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Test Calendar");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(window, CALENDA_WIDTH, CALENDA_HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_deletable(GTK_WINDOW(window), FALSE);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
    gtk_window_stick(GTK_WINDOW(window));
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);

    tran_setup(window);

    display = gdk_display_get_default();
    gdk_monitor_get_geometry(gdk_display_get_monitor(display, 0), &monitor);
    x = monitor.width - CALENDA_WIDTH;
    y = monitor.height - CALENDA_HEIGHT;
    gtk_window_move(GTK_WINDOW(window), x, y);

    calendar = kiran_calendar_new();
    gtk_container_add(GTK_CONTAINER(window), calendar);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show(calendar);
    gtk_widget_show(window);

    gtk_main();

    g_object_unref(provider);
    g_object_unref(css_fp);

    return EXIT_SUCCESS;
}
