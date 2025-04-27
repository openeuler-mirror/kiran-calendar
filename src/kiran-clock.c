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
#include "common.h"
#include "kiran-time-date-gen.h"

#include <sys/time.h>

#define CALENDAR_SCHEMA "com.kylinsec.kiran.calendar"
#define KEY_CLOCK_FORMAT "clock-format"
#define KEY_LONG_DATE_FORMAT "long-date-format"
#define KEY_SHORT_DATE_FORMAT "short-date-format"
#define KEY_SHOW_SECONDS "show-seconds"

typedef enum
{
    CLOCK_FORMAT_12,
    CLOCK_FORMAT_24
} ClockFormat;

typedef enum
{
    DATE_BUFFER_TYPE,
    TIME_BUFFER_TYPE,
    DATE_TIME_BUFFER_TYPE_COUNT
} DateTimeBufferType;

struct _KiranClockPrivate
{
    struct tm time;

    gint refresh_timeout;
    gint align_timeout;
    gboolean is_hover;

    // time and date settings
    gboolean show_second;
    gchar *long_date_format;
    gchar *short_date_format;
    ClockFormat clock_format;

    GSettings *settings;

    GCancellable *cancellable;
    gchar *bus_name;
    gchar *object_path;
    guint name_id;

    KiranTimeDate *proxy;
    GHashTable *long_date_format_format_table;
    GHashTable *short_date_format_format_table;

    // 存储上一次绘制的时间和日期
    gchar *last_time_str;
    gchar *last_date_str;
};

static void kiran_clock_finalize(GObject *object);
static gboolean kiran_clock_draw(GtkWidget *widget,
                                 cairo_t *cr);
static void kiran_clock_destroy(GtkWidget *widget);
static void kiran_clock_map(GtkWidget *widget);
static void kiran_clock_unmap(GtkWidget *widget);
static gboolean kiran_clock_refresh(gpointer data);
static gboolean kiran_clock_timeout_refresh(gpointer data);
static gboolean kiran_clock_enter_notify(GtkWidget *widget,
                                         GdkEventCrossing *event);
static gboolean kiran_clock_leave_notify(GtkWidget *widget,
                                         GdkEventCrossing *event);

G_DEFINE_TYPE_WITH_PRIVATE(KiranClock, kiran_clock, GTK_TYPE_TOGGLE_BUTTON)

static void
kiran_clock_update_property(KiranClock *clock)
{
    KiranClockPrivate *priv;
    gchar *format;
    gint index;

    priv = kiran_clock_get_instance_private(clock);

    index = kiran_time_date_get_date_long_format_index(priv->proxy);
    format = g_hash_table_lookup(priv->long_date_format_format_table, GINT_TO_POINTER(index));
    if (format)
    {
        g_free(priv->long_date_format);
        priv->long_date_format = g_strdup(format);
    }

    index = kiran_time_date_get_date_short_format_index(priv->proxy);
    format = g_hash_table_lookup(priv->short_date_format_format_table, GINT_TO_POINTER(index));
    if (format)
    {
        g_free(priv->short_date_format);
        priv->short_date_format = g_strdup(format);
    }

    priv->clock_format = kiran_time_date_get_hour_format(priv->proxy);
    priv->show_second = kiran_time_date_get_seconds_showing(priv->proxy);

    kiran_clock_refresh(clock);
}

