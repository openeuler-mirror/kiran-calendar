#include <mate-panel-applet.h>
#include <gdk/gdkx.h>

#include "kiran-clock.h"
#include "kiran-calendar-window.h"
#include "config.h"
#include <glib/gi18n.h>
#include <locale.h>

#define CLOCK_ICON "mate-panel-clock"

typedef struct _KiranClockData KiranClockData;

struct _KiranClockData
{
    MatePanelApplet *applet;
    GtkWidget *clock;
    GtkWidget *calendar_window;
    MatePanelAppletOrient  orient;
};

static void display_about_dialog      (GtkAction       *action,
                                       KiranClockData  *kcd);

static const GtkActionEntry kiran_clock_menu_actions [] = {
	{ "KiranClockAbout", "kiran-help-about", N_("_About"),
          NULL, NULL,
          G_CALLBACK (display_about_dialog) }
};

static void 
display_about_dialog(GtkAction       *action, 
		     KiranClockData  *kcd)
{
        static const gchar* authors[] = {
                "wangxiaoqing <wangxiaoqing@kylinos.com.cn>",
                "songchuanfei <songchuanfei@kylinos.com.cn>",
                NULL
        };

        static const char* documenters[] = {
                "HuNan Kylin <www@kylinsec.com.cn>",
                NULL
        };

        gtk_show_about_dialog(NULL,
                "program-name", _("Clock"),
                "title", _("About Clock"),
                "authors", authors,
                "comments", _("The Clock displays the current time and date"),
                "copyright", _("Copyright ©2020 KylinSec. All rights reserved."),
                "documenters", documenters,
                "logo-icon-name", CLOCK_ICON,
                "translator-credits", _("translator-credits"),
                "version", VERSION,
                "website", "http://www.kylinsec.com.cn",
                NULL);
}

static void
position_calendar_window (KiranClockData *kcd)
{
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
    switch (kcd->orient) {
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
    if (event->button != 1)
    {
	g_signal_stop_emission_by_name (widget, "button_press_event");
    }

    return FALSE;
}

static void
button_toggled (GtkWidget      *widget,
		gpointer       user_data)
{
    KiranClockData *kcd  = user_data;
    KiranCalendarWindow *window  = KIRAN_CALENDAR_WINDOW (kcd->calendar_window);

    gtk_widget_set_has_tooltip (widget, FALSE);

    if (gtk_widget_is_visible (GTK_WIDGET (window)))
        kiran_calendar_window_hide (window);
    else
    {
        kiran_calendar_window_show (window);
 	position_calendar_window (kcd);
    }
}

static void
destroy_clock (GtkWidget *widget, 
	       gpointer   user_data)
{
    KiranClockData *kcd  = user_data;

    gtk_widget_destroy (GTK_WIDGET(kcd->calendar_window));

    g_free (kcd);
}

static void
applet_change_orient (MatePanelApplet       *applet,
                      MatePanelAppletOrient  orient,
                      KiranClockData         *kcd)
{
    if (orient == kcd->orient)
            return;

    kcd->orient = orient;
}

static void
applet_change_size (MatePanelApplet *applet,
                    gint            size,
                    KiranClockData  *kcd)
{
    gint width;

    width = 0;

    gtk_widget_style_get(kcd->clock,
                         "box-width", &width,
                         NULL);

    gtk_widget_set_size_request (kcd->clock, 
		                 width, 
		    		 mate_panel_applet_get_size (kcd->applet));

    gtk_widget_queue_draw (kcd->clock);
}

static gboolean
fill_clock_applet (MatePanelApplet *applet)
{
    KiranClockData *kcd;
    GtkActionGroup *action_group;
    GtkAction      *action;

    kcd = g_new0 (KiranClockData, 1);

    kcd->applet = applet;
    kcd->calendar_window = kiran_calendar_window_new ();
    kcd->clock = kiran_clock_new (); 

    gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (kcd->clock),TRUE);
    gtk_container_add (GTK_CONTAINER (applet), kcd->clock);
    gtk_widget_show (kcd->clock);
    gtk_widget_show (GTK_WIDGET (kcd->applet));
    applet_change_size (kcd->applet, mate_panel_applet_get_size (kcd->applet), kcd);

    g_signal_connect (G_OBJECT (kcd->clock), "button-press-event", G_CALLBACK(button_press), kcd);
    g_signal_connect (G_OBJECT (kcd->clock), "toggled", G_CALLBACK(button_toggled), kcd);
    g_signal_connect (G_OBJECT (kcd->clock), "destroy", G_CALLBACK (destroy_clock), kcd);

    g_signal_connect (G_OBJECT (applet),
                          "change_orient",
                          G_CALLBACK (applet_change_orient),
                          kcd);

    g_signal_connect (G_OBJECT (applet),
                          "change-size",
                          G_CALLBACK (applet_change_size),
                          kcd);

    applet_change_orient (MATE_PANEL_APPLET (applet),
                              mate_panel_applet_get_orient (MATE_PANEL_APPLET (applet)),
                              kcd);

    action_group = gtk_action_group_new ("KiranClockApplet Menu Actions");
    gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
    gtk_action_group_add_actions (action_group,
                                  kiran_clock_menu_actions,
                                  G_N_ELEMENTS (kiran_clock_menu_actions),
                                  kcd);
    mate_panel_applet_setup_menu (MATE_PANEL_APPLET (kcd->applet),
                                  "<menuitem name=\"Kiran Clock About Item\" action=\"KiranClockAbout\" />",
                                  action_group);


    return TRUE;
}

static gboolean
kiran_clock_factory (MatePanelApplet *applet,
		     const char *iid,
		     gpointer data)
{
    gboolean retval = FALSE;
    GtkCssProvider *provider;
    GdkScreen *screen;
    GdkDisplay *display;
    GFile *css_fp;

    setlocale(LC_ALL, "");

    /* 设置本地时间， 对于GB18030字符集和其它字符集合设置为UTF-8 */
    if (g_getenv("LANG") && g_strrstr(g_getenv ("LANG"), "zh_CN"))
    {
        setlocale(LC_TIME, "zh_CN.UTF-8");
    }

    bindtextdomain (GETTEXT_PACKAGE, KIRAN_CALENDAR_DATE_LOCALEDIR);
    textdomain (GETTEXT_PACKAGE);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

    mate_panel_applet_set_flags (applet, MATE_PANEL_APPLET_EXPAND_MINOR | MATE_PANEL_APPLET_HAS_HANDLE);

    provider = gtk_css_provider_new ();
    display = gdk_display_get_default ();
    screen = gdk_display_get_default_screen (display);
    gtk_style_context_add_provider_for_screen (screen,
                                             GTK_STYLE_PROVIDER(provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
    css_fp  = g_file_new_for_path ("/usr/share/kiran-calendar/kiran-calendar.css");
    gtk_css_provider_load_from_file (provider, css_fp, NULL);

    if (!strcmp (iid, "KiranClockApplet"))
	retval = fill_clock_applet (applet);

    g_object_unref (provider);
    g_object_unref (css_fp);

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
