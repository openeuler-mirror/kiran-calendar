#include <mate-panel-applet.h>
#include <gdk/gdkx.h>

#include "kiran-clock.h"
#include "kiran-calendar-window.h"

typedef struct _KiranClockData KiranClockData;

struct _KiranClockData
{
    MatePanelApplet *applet;
    GtkWidget *clock;
    GtkWidget *calendar_window;
};

static void
position_calendar_window (KiranClockData *kcd)
{
    GtkRequisition  req;
    GtkAllocation   allocation;
    GdkDisplay     *display;
    GdkScreen      *screen;
    GdkRectangle    monitor;
    GdkGravity      gravity = GDK_GRAVITY_NORTH_WEST;
    int             button_w, button_h;
    int             x, y;
    int             w, h;
    int             i, n;
    gboolean        found_monitor = FALSE;
 
    if (!GDK_IS_X11_DISPLAY (gdk_display_get_default ()))
            return;
 
    /* Get root origin of the toggle button, and position above that. */
    gdk_window_get_origin (gtk_widget_get_window (kcd->clock),
                           &x, &y);
 
    gtk_window_get_size (GTK_WINDOW (kcd->calendar_window), &w, &h);
    gtk_widget_get_preferred_size (kcd->calendar_window, &req, NULL);
    w = req.width;
    h = req.height;
 
    gtk_widget_get_allocation (kcd->clock, &allocation);
    button_w = allocation.width;
    button_h = allocation.height;
 
    screen = gtk_window_get_screen (GTK_WINDOW (kcd->calendar_window));
    display = gdk_screen_get_display (screen);
 
    n = gdk_display_get_n_monitors (display);
    for (i = 0; i < n; i++) {
            gdk_monitor_get_geometry (gdk_display_get_monitor (display, i), &monitor);
            if (x >= monitor.x && x <= monitor.x + monitor.width &&
                y >= monitor.y && y <= monitor.y + monitor.height) {
                    found_monitor = TRUE;
                    break;
            }
    }
 
    if (!found_monitor) {
            /* eek, we should be on one of those xinerama
               monitors */
            monitor.x = 0;
            monitor.y = 0;
            monitor.width = WidthOfScreen (gdk_x11_screen_get_xscreen (screen));
            monitor.height = HeightOfScreen (gdk_x11_screen_get_xscreen (screen));
    }
 
    /* Based on panel orientation, position the popup.
     * Ignore window gravity since the window is undecorated.
     * The orientations are all named backward from what
     * I expected.
     */
    switch (mate_panel_applet_get_orient (kcd->applet)) {
    case MATE_PANEL_APPLET_ORIENT_RIGHT:
            x += button_w;
            if ((y + h) > monitor.y + monitor.height)
                    y -= (y + h) - (monitor.y + monitor.height);
 
            if ((y + h) > (monitor.height / 2))
                    gravity = GDK_GRAVITY_SOUTH_WEST;
            else
                    gravity = GDK_GRAVITY_NORTH_WEST;
 
            break;
    case MATE_PANEL_APPLET_ORIENT_LEFT:
            x -= w;
            if ((y + h) > monitor.y + monitor.height)
                    y -= (y + h) - (monitor.y + monitor.height);
 
            if ((y + h) > (monitor.height / 2))
                    gravity = GDK_GRAVITY_SOUTH_EAST;
            else
                    gravity = GDK_GRAVITY_NORTH_EAST;
 
            break;
    case MATE_PANEL_APPLET_ORIENT_DOWN:
            y += button_h;
            if ((x + w) > monitor.x + monitor.width)
                    x -= (x + w) - (monitor.x + monitor.width);
 
            gravity = GDK_GRAVITY_NORTH_WEST;
 
            break;
    case MATE_PANEL_APPLET_ORIENT_UP:
            y -= h;
            if ((x + w) > monitor.x + monitor.width)
                    x -= (x + w) - (monitor.x + monitor.width);
 
            gravity = GDK_GRAVITY_SOUTH_WEST;
 
            break;
    }
 
    gtk_window_move (GTK_WINDOW (kcd->calendar_window), x, y);
    gtk_window_set_gravity (GTK_WINDOW (kcd->calendar_window), gravity);
}

static gboolean
button_press (GtkWidget      *widget,
              GdkEventButton *event,
              gpointer       user_data)
{
    KiranClockData *kcd  = user_data;
    KiranCalendarWindow *window  = KIRAN_CALENDAR_WINDOW (kcd->calendar_window);

    if (event->button == GDK_BUTTON_PRIMARY)
    {
        if (gtk_widget_is_visible (GTK_WIDGET (window)))
            kiran_calendar_window_hide (window);
        else
	{
	    position_calendar_window (kcd);
            kiran_calendar_window_show (window);
	}
    }

    return TRUE;
}

static void
destroy_clock (GtkWidget *widget, 
	       gpointer   user_data)
{
    KiranClockData *kcd  = user_data;

    gtk_widget_destroy (GTK_WIDGET(kcd->calendar_window));

    g_free (kcd);
}

static gboolean
fill_clock_applet (MatePanelApplet *applet)
{
    KiranClockData *kcd;

    kcd = g_new0 (KiranClockData, 1);

    kcd->applet = applet;
    kcd->calendar_window = kiran_calendar_window_new ();
    kcd->clock = kiran_clock_new (); 

    gtk_container_add (GTK_CONTAINER (applet), kcd->clock);
    gtk_widget_show (kcd->clock);
    gtk_widget_show (GTK_WIDGET (kcd->applet));
    gtk_widget_set_size_request (kcd->clock, 80, mate_panel_applet_get_size (kcd->applet));

    g_signal_connect (G_OBJECT (kcd->clock), "button-press-event", G_CALLBACK(button_press), kcd);
    g_signal_connect (G_OBJECT (kcd->clock), "destroy", G_CALLBACK (destroy_clock), kcd);

    return TRUE;
}

static gboolean
kiran_clock_factory (MatePanelApplet *applet,
		     const char *iid,
		     gpointer data)
{
    gboolean retval = FALSE;

    if (!strcmp (iid, "KiranClockApplet"))
	retval = fill_clock_applet (applet);
    return retval;
}

#ifdef CLOCK_INPROCESS
MATE_PANEL_APPLET_IN_PROCESS_FACTORY ("KiranClockAppletFactory",
                                 PANEL_TYPE_APPLET,
                                 "KiranClockApplet",
                                 kiran_clock_factory,
                                 NULL)
#else
MATE_PANEL_APPLET_OUT_PROCESS_FACTORY ("KiranClockAppletFactory",
                                  PANEL_TYPE_APPLET,
                                  "KiranClockApplet",
                                  kiran_clock_factory,
                                  NULL)
#endif
