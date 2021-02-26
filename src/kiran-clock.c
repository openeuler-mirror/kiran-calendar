#include "kiran-clock.h"
#include "common.h"

#define CALENDAR_SCHEMA "com.unikylin.Kiran.kiran-calendar"
#define KEY_CLOCK_FORMAT "clock-format"
#define KEY_LONG_DATE_FORMAT  "long-date-format"
#define KEY_SHORT_DATE_FORMAT  "short-date-format"
#define KEY_SHOW_SECONDS "show-seconds"

typedef enum {
        CLOCK_FORMAT_12,
        CLOCK_FORMAT_24
} ClockFormat;

struct _KiranClockPrivate
{
    struct tm time;

    gint refresh_timeout; 
    gboolean is_hover;

    //time and date settings
    gboolean show_second;
    gchar *long_date_format;
    gchar *short_date_format;
    ClockFormat clock_format;
    
    GSettings *settings;
};

static void kiran_clock_finalize        (GObject *object);
static gboolean kiran_clock_draw 	(GtkWidget *widget,
				         cairo_t   *cr);
static void kiran_clock_destroy        	(GtkWidget *widget);
static void kiran_clock_map        	(GtkWidget *widget);
static void kiran_clock_unmap        	(GtkWidget *widget);
static gboolean kiran_clock_refresh 	(gpointer data);
static gboolean kiran_clock_enter_notify (GtkWidget        *widget,
                                          GdkEventCrossing *event);
static gboolean kiran_clock_leave_notify (GtkWidget        *widget,
                                          GdkEventCrossing *event);

G_DEFINE_TYPE_WITH_PRIVATE (KiranClock, kiran_clock, GTK_TYPE_TOGGLE_BUTTON)

