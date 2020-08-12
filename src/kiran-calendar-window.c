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

static void
kiran_calendar_window_class_init (KiranCalendarWindowClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);

    gobject_class->constructor = kiran_calendar_window_constructor;
}

static void
kiran_calendar_window_init (KiranCalendarWindow *window)
{
    KiranCalendarWindowPrivate *priv;

    priv = window->priv = kiran_calendar_window_get_instance_private (window);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
    gtk_window_stick (GTK_WINDOW (window));
    gtk_window_set_title (GTK_WINDOW (window), "Kiran Calendar"); 
    gtk_window_set_default_size (GTK_WINDOW (window), CALENDA_WIDTH, CALENDA_HEIGHT);
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

    obj = G_OBJECT_CLASS (kiran_calendar_window_parent_class)->constructor (type,
                                                                      n_construct_properties,
                                                                      construct_properties);
    win = KIRAN_CALENDAR_WINDOW (obj);

    tran_setup (GTK_WIDGET (win));
    win->priv->calendar = kiran_calendar_new ();  
    gtk_widget_show (GTK_WIDGET (win->priv->calendar));
    gtk_container_add (GTK_CONTAINER (win), win->priv->calendar);

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
