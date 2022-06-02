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

#include "kiran-clock.h"
#include <gtk/gtk.h>
#include "kiran-calendar-window.h"

static void
button_toggled(GtkWidget *widget,
               gpointer user_data)
{
    KiranCalendarWindow *window = KIRAN_CALENDAR_WINDOW(user_data);

    if (gtk_widget_is_visible(GTK_WIDGET(window)))
        kiran_calendar_window_hide(window);
    else
        kiran_calendar_window_show(window);
}

int main(int argc,
         char **argv)
{
    GtkWidget *window;
    GtkWidget *calendar_window;
    GtkWidget *clock;
    GError *error = NULL;

    if (!gtk_init_with_args(&argc, &argv, NULL, NULL, NULL, &error))
    {
        fprintf(stderr, "%s", error->message);
        g_error_free(error);
        exit(1);
    }

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Test Clock");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    calendar_window = kiran_calendar_window_new();

    clock = kiran_clock_new();
    gtk_container_add(GTK_CONTAINER(window), clock);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    g_signal_connect(clock, "toggled", G_CALLBACK(button_toggled), calendar_window);

    gtk_widget_show(clock);
    gtk_widget_show(window);

    gtk_main();

    gtk_widget_destroy(calendar_window);

    return EXIT_SUCCESS;
}
