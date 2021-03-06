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

#include "kiran-calendar-window.h"
#include "kiran-calendar.h"

struct _KiranCalendarWindowPrivate
{
    GtkWidget *calendar;
};

static GObject *kiran_calendar_window_constructor (GType                  type,
                             		           guint                  n_construct_properties,
                            		           GObjectConstructParam *construct_properties);

G_DEFINE_TYPE_WITH_PRIVATE (KiranCalendarWindow, kiran_calendar_window, GTK_TYPE_WINDOW)

static gboolean
kiran_calendar_window_grab(GtkWidget *widget)
{
    GdkGrabStatus status;
    GdkDisplay *display;
    GdkSeat *seat;

    if (!gtk_widget_is_visible(widget))
        return FALSE;

    display = gtk_widget_get_display(widget);
    seat = gdk_display_get_default_seat(display);

    status = gdk_seat_grab(seat, gtk_widget_get_window(widget),
                           GDK_SEAT_CAPABILITY_POINTER, TRUE,
                           NULL, NULL, NULL, NULL);
    return status == GDK_GRAB_SUCCESS;
}

static void
kiran_calendar_window_ungrab(GtkWidget *widget)
{
    GdkDisplay *display;
    GdkSeat *seat;

    display = gtk_widget_get_display(widget);
    seat = gdk_display_get_default_seat(display);

    gdk_seat_ungrab(seat);
}

static gboolean
kiran_calendar_window_map_event(GtkWidget *widget,
                                   GdkEventAny *event)
{
    return kiran_calendar_window_grab(widget);
}

static gboolean
kiran_calendar_window_unmap_event(GtkWidget *widget,
                                     GdkEventAny *event)
{
    kiran_calendar_window_ungrab(widget);

    return FALSE;
}

static gboolean
kiran_calendar_window_button_press_event(GtkWidget *widget,
                                            GdkEventButton *event)
{
    GdkWindow *window;
    gint root_x, root_y;
    gint width, height;

    window = gtk_widget_get_window(widget);

    gdk_window_get_origin(window, &root_x, &root_y);
    width = gdk_window_get_width(window);
    height = gdk_window_get_height(window);

    if (event->x_root < root_x || event->x_root > root_x + width ||
        event->y_root < root_y || event->y_root > root_y + height)
    {
        gtk_widget_hide(widget);
    }

    return FALSE;
}

static gboolean
kiran_calendar_window_draw(GtkWidget *widget,
                           cairo_t *cr)
{
    GtkStyleContext *context;
    GtkAllocation alloc;
    GtkStateFlags state;
    gboolean ret = FALSE;
    gdouble degrees = M_PI / 180.0;
    GdkRGBA bg_color;
    GValue value = G_VALUE_INIT;
    gint radius;

    context = gtk_widget_get_style_context (GTK_WIDGET (widget));
    gtk_style_context_save (context);

    gtk_widget_get_allocation(widget, &alloc);
    state = gtk_widget_get_state_flags (widget);

    gtk_style_context_add_class (context, "background");
    gtk_style_context_get_background_color (context,
                                            state,
                                            &bg_color);

    gtk_style_context_get_property (context,
                                    "border-radius",
                                    state,
                                    &value);

    radius = g_value_get_int (&value);
    g_value_unset (&value);

    cairo_new_sub_path(cr);
    cairo_arc(cr, alloc.x + alloc.width - radius, alloc.y + radius, radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, alloc.x + alloc.width - radius, alloc.y + alloc.height - radius, radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, alloc.x + radius, alloc.y + alloc.height - radius, radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, alloc.x + radius, alloc.y + radius, radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);

    cairo_set_source_rgba (cr, bg_color.red, bg_color.green, bg_color.blue, bg_color.alpha);
    cairo_fill_preserve (cr);
    cairo_clip(cr);

    gtk_style_context_restore (context);

    if (GTK_WIDGET_CLASS(kiran_calendar_window_parent_class)->draw)
        ret = GTK_WIDGET_CLASS(kiran_calendar_window_parent_class)->draw(widget, cr);

    return ret;
}

static void
kiran_calendar_window_class_init (KiranCalendarWindowClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    gobject_class->constructor = kiran_calendar_window_constructor;
    widget_class->map_event = kiran_calendar_window_map_event;
    widget_class->unmap_event = kiran_calendar_window_unmap_event;
    widget_class->button_press_event = kiran_calendar_window_button_press_event;
    widget_class->draw = kiran_calendar_window_draw;
}

static void
kiran_calendar_window_init (KiranCalendarWindow *window)
{
    KiranCalendarWindowPrivate *priv;

    priv = window->priv = kiran_calendar_window_get_instance_private (window);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_NORMAL);
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
    gtk_window_stick (GTK_WINDOW (window));
    gtk_window_set_title (GTK_WINDOW (window), "Kiran Calendar"); 
    gtk_window_set_default_size (GTK_WINDOW (window), CALENDA_WIDTH, CALENDA_HEIGHT);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);

}

static 
void tran_setup (GtkWidget *window)
{
    GdkScreen *screen;
    GdkVisual *visual;

    gtk_widget_set_app_paintable (window, TRUE);
    screen = gdk_screen_get_default ();
    visual = gdk_screen_get_rgba_visual (screen);

    if (visual != NULL && gdk_screen_is_composited (screen))
    {
        gtk_widget_set_visual (window, visual);
    }
}

static GObject *
kiran_calendar_window_constructor (GType                  type,
                                   guint                  n_construct_properties,
                                   GObjectConstructParam *construct_properties)
{
    GObject        *obj;
    KiranCalendarWindow *win;
    GtkWidget *frame;

    obj = G_OBJECT_CLASS (kiran_calendar_window_parent_class)->constructor (type,
                                                                      n_construct_properties,
                                                                      construct_properties);
    win = KIRAN_CALENDAR_WINDOW (obj);
    gtk_widget_set_name(GTK_WIDGET(win), "calendarWindow");

    tran_setup (GTK_WIDGET (win));

    frame = gtk_frame_new (NULL);
    gtk_widget_show (frame);
    gtk_container_add (GTK_CONTAINER (win), frame);

    win->priv->calendar = kiran_calendar_new ();  
    gtk_widget_show (GTK_WIDGET (win->priv->calendar));
    gtk_container_add (GTK_CONTAINER (frame), win->priv->calendar);

    return obj;
}

GtkWidget *
kiran_calendar_window_new (void)
{
    return g_object_new (KIRAN_TYPE_CALENDAR_WINDOW, NULL);
}

void 
kiran_calendar_window_show (KiranCalendarWindow *window)
{
    
    kiran_calendar_refresh (KIRAN_CALENDAR (window->priv->calendar));
    gtk_widget_show (GTK_WIDGET (window));
}

void 
kiran_calendar_window_hide (KiranCalendarWindow *window)
{
   gtk_widget_hide (GTK_WIDGET (window));
}
