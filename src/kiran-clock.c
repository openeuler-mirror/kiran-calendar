#include "kiran-clock.h"
#include "common.h"

struct _KiranClockPrivate
{
    struct tm time;

    gint refresh_timeout; 
    gboolean show_second;
    gboolean show_long_date_format;
    gboolean show_24_hour;
    gboolean is_hover;
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

G_DEFINE_TYPE_WITH_PRIVATE (KiranClock, kiran_clock, GTK_TYPE_EVENT_BOX)

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

   
    gtk_widget_class_set_css_name (widget_class, "kiranclock");
};

static void
kiran_clock_init (KiranClock *clock)
{
    KiranClockPrivate *priv;
    time_t timet;

    priv = clock->priv = kiran_clock_get_instance_private (clock);
    priv->refresh_timeout = 0;

    time (&timet);
    localtime_r (&timet, &priv->time);

    priv->show_second = FALSE;
    priv->show_long_date_format = FALSE;
    priv->show_24_hour = TRUE;
    priv->is_hover = FALSE;
}

static void 
kiran_clock_finalize (GObject *object)
{
    KiranClockPrivate *priv = KIRAN_CLOCK (object)->priv;

    if (priv->refresh_timeout)
    {
        g_source_remove (priv->refresh_timeout);
        priv->refresh_timeout = 0;
    }

    G_OBJECT_CLASS (kiran_clock_parent_class)->finalize (object);
}
static gboolean 
kiran_clock_draw (GtkWidget *widget,
		  cairo_t   *cr)
{
    KiranClockPrivate *priv;
    gint width, height;
    GtkStyleContext *context;
    PangoLayout *layout;
    PangoRectangle logical_rect;
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
    height = gtk_widget_get_allocated_width (widget);

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
        paint_round_rectangle (cr, &rect, 0.33, 0.54, 0.98, 1, color.red, color.green, color.blue, color.alpha, 2, FALSE, TRUE);
	
        cairo_restore (cr);
    }
    if (priv->show_second)
    {
	if (priv->show_24_hour)
   	    strftime (buffer, sizeof (buffer), "%H:%M:%S", &(priv->time));
	else
	    strftime (buffer, sizeof (buffer), "%p %l:%M:%S", &(priv->time));
    }
    else
    {
	if (priv->show_24_hour)
   	    strftime (buffer, sizeof (buffer), "%H:%M", &(priv->time));
	else
   	    strftime (buffer, sizeof (buffer), "%p %l:%M", &(priv->time));
    }
	
    markup = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);

    layout = gtk_widget_create_pango_layout (widget, buffer);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup (layout, markup, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    x_loc = (width - logical_rect.width) / 2;
    y_loc = 0;

    gtk_render_layout (context, cr, x_loc, y_loc, layout);

    g_free (markup);
    g_object_unref (layout);

    y_loc = y_loc + logical_rect.height;

    if (priv->show_long_date_format)
    {
	if (g_getenv ("LANG") && g_strrstr (g_getenv ("LANG"), "zh_CN"))
	    strftime (buffer, sizeof (buffer), "%Y年%m月%d日", &(priv->time));
	else
	    strftime (buffer, sizeof (buffer), "%B %d, %Y", &(priv->time));
    }
    else
        strftime (buffer, sizeof (buffer), "%Y-%m-%d", &(priv->time));

    markup = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);
    layout = gtk_widget_create_pango_layout (widget, buffer);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup (layout, markup, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    x_loc = (width - logical_rect.width) / 2; 

    gtk_render_layout (context, cr, x_loc, y_loc, layout);

    g_free (markup);
    g_object_unref (layout);
 
    cairo_restore (cr);
    gtk_style_context_restore (context);

    g_free (tcolor);
    g_free (tfont);

    return FALSE;
}

static gboolean
kiran_clock_refresh (gpointer data)
{
    KiranClock *clock = KIRAN_CLOCK (data);
    KiranClockPrivate *priv = clock->priv;
    time_t timet;

    time (&timet);
    localtime_r (&timet, &priv->time);

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