static void
kiran_clock_class_init(KiranClockClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

    gobject_class->finalize = kiran_clock_finalize;
    widget_class->draw = kiran_clock_draw;
    widget_class->destroy = kiran_clock_destroy;
    widget_class->map = kiran_clock_map;
    widget_class->unmap = kiran_clock_unmap;
    widget_class->enter_notify_event = kiran_clock_enter_notify;
    widget_class->leave_notify_event = kiran_clock_leave_notify;

    gtk_widget_class_install_style_property(widget_class,
                                            g_param_spec_string("text-color",
                                                                "Text color",
                                                                "Text color",
                                                                "#ffffff",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property(widget_class,
                                            g_param_spec_string("text-font",
                                                                "Text font",
                                                                "Text font",
                                                                "",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property(widget_class,
                                            g_param_spec_int("box-width",
                                                             "Box width",
                                                             "Box width",
                                                             -1, G_MAXINT,
                                                             100,
                                                             G_PARAM_READABLE));
};

static void
clock_format_changed(GSettings *settings,
                     gchar *key,
                     KiranClock *clock)
{
    KiranClockPrivate *priv;
    int new_format;

    priv = kiran_clock_get_instance_private(clock);
    new_format = g_settings_get_enum(settings, key);

    if (new_format == priv->clock_format)
        return;

    priv->clock_format = new_format;

    kiran_clock_refresh(clock);
}

static void
long_date_format_changed(GSettings *settings,
                         gchar *key,
                         KiranClock *clock)
{
    KiranClockPrivate *priv;

    priv = kiran_clock_get_instance_private(clock);

    g_free(priv->long_date_format);
    priv->long_date_format = g_settings_get_string(settings, key);

    kiran_clock_refresh(clock);
}

static void
short_date_format_changed(GSettings *settings,
                          gchar *key,
                          KiranClock *clock)
{
    KiranClockPrivate *priv;

    priv = kiran_clock_get_instance_private(clock);

    g_free(priv->short_date_format);
    priv->short_date_format = g_settings_get_string(settings, key);

    kiran_clock_refresh(clock);
}

static void
show_seconds_changed(GSettings *settings,
                     gchar *key,
                     KiranClock *clock)
{
    KiranClockPrivate *priv;

    priv = kiran_clock_get_instance_private(clock);
    priv->show_second = g_settings_get_boolean(priv->settings, KEY_SHOW_SECONDS);

    kiran_clock_refresh(clock);
}

static void
get_long_date_format_list_cb(GObject *source_object,
                             GAsyncResult *res,
                             gpointer user_data)
{
    GError *error;
    KiranClock *clock;
    KiranClockPrivate *priv;
    gchar **strokes;
    gsize i;

    clock = KIRAN_CLOCK(user_data);
    priv = kiran_clock_get_instance_private(clock);

    error = NULL;
    kiran_time_date_call_get_date_format_list_finish(KIRAN_TIME_DATE(source_object),
                                                     &strokes, res, &error);

    if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
        g_error_free(error);
        return;
    }

    if (error != NULL)
    {
        g_warning("%s", error->message);
        g_error_free(error);
        return;
    }

    for (i = 0; strokes && strokes[i]; i++)
    {
        g_hash_table_insert(priv->long_date_format_format_table, GINT_TO_POINTER(i), g_strdup(strokes[i]));
    }

    kiran_clock_update_property(clock);
}

static void
get_short_date_format_list_cb(GObject *source_object,
                              GAsyncResult *res,
                              gpointer user_data)
{
    GError *error;
    KiranClock *clock;
    KiranClockPrivate *priv;
    gchar **strokes;
    gsize i;

    clock = KIRAN_CLOCK(user_data);
    priv = kiran_clock_get_instance_private(clock);

    error = NULL;
    kiran_time_date_call_get_date_format_list_finish(KIRAN_TIME_DATE(source_object),
                                                     &strokes, res, &error);

    if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
        g_error_free(error);
        return;
    }

    if (error != NULL)
    {
        g_warning("%s", error->message);
        g_error_free(error);
        return;
    }

    for (i = 0; strokes && strokes[i]; i++)
    {
        g_hash_table_insert(priv->short_date_format_format_table, GINT_TO_POINTER(i), g_strdup(strokes[i]));
    }

    kiran_clock_update_property(clock);
}

static void
properties_changed(KiranTimeDate *proxy,
                   GVariant *change_properties,
                   GStrv invalidated_properties,
                   gpointer user_data)
{
    KiranClock *clock;

    clock = KIRAN_CLOCK(user_data);
    kiran_clock_update_property(clock);
}

static void
proxy_ready_cb(GObject *source_object,
               GAsyncResult *res,
               gpointer user_data)
{
    KiranTimeDate *proxy;
    GError *error;
    KiranClock *clock;
    KiranClockPrivate *priv;

    clock = KIRAN_CLOCK(user_data);
    priv = kiran_clock_get_instance_private(clock);

    error = NULL;
    proxy = kiran_time_date_proxy_new_finish(res, &error);

    if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
        g_error_free(error);
        return;
    }

    priv->proxy = proxy;

    if (error)
    {
        g_warning("%s", error->message);
        g_error_free(error);
        return;
    }

    g_signal_connect(proxy,
                     "g-properties-changed",
                     G_CALLBACK(properties_changed), clock);

    kiran_time_date_call_get_date_format_list(priv->proxy,
                                              0,
                                              priv->cancellable,
                                              get_long_date_format_list_cb,
                                              clock);

    kiran_time_date_call_get_date_format_list(priv->proxy,
                                              1,
                                              priv->cancellable,
                                              get_short_date_format_list_cb,
                                              clock);
}

static void
name_appeared_cb(GDBusConnection *connection,
                 const gchar *name,
                 const gchar *name_owner,
                 gpointer user_data)
{
    KiranClock *clock;
    KiranClockPrivate *priv;

    clock = KIRAN_CLOCK(user_data);
    priv = kiran_clock_get_instance_private(clock);

    kiran_time_date_proxy_new(connection, G_DBUS_PROXY_FLAGS_NONE,
                              priv->bus_name, priv->object_path,
                              priv->cancellable, proxy_ready_cb, clock);
}

static void
name_vanished_cb(GDBusConnection *connection,
                 const gchar *name,
                 gpointer user_data)
{
    KiranClock *clock;

    clock = KIRAN_CLOCK(user_data);

    g_clear_object(&clock->priv->proxy);
}

static void
kiran_clock_init(KiranClock *clock)
{
    KiranClockPrivate *priv;
    time_t timet;

    priv = clock->priv = kiran_clock_get_instance_private(clock);
    priv->refresh_timeout = 0;
    priv->align_timeout = 0;

    time(&timet);
    localtime(&timet);
    localtime_r(&timet, &priv->time);

    priv->long_date_format_format_table = g_hash_table_new_full(NULL, NULL, NULL, g_free);
    priv->short_date_format_format_table = g_hash_table_new_full(NULL, NULL, NULL, g_free);

    priv->settings = g_settings_new(CALENDAR_SCHEMA);

    priv->clock_format = g_settings_get_enum(priv->settings, KEY_CLOCK_FORMAT);
    priv->long_date_format = g_settings_get_string(priv->settings, KEY_LONG_DATE_FORMAT);
    priv->short_date_format = g_settings_get_string(priv->settings, KEY_SHORT_DATE_FORMAT);
    priv->show_second = g_settings_get_boolean(priv->settings, KEY_SHOW_SECONDS);

    g_signal_connect(priv->settings, "changed::" KEY_CLOCK_FORMAT, G_CALLBACK(clock_format_changed), clock);
    g_signal_connect(priv->settings, "changed::" KEY_LONG_DATE_FORMAT, G_CALLBACK(long_date_format_changed), clock);
    g_signal_connect(priv->settings, "changed::" KEY_SHORT_DATE_FORMAT, G_CALLBACK(short_date_format_changed), clock);
    g_signal_connect(priv->settings, "changed::" KEY_SHOW_SECONDS, G_CALLBACK(show_seconds_changed), clock);

    priv->is_hover = FALSE;

    priv->cancellable = g_cancellable_new();
    priv->bus_name = g_strdup("com.kylinsec.Kiran.SystemDaemon.TimeDate");
    priv->object_path = g_strdup("/com/kylinsec/Kiran/SystemDaemon/TimeDate");
    priv->name_id = g_bus_watch_name(G_BUS_TYPE_SYSTEM, priv->bus_name,
                                     G_BUS_NAME_WATCHER_FLAGS_NONE,
                                     name_appeared_cb, name_vanished_cb,
                                     clock, NULL);

    gtk_widget_set_name(GTK_WIDGET(clock), "kiranclock");

    priv->last_time_str = NULL;
    priv->last_date_str = NULL;
}

static void
kiran_clock_finalize(GObject *object)
{
    KiranClockPrivate *priv = KIRAN_CLOCK(object)->priv;

    if (priv->settings)
        g_object_unref(priv->settings);
    priv->settings = NULL;

    if (priv->name_id > 0)
    {
        g_bus_unwatch_name(priv->name_id);
        priv->name_id = 0;
    }

    if (priv->align_timeout)
    {
        g_source_remove(priv->align_timeout);
        priv->align_timeout = 0;
    }

    if (priv->refresh_timeout)
    {
        g_source_remove(priv->refresh_timeout);
        priv->refresh_timeout = 0;
    }

    g_cancellable_cancel(priv->cancellable);
    g_clear_object(&priv->cancellable);
    g_clear_object(&priv->proxy);

    g_free(priv->long_date_format);
    g_free(priv->short_date_format);
    g_free(priv->bus_name);
    g_free(priv->object_path);

    g_free(priv->last_time_str);
    g_free(priv->last_date_str);

    g_hash_table_destroy(priv->long_date_format_format_table);
    g_hash_table_destroy(priv->short_date_format_format_table);

    G_OBJECT_CLASS(kiran_clock_parent_class)->finalize(object);
}

// 获取当前显示的时间
static void
kiran_get_time_buffer(KiranClockPrivate *priv, gchar *buffer, int buf_size, DateTimeBufferType type)
{
    switch (type)
    {
    case DATE_BUFFER_TYPE:
    {
        if (g_strcmp0(priv->short_date_format, "") == 0)
        {
            // 默认格式
            if (g_getenv("LANG") && g_strrstr(g_getenv("LANG"), "zh_CN"))
                strftime(buffer, buf_size, "%Y/%m/%d", &(priv->time));
            else
                strftime(buffer, buf_size, "%m/%d/%Y", &(priv->time));
        }
        else
        {
            strftime(buffer, buf_size, priv->short_date_format, &(priv->time));
        }
    }
    break;
    case TIME_BUFFER_TYPE:
    {
        if (priv->show_second)
        {
            if (priv->clock_format == CLOCK_FORMAT_24)
                strftime(buffer, buf_size, "%H:%M:%S", &(priv->time));
            else
                strftime(buffer, buf_size, "%p %l:%M:%S", &(priv->time));
        }
        else
        {
            if (priv->clock_format == CLOCK_FORMAT_24)
                strftime(buffer, buf_size, "%H:%M", &(priv->time));
            else
                strftime(buffer, buf_size, "%p %l:%M", &(priv->time));
        }
    }
    break;
    default:
        break;
    }
}

static gboolean
kiran_clock_draw(GtkWidget *widget,
                 cairo_t *cr)
{
    KiranClockPrivate *priv;
    gint width, height;
    GtkStyleContext *context;
    PangoLayout *time_layout;
    PangoLayout *date_layout;
    PangoRectangle time_logical_rect;
    PangoRectangle date_logical_rect;
    gint x_loc, y_loc;
    gchar buffer[32];
    gchar *markup;
    gchar *tcolor;
    gchar *tfont;
    GdkRGBA color;

    gtk_widget_style_get(widget,
                         "text-font", &tfont,
                         NULL);

    priv = KIRAN_CLOCK(widget)->priv;

    cairo_save(cr);
    context = gtk_widget_get_style_context(widget);
    gtk_style_context_save(context);

    gtk_style_context_get_color(context,
                                GTK_STATE_NORMAL,
                                &color);

    tcolor = rgba_to_rgb_string(&color);

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    if (priv->is_hover)
    {
        // draw background
        gtk_render_background(context,
                              cr,
                              0, 0, width, height);
    }

    kiran_get_time_buffer(priv, buffer, sizeof(buffer), TIME_BUFFER_TYPE);

    markup = g_strconcat("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);

    time_layout = gtk_widget_create_pango_layout(widget, buffer);
    pango_layout_set_alignment(time_layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup(time_layout, markup, -1);
    g_free(markup);

    kiran_get_time_buffer(priv, buffer, sizeof(buffer), DATE_BUFFER_TYPE);

    markup = g_strconcat("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);
    date_layout = gtk_widget_create_pango_layout(widget, buffer);
    pango_layout_set_alignment(date_layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup(date_layout, markup, -1);
    g_free(markup);

    pango_layout_get_pixel_extents(time_layout, NULL, &time_logical_rect);
    pango_layout_get_pixel_extents(date_layout, NULL, &date_logical_rect);

    y_loc = (height - (time_logical_rect.height + date_logical_rect.height)) / 2;

    x_loc = (width - time_logical_rect.width) / 2;
    gtk_render_layout(context, cr, x_loc, y_loc, time_layout);

    y_loc = y_loc + time_logical_rect.height;
    x_loc = (width - date_logical_rect.width) / 2;
    gtk_render_layout(context, cr, x_loc, y_loc, date_layout);

    g_object_unref(time_layout);
    g_object_unref(date_layout);

    cairo_restore(cr);
    gtk_style_context_restore(context);

    g_free(tcolor);
    g_free(tfont);

    return FALSE;
}

static void
kiran_clock_update_tooltip(KiranClock *clock)
{
    KiranClockPrivate *priv = clock->priv;
    gchar buffer[32];

    if (g_strcmp0(priv->long_date_format, "") == 0)
    {
        // 默认格式
        if (g_getenv("LANG") && g_strrstr(g_getenv("LANG"), "zh_CN"))
            strftime(buffer, sizeof(buffer), "%A,%Y年%m月%d日", &(priv->time));
        else
            strftime(buffer, sizeof(buffer), "%A,%B %d,%Y", &(priv->time));
    }
    else
    {
        strftime(buffer, sizeof(buffer), priv->long_date_format, &(priv->time));
    }

    gtk_widget_set_tooltip_text(GTK_WIDGET(clock), buffer);
}

static void
kiran_clock_update_time(KiranClock *clock)
{
    KiranClockPrivate *priv = clock->priv;

    time_t timet;

    time(&timet);
    localtime_r(&timet, &priv->time);
    kiran_clock_update_tooltip(clock);
}

static gboolean
kiran_clock_timeout_align(gpointer data)
{
    kiran_clock_timeout_refresh(data);

    KiranClock *clock = KIRAN_CLOCK(data);
    KiranClockPrivate *priv = clock->priv;

    if (priv->refresh_timeout)
    {
        g_source_remove(priv->refresh_timeout);
        priv->refresh_timeout = 0;
    }

    priv->refresh_timeout = gdk_threads_add_timeout(1000,
                                                    kiran_clock_timeout_refresh,
                                                    clock);
    g_source_set_name_by_id(priv->refresh_timeout, "[kiran]kiran_clock_timeout_refresh");

    return FALSE;  // 让align定时器停止
}

void kiran_clock_start_align(gpointer data)
{
    KiranClock *clock = KIRAN_CLOCK(data);
    KiranClockPrivate *priv = clock->priv;

    // 时间对齐
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long ms = tv.tv_usec / 1000;
    long delay = 1000 - ms;

    if (priv->align_timeout)
    {
        g_source_remove(priv->align_timeout);
        priv->align_timeout = 0;
    }
    priv->align_timeout = gdk_threads_add_timeout(delay, kiran_clock_timeout_align, clock);
    g_source_set_name_by_id(priv->align_timeout, "[kiran]kiran_clock_timeout_align");
}

static gboolean
kiran_clock_timeout_refresh(gpointer data)
{
    KiranClock *clock = KIRAN_CLOCK(data);
    KiranClockPrivate *priv = clock->priv;

    static int tick_count = 0;
    tick_count++;
    if (tick_count >= 60)
    {
        // 每60次，也就是大约1分钟，重新对齐时间
        tick_count = 0;
        kiran_clock_start_align(data);

        return FALSE;  // 当前这个定时器停掉
    }

    gchar time_buffer[32];
    gchar date_buffer[32];
    gboolean time_changed = FALSE;
    gboolean date_changed = FALSE;

    kiran_clock_update_time(clock);

    kiran_get_time_buffer(priv, time_buffer, sizeof(time_buffer), TIME_BUFFER_TYPE);
    kiran_get_time_buffer(priv, date_buffer, sizeof(date_buffer), DATE_BUFFER_TYPE);

    if (g_strcmp0(time_buffer, priv->last_time_str) != 0)
    {
        time_changed = TRUE;
        g_free(priv->last_time_str);
        priv->last_time_str = g_strdup(time_buffer);
    }
    if (g_strcmp0(date_buffer, priv->last_date_str) != 0)
    {
        date_changed = TRUE;
        g_free(priv->last_date_str);
        priv->last_date_str = g_strdup(date_buffer);
    }

    // 若需要显示的时间或日期改变，则刷新
    if (time_changed || date_changed)
    {
        gtk_widget_queue_draw(GTK_WIDGET(clock));
    }

    return TRUE;
}

static gboolean
kiran_clock_refresh(gpointer data)
{
    KiranClock *clock = KIRAN_CLOCK(data);

    kiran_clock_update_time(clock);

    gtk_widget_queue_draw(GTK_WIDGET(clock));

    return TRUE;
}

static gboolean
kiran_clock_enter_notify(GtkWidget *widget,
                         GdkEventCrossing *event)
{
    KiranClockPrivate *priv = KIRAN_CLOCK(widget)->priv;
    gtk_widget_set_state_flags(widget, GTK_STATE_FLAG_PRELIGHT, FALSE);

    priv->is_hover = TRUE;

    kiran_clock_refresh(widget);

    return TRUE;
}

static gboolean
kiran_clock_leave_notify(GtkWidget *widget,
                         GdkEventCrossing *event)
{
    KiranClockPrivate *priv = KIRAN_CLOCK(widget)->priv;
    GtkStateFlags flags;

    flags = gtk_widget_get_state_flags(widget);
    flags = (GtkStateFlags)(flags & ~GTK_STATE_FLAG_PRELIGHT);
    gtk_widget_set_state_flags(widget, flags, TRUE);

    priv->is_hover = FALSE;
    kiran_clock_refresh(widget);

    return TRUE;
}

static void
kiran_clock_destroy(GtkWidget *widget)
{
    KiranClockPrivate *priv = KIRAN_CLOCK(widget)->priv;

    if (priv->align_timeout)
    {
        g_source_remove(priv->align_timeout);
        priv->align_timeout = 0;
    }

    if (priv->refresh_timeout)
    {
        g_source_remove(priv->refresh_timeout);
        priv->refresh_timeout = 0;
    }

    GTK_WIDGET_CLASS(kiran_clock_parent_class)->destroy(widget);
}

static void
kiran_clock_map(GtkWidget *widget)
{
    KiranClock *clock = KIRAN_CLOCK(widget);
    KiranClockPrivate *priv = clock->priv;

    GTK_WIDGET_CLASS(kiran_clock_parent_class)->map(widget);

    if (!priv->align_timeout)
    {
        kiran_clock_start_align(clock);
    }
}

static void
kiran_clock_unmap(GtkWidget *widget)
{
    KiranClockPrivate *priv = KIRAN_CLOCK(widget)->priv;

    GTK_WIDGET_CLASS(kiran_clock_parent_class)->unmap(widget);

    if (priv->align_timeout)
    {
        g_source_remove(priv->align_timeout);
        priv->align_timeout = 0;
    }

    if (priv->refresh_timeout)
    {
        g_source_remove(priv->refresh_timeout);
        priv->refresh_timeout = 0;
    }
}

/**
 * kiran_clock_new
 *
 * 创建一个新的 #KiranClock
 *
 * Returns: 一个新的 #KiranClock
 */
GtkWidget *
kiran_clock_new(void)
{
    return g_object_new(KIRAN_TYPE_CLOCK, NULL);
}