static void
kiran_clock_class_init (KiranClockClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

    gobject_class->finalize  = kiran_clock_finalize;
    widget_class->draw = kiran_clock_draw;
    widget_class->destroy = kiran_clock_destroy;
    widget_class->map = kiran_clock_map;
    widget_class->unmap = kiran_clock_unmap;
    widget_class->enter_notify_event = kiran_clock_enter_notify;
    widget_class->leave_notify_event = kiran_clock_leave_notify;

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("text-color",
                                                                "Text color",
                                                                "Text color",
                                                                "#ffffff",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("text-font",
                                                                "Text font",
                                                                "Text font",
                                                                "",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_int ("box-width",
                                                             "Box width",
                                                             "Box width",
							     -1, G_MAXINT,
                                                             -1,
                                                             G_PARAM_READABLE));

   
    gtk_widget_class_set_css_name (widget_class, "kiranclock");
};

static void
clock_format_changed (GSettings    *settings,
                      gchar        *key,
                      KiranClock   *clock)
{  
    KiranClockPrivate *priv;
    int new_format;

    priv = kiran_clock_get_instance_private (clock);
    new_format = g_settings_get_enum (settings, key);

    if (new_format == priv->clock_format)
	return;

    priv->clock_format = new_format;

    kiran_clock_refresh (clock);
}

static void
long_date_format_changed (GSettings    *settings,
                      gchar        *key,
                      KiranClock   *clock)
{
    KiranClockPrivate *priv;

    priv = kiran_clock_get_instance_private (clock);

    g_free ( priv->long_date_format);
    priv->long_date_format = g_settings_get_string (settings, key);

    kiran_clock_refresh (clock);
}

static void
short_date_format_changed (GSettings    *settings,
                      gchar        *key,
                      KiranClock   *clock)
{
    KiranClockPrivate *priv;

    priv = kiran_clock_get_instance_private (clock);

    g_free ( priv->short_date_format);
    priv->short_date_format = g_settings_get_string (settings, key);

    kiran_clock_refresh (clock);
}

static void
show_seconds_changed (GSettings    *settings,
                      gchar        *key,
                      KiranClock   *clock)
{
    KiranClockPrivate *priv;
    gint timeout = 60000;

    priv = kiran_clock_get_instance_private (clock);
    priv->show_second = g_settings_get_boolean(priv->settings, KEY_SHOW_SECONDS);

    if (priv->show_second)
        timeout = 1000;

    if (priv->refresh_timeout)
    {
        g_source_remove (priv->refresh_timeout);
    }

    priv->refresh_timeout = gdk_threads_add_timeout (timeout,
                                                   kiran_clock_refresh,
                                                   clock);
    g_source_set_name_by_id (priv->refresh_timeout, "[kiran]kiran_clock_refresh");

    kiran_clock_refresh (clock);
}

static void
kiran_clock_init (KiranClock *clock)
{
    KiranClockPrivate *priv;
    time_t timet;

    priv = clock->priv = kiran_clock_get_instance_private (clock);
    priv->refresh_timeout = 0;

    time (&timet);
    localtime_r (&timet, &priv->time);

    priv->settings = g_settings_new (CALENDAR_SCHEMA);

    priv->clock_format = g_settings_get_enum (priv->settings, KEY_CLOCK_FORMAT);
    priv->long_date_format = g_settings_get_string (priv->settings, KEY_LONG_DATE_FORMAT);
    priv->short_date_format =g_settings_get_string (priv->settings, KEY_SHORT_DATE_FORMAT);
    priv->show_second = g_settings_get_boolean(priv->settings, KEY_SHOW_SECONDS);

    g_signal_connect (priv->settings, "changed::" KEY_CLOCK_FORMAT, G_CALLBACK (clock_format_changed), clock);
    g_signal_connect (priv->settings, "changed::" KEY_LONG_DATE_FORMAT, G_CALLBACK (long_date_format_changed), clock);
    g_signal_connect (priv->settings, "changed::" KEY_SHORT_DATE_FORMAT, G_CALLBACK (short_date_format_changed), clock);
    g_signal_connect (priv->settings, "changed::" KEY_SHOW_SECONDS, G_CALLBACK (show_seconds_changed), clock);

    priv->is_hover = FALSE;
}

static void 
kiran_clock_finalize (GObject *object)
{
    KiranClockPrivate *priv = KIRAN_CLOCK (object)->priv;

    if (priv->settings)
        g_object_unref (priv->settings);
    priv->settings = NULL;

    if (priv->refresh_timeout)
    {
        g_source_remove (priv->refresh_timeout);
        priv->refresh_timeout = 0;
    }

    g_free ( priv->long_date_format);
    g_free ( priv->short_date_format);

    G_OBJECT_CLASS (kiran_clock_parent_class)->finalize (object);
}
static gboolean 
kiran_clock_draw (GtkWidget *widget,
		  cairo_t   *cr)
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

    gtk_widget_style_get(widget,
                         "text-color", &tcolor,
                         NULL);

    gtk_widget_style_get(widget,
                         "text-font", &tfont,
                         NULL);

    priv = KIRAN_CLOCK (widget)->priv;

    cairo_save (cr);
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_save (context);

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

    if (priv->is_hover)
    {
    	//draw background
	GdkRectangle rect;
	GdkRGBA color;

        cairo_save (cr);

        rect.x = 0;
        rect.y = 0;
        rect.width = width;
        rect.height = height;
	gtk_style_context_lookup_color(context, "clock_box_hover_color", &color);
        paint_round_rectangle (cr, &rect, 0.33, 0.54, 0.98, 1, 1.0, color.red, color.green, color.blue, color.alpha, 2, FALSE, TRUE);
	
        cairo_restore (cr);
    }
    if (priv->show_second)
    {
	if (priv->clock_format == CLOCK_FORMAT_24)
   	    strftime (buffer, sizeof (buffer), "%H:%M:%S", &(priv->time));
	else
	    strftime (buffer, sizeof (buffer), "%p %l:%M:%S", &(priv->time));
    }
    else
    {
	if (priv->clock_format == CLOCK_FORMAT_24)
   	    strftime (buffer, sizeof (buffer), "%H:%M", &(priv->time));
	else
   	    strftime (buffer, sizeof (buffer), "%p %l:%M", &(priv->time));
    }
	
    markup = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);

    time_layout = gtk_widget_create_pango_layout (widget, buffer);
    pango_layout_set_alignment (time_layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup (time_layout, markup, -1);
    g_free (markup);


    if (g_strcmp0 (priv->short_date_format, "") == 0)
    {
	//默认格式
	if (g_getenv ("LANG") && g_strrstr (g_getenv ("LANG"), "zh_CN"))
            strftime (buffer, sizeof (buffer), "%Y/%m/%d", &(priv->time));
	else
            strftime (buffer, sizeof (buffer), "%m/%d/%Y", &(priv->time));
    }
    else
    {
	strftime (buffer, sizeof (buffer), priv->short_date_format, &(priv->time));
    }

    markup = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);
    date_layout = gtk_widget_create_pango_layout (widget, buffer);
    pango_layout_set_alignment (date_layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup (date_layout, markup, -1);
    g_free (markup);

    pango_layout_get_pixel_extents (time_layout, NULL, &time_logical_rect);
    pango_layout_get_pixel_extents (date_layout, NULL, &date_logical_rect);


    y_loc = (height - (time_logical_rect.height + date_logical_rect.height)) / 2;

    x_loc = (width - time_logical_rect.width) / 2;
    gtk_render_layout (context, cr, x_loc, y_loc, time_layout);

    y_loc = y_loc + time_logical_rect.height;
    x_loc = (width - date_logical_rect.width) / 2; 
    gtk_render_layout (context, cr, x_loc, y_loc, date_layout);

    g_object_unref (time_layout);
    g_object_unref (date_layout);
 
    cairo_restore (cr);
    gtk_style_context_restore (context);

    g_free (tcolor);
    g_free (tfont);

    return FALSE;
}

static void
kiran_clock_update_tooltip (KiranClock *clock)
{
    KiranClockPrivate *priv = clock->priv;
    gchar buffer[32];

    if (g_strcmp0 (priv->long_date_format, "") == 0)
    {
	//默认格式
        if (g_getenv ("LANG") && g_strrstr (g_getenv ("LANG"), "zh_CN"))
            strftime (buffer, sizeof (buffer), "%A,%Y年%m月%d日", &(priv->time));
        else
            strftime (buffer, sizeof (buffer), "%A,%B %d,%Y", &(priv->time));
    }
    else
    {
	strftime (buffer, sizeof (buffer), priv->long_date_format, &(priv->time));
    }

    gtk_widget_set_tooltip_text (GTK_WIDGET (clock), buffer);
}

static gboolean
kiran_clock_refresh (gpointer data)
{
    KiranClock *clock = KIRAN_CLOCK (data);
    KiranClockPrivate *priv = clock->priv;
    time_t timet;

    time (&timet);
    localtime_r (&timet, &priv->time);
    kiran_clock_update_tooltip (clock);

    gtk_widget_queue_draw (GTK_WIDGET (clock));

    return TRUE;
}

static gboolean 
kiran_clock_enter_notify (GtkWidget        *widget,
                          GdkEventCrossing *event)
{
    KiranClockPrivate *priv = KIRAN_CLOCK (widget)->priv;

    priv->is_hover = TRUE;
    kiran_clock_refresh (widget);

    return TRUE;
}

static gboolean 
kiran_clock_leave_notify (GtkWidget        *widget,
                          GdkEventCrossing *event)
{
    KiranClockPrivate *priv = KIRAN_CLOCK (widget)->priv;

    priv->is_hover = FALSE;
    kiran_clock_refresh (widget);

    return TRUE;
}

static void 
kiran_clock_destroy (GtkWidget *widget)
{
    KiranClockPrivate *priv = KIRAN_CLOCK (widget)->priv;

    if (priv->refresh_timeout)
    {
        g_source_remove (priv->refresh_timeout);
        priv->refresh_timeout = 0;
    }

    GTK_WIDGET_CLASS (kiran_clock_parent_class)->destroy (widget);
}

static void
kiran_clock_map (GtkWidget *widget)
{
    KiranClock *clock = KIRAN_CLOCK (widget);
    KiranClockPrivate *priv = clock->priv;

    GTK_WIDGET_CLASS (kiran_clock_parent_class)->map (widget);

    if (!priv->refresh_timeout)
    {
	gint timeout = 60000;
        kiran_clock_refresh (clock);
	if (priv->show_second)
		timeout = 1000;

        priv->refresh_timeout = gdk_threads_add_timeout (timeout,
        				           kiran_clock_refresh,
        				           clock);	
        g_source_set_name_by_id (priv->refresh_timeout, "[kiran]kiran_clock_refresh");
    }
}

static void
kiran_clock_unmap (GtkWidget *widget)
{
    KiranClockPrivate *priv = KIRAN_CLOCK (widget)->priv;

    GTK_WIDGET_CLASS (kiran_clock_parent_class)->unmap (widget);

    if (priv->refresh_timeout)
    {
        g_source_remove (priv->refresh_timeout);
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
kiran_clock_new (void)
{
    return g_object_new (KIRAN_TYPE_CLOCK, NULL);
}
