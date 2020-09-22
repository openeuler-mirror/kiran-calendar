#include "config.h"
#include <lunar-date/lunar-date.h>
#include <glib/gi18n-lib.h>

#include "kiran-calendar.h"
#include "common.h"

#define STANDER_DPI 96.0

#define CALENDA_LINE_WIDTH 1

#define TODAY_WIN_WIDTH 36 
#define TODAY_WIN_HEIGHT 36

#define LUNAR_LEFT_DIS 20.0
#define LUNAR_TOP_DIS  28.0
#define LUNAR_SPACE 8.0

#define SEPARATE_LINE_TOP_DIS 74.0
#define SEPARATE_LINE_LR_DIS 12

#define HEADER_IMAGES 12
#define HEADER_LEFT_DIS 20
#define HEADER_RIGHT_DIS 20
#define HEADER_ARROW_SPACE 16
#define HEADER_TOP_DIS (SEPARATE_LINE_TOP_DIS + 24)
#define HEADER_TEXT_LEFT_DIS 102
#define HEADER_TEXT_SPACE 10
#define HEADER_TEXT_TOP_DIS (HEADER_TOP_DIS - 6)

#define YEAR_TEXT_AREA_WIDTH 44
#define YEAR_TEXT_AREA_HEIGHT 28

#define MONTH_TEXT_AREA_WIDTH 30
#define MONTH_TEXT_AREA_HEIGHT 28
#define MONTH_TEXT_AREA_LEFT_DIS (HEADER_TEXT_LEFT_DIS + 64)

#define DAY_NAME_LEFT_DIS 28
#define DAY_NAME_TOP_DIS (HEADER_TOP_DIS + 36)

#define SEPARATE_LINE_TOP_DIS_1 (DAY_NAME_TOP_DIS + 16 + 20) 

#define DAY_COL 7
#define DAY_ROW 6
#define DAY_ROW_SPACE 16
#define DAY_COL_SPACE 2
#define DAY_COL_LEFT_DIS 10
#define DAY_RECT_WIDTH 38
#define DAY_RECT_HEIGHT 34
#define DAY_RECT_LINE_WIDTH 2
#define LUNAR_STR_LEN 256
#define DAY_ROW_TOP_DIS (SEPARATE_LINE_TOP_DIS_1 + 17)

#define YEAR_INPUT_MAX 5
#define MONTH_INPUT_MAX 3
#define INVALIDE_NUM -1

#define BLINK_TIMEOUT 500
#define SELECT_BOX_COLOR_Y_DIS 2

#define ARROW_WIDTH 6
#define ARROW_HEIGHT 12
#define ARROW_SPACE 4

#define SETTING_BTN_SVG_PATH "/usr/share/kiran-calendar/setting_btn.svg"
#define SETTING_BTN_SVG_W 16
#define SETTING_BTN_SVG_H 16
#define SETTING_BTN_RIGHT_DIS 18
#define SETTING_BTN_TOP_DIS  34

typedef enum
{
     KIRAN_NORMAL,
     KIRAN_PRESS,
     KIRAN_HOVER
}KiranCalendarState;


enum {
    MONTH_PREV,
    MONTH_CURRENT,
    MONTH_NEXT
};

enum {
    ARROW_YEAR_PREV = 0,
    ARROW_MONTH_PREV,
    ARROW_YEAR_NEXT,
    ARROW_MONTH_NEXT
};

enum {
    IMAGE_ARROW_YEAR_PREV_N = 0,
    IMAGE_ARROW_YEAR_PREV_P,
    IMAGE_ARROW_YEAR_PREV_H,
    IMAGE_ARROW_YEAR_NEXT_N,
    IMAGE_ARROW_YEAR_NEXT_P,
    IMAGE_ARROW_YEAR_NEXT_H,
    IMAGE_ARROW_MONTH_PREV_N,
    IMAGE_ARROW_MONTH_PREV_P,
    IMAGE_ARROW_MONTH_PREV_H,
    IMAGE_ARROW_MONTH_NEXT_N,
    IMAGE_ARROW_MONTH_NEXT_P,
    IMAGE_ARROW_MONTH_NEXT_H
};


struct _KiranCalendarPrivate
{
    GdkWindow *main_win;
    GdkWindow *day_win;
    GdkWindow *today_win;
    GdkWindow *setting_btn_win;
    GdkWindow *arrow_win[4];
    GdkWindow *year_text_area;
    GdkWindow *month_text_area;

    LunarDate *lundate;
    gboolean  show_lunar_calendar;

    gint month;
    gint year;
    gint selected_day;
    gint hover_day_row;
    gint hover_day_col;

    gint week_start;

    gint day_month[DAY_ROW][DAY_COL];
    gint day[DAY_ROW][DAY_COL];
    gchar day_lunar[DAY_ROW][DAY_COL][LUNAR_STR_LEN];

    gchar lunar[3][LUNAR_STR_LEN];

    KiranCalendarState prev_year_state;
    KiranCalendarState next_year_state;
    KiranCalendarState prev_month_state;
    KiranCalendarState next_month_state;
    KiranCalendarState today_state;
    KiranCalendarState setting_btn_state;

    gboolean year_text_input;
    gboolean month_text_input;

    gboolean year_input;
    gboolean month_input;

    gchar year_text[YEAR_INPUT_MAX];
    gchar month_text[MONTH_INPUT_MAX];

    gboolean cursor_visible;
    gint blink_timeout; 

    gboolean year_text_select;
    gboolean month_text_select;

    GdkPixbuf *setting_btn_svg;
};

static const guint month_length[2][13] =
{
    { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static char    *default_abbreviated_dayname[7];
static gint    year_text_length = 0;
static gint    month_text_length = 0;
static gint    year_text_height = 0;

static void kiran_calendar_finalize (GObject *object);
static void kiran_calendar_destroy (GtkWidget *widget);
static void kiran_calendar_realize (GtkWidget *widget);
static void kiran_calendar_unrealize (GtkWidget *widget);
static void kiran_calendar_map (GtkWidget *widget);
static void kiran_calendar_unmap (GtkWidget *widget);
static gboolean kiran_calendar_draw (GtkWidget *widget,
				     cairo_t   *cr);
static gboolean kiran_calendar_button_press (GtkWidget      *widget,
                                             GdkEventButton *event);
static gboolean kiran_calendar_button_release (GtkWidget      *widget,
                                               GdkEventButton *event);
static gboolean kiran_calendar_enter_notify (GtkWidget      *widget,
                                             GdkEventCrossing *event);
static gboolean kiran_calendar_leave_notify (GtkWidget      *widget,
                                             GdkEventCrossing *event);
static gboolean kiran_calendar_motion_notify (GtkWidget      *widget,
                                              GdkEventMotion *event);
static gboolean kiran_calendar_scroll (GtkWidget      *widget,
                                       GdkEventScroll *event);
static gboolean kiran_calendar_key_press (GtkWidget      *widget,
	                                  GdkEventKey    *event);
static gboolean kiran_calendar_focus_in (GtkWidget      *widget,
	                                 GdkEventFocus  *event);
static gboolean kiran_calendar_focus_out (GtkWidget      *widget,
	                                 GdkEventFocus  *event);

static void calendar_compute_today_lunar (KiranCalendar *calendar);
static void calendar_compute_days (KiranCalendar *calendar);
static void calendar_invalidate_day_num (KiranCalendar *calendar,
					 gint           day);
static void calendar_invalidate_day (KiranCalendar *calendar,
                                     gint           row,
                                     gint           col);

static void calendar_redraw_component (KiranCalendar *calendar,
                                   GdkWindow     *win);
static void calendar_arrows_change_state (KiranCalendar *calendar,
                                          GdkWindow *event_window,
					  KiranCalendarState state);

static gboolean
leap (guint year)
{
  return ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0));
}

static guint
day_of_week (guint year, guint mm, guint dd)
{
  GDateTime *dt;
  guint days;

  dt = g_date_time_new_local (year, mm, dd, 1, 1, 1);
  if (dt == NULL)
    return 0;

  days = g_date_time_get_day_of_week (dt);
  g_date_time_unref (dt);

  return days;
}

G_DEFINE_TYPE_WITH_PRIVATE (KiranCalendar, kiran_calendar, GTK_TYPE_WIDGET)

static void
kiran_calendar_class_init (KiranCalendarClass *class)
{
    GtkWidgetClass *widget_class;
    GObjectClass *gobject_class;

    gobject_class = (GObjectClass *) class;
    widget_class = (GtkWidgetClass *) class; 

    gobject_class->finalize = kiran_calendar_finalize;

    widget_class->destroy = kiran_calendar_destroy;
    widget_class->realize = kiran_calendar_realize;
    widget_class->unrealize = kiran_calendar_unrealize;
    widget_class->map = kiran_calendar_map;
    widget_class->unmap = kiran_calendar_unmap;
    widget_class->draw = kiran_calendar_draw;
    widget_class->button_press_event = kiran_calendar_button_press;
    widget_class->button_release_event = kiran_calendar_button_release;
    widget_class->enter_notify_event = kiran_calendar_enter_notify;
    widget_class->leave_notify_event = kiran_calendar_leave_notify;
    widget_class->motion_notify_event = kiran_calendar_motion_notify;
    widget_class->scroll_event = kiran_calendar_scroll;
    widget_class->key_press_event = kiran_calendar_key_press;
    widget_class->focus_in_event = kiran_calendar_focus_in;
    widget_class->focus_out_event = kiran_calendar_focus_out;

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("today-calendar-color",
                                                                "Today calendar color",
                                                                "Today calendar color",
                                                                "#ffffff",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("today-calendar-button-normal-color",
                                                                "Today calendar button normal color",
                                                                "Today calendar button normal color",
                                                                "#ffffff",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("today-calendar-button-press-color",
                                                                "Today calendar button press color",
                                                                "Today calendar button press color",
                                                                "#ffffff",
                                                                G_PARAM_READABLE));
    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("today-calendar-button-hover-color",
                                                                "Today calendar button hover color",
                                                                "Today calendar button hover color",
                                                                "#ffffff",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("day-current-month-color",
                                                                "Day current month color",
                                                                "Day current month color",
                                                                "#ffffff",
                                                                G_PARAM_READABLE));
    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("day-other-month-color",
                                                                "Day other month color",
                                                                "Day other month color",
                                                                "#ffffff",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("day-names-color",
                                                                "Day names color",
                                                                "Day names color",
                                                                "#ffffff",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("day-head-color",
                                                                "Day head color",
                                                                "Day head color",
                                                                "#ffffff",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("today-font",
                                                                "Today font",
                                                                "Today font",
                                                                "",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("day-box-font",
                                                                "Day box font",
                                                                "Day box font",
                                                                "",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("day-box-lunar-font",
                                                                "Day box lunar font",
                                                                "Day box lunar font",
                                                                "",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("day-names-font",
                                                                "Day names font",
                                                                "Day names font",
                                                                "",
                                                                G_PARAM_READABLE));

    gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_string ("day-head-font",
                                                                "Day head font",
                                                                "Day head font",
                                                                "",
                                                                G_PARAM_READABLE));


    gtk_widget_class_set_css_name (widget_class, "kirancalendar");
}

static void
kiran_calendar_date_init_i18n ()
{
    static gboolean _kiran_calendar_gettext_initialized = FALSE;
    if (!_kiran_calendar_gettext_initialized)
    {
	_kiran_calendar_gettext_initialized = TRUE;
        bindtextdomain (GETTEXT_PACKAGE, KIRAN_CALENDAR_DATE_LOCALEDIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
    }
}

static void
kiran_calendar_init (KiranCalendar *calendar)
{
    KiranCalendarPrivate *priv;
    char buffer[255];
    time_t tmp_time;
    gint i;

    kiran_calendar_date_init_i18n ();

    gtk_widget_set_can_focus (GTK_WIDGET (calendar), TRUE);
    gtk_widget_set_has_window (GTK_WIDGET (calendar), TRUE);

    priv = calendar->priv = kiran_calendar_get_instance_private (calendar);

    if (!default_abbreviated_dayname[0])
    for (i=0; i<7; i++)
    {
#ifndef G_OS_WIN32
        tmp_time= (i+3)*86400;
        strftime (buffer, sizeof (buffer), "%a", gmtime (&tmp_time));
        default_abbreviated_dayname[i] = g_locale_to_utf8 (buffer, -1, NULL, NULL, NULL);
#else
        if (!GetLocaleInfoW (GetThreadLocale (), LOCALE_SABBREVDAYNAME1 + (i+6)%7,
                             wbuffer, G_N_ELEMENTS (wbuffer)))
            default_abbreviated_dayname[i] = g_strdup_printf ("(%d)", i);
        else
            default_abbreviated_dayname[i] = g_utf16_to_utf8 (wbuffer, -1, NULL, NULL, NULL);
#endif
    }

    priv->setting_btn_svg = gdk_pixbuf_new_from_file_at_size (SETTING_BTN_SVG_PATH, 
		    					      SETTING_BTN_SVG_W, SETTING_BTN_SVG_H, 
							      NULL);

    kiran_calendar_refresh (calendar);
}

static void 
kiran_calendar_finalize (GObject *object)
{
    KiranCalendarPrivate *priv = KIRAN_CALENDAR (object)->priv;

    if (priv->lundate)
	lunar_date_free (priv->lundate);
    
    if (priv->blink_timeout)
        g_source_remove (priv->blink_timeout);

    if (priv->setting_btn_svg)
    {
	g_object_unref (priv->setting_btn_svg);
    }

    G_OBJECT_CLASS (kiran_calendar_parent_class)->finalize (object);
}

static void
calendar_realize_arrows (KiranCalendar *calendar)
{
    GtkWidget *widget = GTK_WIDGET (calendar);
    KiranCalendarPrivate *priv = calendar->priv;
    GdkWindowAttr attributes;
    gint attributes_mask;
    gint x;
    gint w;
    gint h;
    GtkAllocation allocation;

    gtk_widget_get_allocation (widget, &allocation);

    attributes.wclass = GDK_INPUT_ONLY;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.event_mask = gtk_widget_get_events(widget)
                            | GDK_BUTTON_PRESS_MASK
                            | GDK_BUTTON_RELEASE_MASK
                            | GDK_ENTER_NOTIFY_MASK
                            | GDK_POINTER_MOTION_MASK
                            | GDK_SCROLL_MASK
                            | GDK_LEAVE_NOTIFY_MASK;

    attributes_mask = GDK_WA_X | GDK_WA_Y;

    //prev year arrow
    x = HEADER_LEFT_DIS;
    w = ARROW_SPACE + ARROW_WIDTH;
    h = ARROW_HEIGHT;
    attributes.x = allocation.x + x;
    attributes.y = allocation.y + HEADER_TOP_DIS;
    attributes.width = w;
    attributes.height = h;

    priv->arrow_win[ARROW_YEAR_PREV] = gdk_window_new (gtk_widget_get_window (widget),
				         &attributes,
                                         attributes_mask);
    gtk_widget_register_window (widget, priv->arrow_win[ARROW_YEAR_PREV]);


    //prev month arrow
    x = x + w + HEADER_ARROW_SPACE;
    w = ARROW_WIDTH;
    h = ARROW_HEIGHT;
    attributes.x = allocation.x + x;
    attributes.y = allocation.y + HEADER_TOP_DIS;
    attributes.width = w;
    attributes.height = h;
    priv->arrow_win[ARROW_MONTH_PREV] = gdk_window_new (gtk_widget_get_window (widget),
				         &attributes,
                                         attributes_mask);
    gtk_widget_register_window (widget, priv->arrow_win[ARROW_MONTH_PREV]);

    //net year arrow
    w = ARROW_SPACE + ARROW_WIDTH;
    h = ARROW_HEIGHT;
    x = CALENDA_WIDTH - HEADER_RIGHT_DIS - w;
    attributes.x = allocation.x + x;
    attributes.y = allocation.y + HEADER_TOP_DIS;
    attributes.width = w;
    attributes.height = h;
    priv->arrow_win[ARROW_YEAR_NEXT] = gdk_window_new (gtk_widget_get_window (widget),
				         &attributes,
                                         attributes_mask);
    gtk_widget_register_window (widget, priv->arrow_win[ARROW_YEAR_NEXT]);

    //net month arrow
    w = ARROW_WIDTH;
    h = ARROW_HEIGHT;
    x = x - HEADER_ARROW_SPACE - w;
    attributes.x = allocation.x + x;
    attributes.y = allocation.y + HEADER_TOP_DIS;
    attributes.width = w;
    attributes.height = h;
    priv->arrow_win[ARROW_MONTH_NEXT] = gdk_window_new (gtk_widget_get_window (widget),
				         &attributes,
                                         attributes_mask);
    gtk_widget_register_window (widget, priv->arrow_win[ARROW_MONTH_NEXT]);

}

static void
kiran_calendar_destroy(GtkWidget *widget)
{
    KiranCalendarPrivate *priv = KIRAN_CALENDAR (widget)->priv;
    if (priv->blink_timeout)
    {
      g_source_remove (priv->blink_timeout);
      priv->blink_timeout = 0;
    }

    GTK_WIDGET_CLASS (kiran_calendar_parent_class)->destroy (widget);
}

static void
kiran_calendar_realize(GtkWidget *widget)
{
    KiranCalendarPrivate *priv;
    GtkAllocation allocation;
    GdkWindow *window;
    GdkWindowAttr attributes;
    gint attributes_mask;
    gboolean visible_window;
  
    gtk_widget_get_allocation (widget, &allocation);
  
    gtk_widget_set_realized (widget, TRUE);
  
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.event_mask = gtk_widget_get_events (widget)
                          | GDK_BUTTON_PRESS_MASK
                          | GDK_BUTTON_RELEASE_MASK
                          | GDK_ENTER_NOTIFY_MASK
                          | GDK_POINTER_MOTION_MASK
                          | GDK_SCROLL_MASK
                          | GDK_LEAVE_NOTIFY_MASK;
  
    priv = KIRAN_CALENDAR (widget)->priv;
    visible_window = gtk_widget_get_has_window (widget);
    if (visible_window)
    {
        attributes.visual = gtk_widget_get_visual (widget);
        attributes.wclass = GDK_INPUT_OUTPUT;
  
        attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
  
        priv->main_win = window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                 &attributes, attributes_mask);
        gtk_widget_set_window (widget, window);
        gtk_widget_register_window (widget, window);
    }
    else
    {
        window = gtk_widget_get_parent_window (widget);
        gtk_widget_set_window (widget, window);
        g_object_ref (window);
    }
  
    if (!visible_window)
    {
        attributes.wclass = GDK_INPUT_ONLY;
        if (!visible_window)
          attributes_mask = GDK_WA_X | GDK_WA_Y;
        else
          attributes_mask = 0;
  
        priv->main_win = gdk_window_new (window,
                                         &attributes, attributes_mask);
        gtk_widget_register_window (widget, priv->main_win);
    }
 
    attributes.wclass = GDK_INPUT_ONLY;
    attributes_mask = GDK_WA_X | GDK_WA_Y;
    attributes.y = allocation.y + DAY_ROW_TOP_DIS;
    priv->day_win = gdk_window_new (gtk_widget_get_window (widget),
				         &attributes,
                                         attributes_mask);
    gtk_widget_register_window (widget, priv->day_win);

    attributes.wclass = GDK_INPUT_ONLY;
    attributes_mask = GDK_WA_X | GDK_WA_Y;
    attributes.x = allocation.x + LUNAR_LEFT_DIS;
    attributes.y = allocation.y + LUNAR_TOP_DIS;
    attributes.width = TODAY_WIN_WIDTH;
    attributes.height = TODAY_WIN_HEIGHT;
    priv->today_win = gdk_window_new (gtk_widget_get_window (widget),
				         &attributes,
                                         attributes_mask);
    gtk_widget_register_window (widget, priv->today_win);

    attributes.x = allocation.x + (CALENDA_WIDTH - SETTING_BTN_RIGHT_DIS - SETTING_BTN_SVG_W);
    attributes.y = allocation.y + SETTING_BTN_TOP_DIS;
    priv->setting_btn_win = gdk_window_new (gtk_widget_get_window (widget),
                                         &attributes,
                                         attributes_mask);
    gtk_widget_register_window (widget, priv->setting_btn_win);

    attributes.cursor = gdk_cursor_new_from_name (gtk_widget_get_display (widget), "text");
    attributes_mask |= GDK_WA_CURSOR;
    attributes_mask |= GDK_KEY_PRESS_MASK;
    attributes.x = allocation.x + HEADER_TEXT_LEFT_DIS - (HEADER_TEXT_SPACE / 2) - 4;
    attributes.y = allocation.y + HEADER_TEXT_TOP_DIS - 4;
    attributes.width = YEAR_TEXT_AREA_WIDTH + 4;
    attributes.height = YEAR_TEXT_AREA_HEIGHT + 4;
    priv->year_text_area = gdk_window_new (gtk_widget_get_window (widget),
				         &attributes,
                                         attributes_mask);
    gtk_widget_register_window (widget, priv->year_text_area);

    attributes.x = allocation.x + MONTH_TEXT_AREA_LEFT_DIS - HEADER_TEXT_SPACE / 2 - 4;
    attributes.width = MONTH_TEXT_AREA_WIDTH + 4;
    attributes.height = MONTH_TEXT_AREA_HEIGHT + 4;
    priv->month_text_area = gdk_window_new (gtk_widget_get_window (widget),
				         &attributes,
                                         attributes_mask);
    gtk_widget_register_window (widget, priv->month_text_area);


    calendar_realize_arrows (KIRAN_CALENDAR (widget));
}

static void
kiran_calendar_unrealize(GtkWidget *widget)
{
    KiranCalendarPrivate *priv = KIRAN_CALENDAR (widget)->priv;
    gint i;
   
    if (priv->main_win != NULL )
    {
	gtk_widget_unregister_window (widget, priv->main_win);
        gdk_window_destroy (priv->main_win);
        priv->main_win = NULL;
    }

    if (priv->day_win != NULL )
    {
	gtk_widget_unregister_window (widget, priv->day_win);
        gdk_window_destroy (priv->day_win);
        priv->day_win = NULL;
    }

    if (priv->today_win != NULL )
    {
	gtk_widget_unregister_window (widget, priv->today_win);
        gdk_window_destroy (priv->today_win);
        priv->today_win = NULL;
    }

    if (priv->setting_btn_win != NULL )
    {
	gtk_widget_unregister_window (widget, priv->setting_btn_win);
        gdk_window_destroy (priv->setting_btn_win);
        priv->setting_btn_win = NULL;
    }

    if (priv->year_text_area != NULL )
    {
	gtk_widget_unregister_window (widget, priv->year_text_area);
        gdk_window_destroy (priv->year_text_area);
        priv->year_text_area = NULL;
    }

    if (priv->month_text_area != NULL )
    {
	gtk_widget_unregister_window (widget, priv->month_text_area);
        gdk_window_destroy (priv->month_text_area);
        priv->month_text_area = NULL;
    }
   
    for (i = 0; i < 4; i++)
    {
	if (priv->arrow_win[i] != NULL)
        {
 	    gtk_widget_unregister_window (widget, priv->arrow_win[i]);
            gdk_window_destroy (priv->arrow_win[i]);
            priv->arrow_win[i] = NULL;
    	}
    }
}

static void
calendar_map_arrows  (GtkWidget *widget)
{
    KiranCalendarPrivate *priv = KIRAN_CALENDAR (widget)->priv;
    gint i;

    for (i = 0; i < 4; i++)
    {
	if (priv->arrow_win[i])
	    gdk_window_show (priv->arrow_win[i]);
    }
}

static void
kiran_calendar_map(GtkWidget *widget)
{
    KiranCalendarPrivate *priv = KIRAN_CALENDAR (widget)->priv;

    if (priv->main_win)
        gdk_window_show (priv->main_win);

    if (priv->day_win)
        gdk_window_show (priv->day_win);

    if (priv->today_win)
        gdk_window_show (priv->today_win);

    if (priv->setting_btn_win)
        gdk_window_show (priv->setting_btn_win);

    if (priv->year_text_area)
        gdk_window_show (priv->year_text_area);

    if (priv->month_text_area)
        gdk_window_show (priv->month_text_area);

    calendar_map_arrows  (widget);

    GTK_WIDGET_CLASS (kiran_calendar_parent_class)->map (widget);
}

static void
calendar_unmap_arrows  (GtkWidget *widget)
{
    KiranCalendarPrivate *priv = KIRAN_CALENDAR (widget)->priv;
    gint i;

    for (i = 0; i < 4; i++)
    {
	if (priv->arrow_win[i])
	    gdk_window_hide (priv->arrow_win[i]);
    }
}


static void
kiran_calendar_unmap(GtkWidget *widget)
{
    KiranCalendarPrivate *priv = KIRAN_CALENDAR (widget)->priv;
  
    if (priv->main_win)
        gdk_window_hide (priv->main_win);

    if (priv->day_win)
        gdk_window_hide (priv->day_win);

    if (priv->today_win)
        gdk_window_hide (priv->today_win);

    if (priv->setting_btn_win)
        gdk_window_hide (priv->setting_btn_win);

    if (priv->year_text_area)
        gdk_window_hide (priv->year_text_area);

    if (priv->month_text_area)
        gdk_window_hide (priv->month_text_area);

    calendar_unmap_arrows  (widget);

    GTK_WIDGET_CLASS (kiran_calendar_parent_class)->unmap (widget);
}

static void
calendar_day_rectangle (KiranCalendar *calendar,
                        gint           row,
                        gint           col,
                        GdkRectangle  *rect)
{
    rect->x = (DAY_RECT_WIDTH + DAY_COL_SPACE) * col + DAY_COL_LEFT_DIS; 
    rect->y = (DAY_RECT_HEIGHT + DAY_ROW_SPACE) * row + DAY_ROW_TOP_DIS; 
    rect->height = DAY_RECT_HEIGHT + DAY_ROW_SPACE;
    rect->width = DAY_RECT_WIDTH; 
}

static void
calendar_paint_day (KiranCalendar *calendar,
                    cairo_t       *cr,
                    gint          row,
                    gint          col)
{
    GtkWidget *widget = GTK_WIDGET (calendar);
    KiranCalendarPrivate *priv = calendar->priv;
    GdkRectangle day_rect;
    gint day;
    GtkStyleContext *context;
    GtkStateFlags state = 0;
    PangoLayout *layout;
    PangoRectangle logical_rect;
    gchar buffer[32];
    gchar buffer_lunar[32];
    gint x_loc, y_loc;
    gchar *markup;
    gchar *markup_lunar;
    time_t secs;
    struct tm *tm;
    gint top_dis;
    GdkRGBA color;
    gchar *tcolor;
    gchar *tfont;
    gchar *tfont_lunar;

    g_return_if_fail (row < DAY_ROW);
    g_return_if_fail (col < DAY_COL);

    secs = time (NULL);
    tm = localtime (&secs);

    context = gtk_widget_get_style_context (widget);
    gtk_style_context_save (context);

    day = priv->day[row][col];
    calendar_day_rectangle (calendar, row, col, &day_rect);

    state &= ~(GTK_STATE_FLAG_INCONSISTENT | GTK_STATE_FLAG_ACTIVE | GTK_STATE_FLAG_SELECTED | GTK_STATE_FLAG_DROP_ACTIVE);

    g_snprintf (buffer, sizeof (buffer), "%d", day);
    g_snprintf (buffer_lunar, sizeof (buffer_lunar), "%s", priv->day_lunar[row][col]);

    gtk_widget_style_get(GTK_WIDGET (calendar), 
	    	         "day-box-font", &tfont,
		         NULL);

    gtk_widget_style_get(GTK_WIDGET (calendar), 
	    	         "day-box-lunar-font", &tfont_lunar,
		         NULL);

    if (priv->day_month[row][col] == MONTH_PREV ||
        priv->day_month[row][col] == MONTH_NEXT)
    {
	gtk_widget_style_get(GTK_WIDGET (calendar), 
	         	     "day-other-month-color", &tcolor,
			     NULL);
	state |= GTK_STATE_FLAG_INCONSISTENT;
    }
    else
    {
	gtk_widget_style_get(GTK_WIDGET (calendar), 
	         	     "day-current-month-color", &tcolor,
			     NULL);

	if (priv->selected_day == day)
	{
    	    GdkRectangle rect;
	    day_rect.height = day_rect.height - DAY_ROW_SPACE;

	    rect.x = day_rect.x + 1;
            rect.y = day_rect.y + 1;
	    rect.width = day_rect.width - 2;
	    rect.height = day_rect.height - 2;
	    gtk_style_context_lookup_color(context, "day_box_select_color", &color);
	    paint_round_rectangle (cr, &rect, color.red, color.green, color.blue, color.alpha, DAY_RECT_LINE_WIDTH, 0.33, 0.54, 0.98, 1, 2, TRUE, FALSE);
	    state |= GTK_STATE_FLAG_SELECTED;
	    day_rect.height = day_rect.height + DAY_ROW_SPACE;
       
	}
        if ( priv->year == 1900 + tm->tm_year &&
             priv->month ==  tm->tm_mon &&
             day == tm->tm_mday )
        {
            day_rect.height = day_rect.height - DAY_ROW_SPACE;
	    gtk_style_context_lookup_color(context, "day_box_today_color", &color);
            paint_round_rectangle (cr, &day_rect, 0.33, 0.54, 0.98, 1.0, DAY_RECT_LINE_WIDTH, color.red, color.green, color.blue, color.alpha, 2, FALSE, TRUE);
            day_rect.height = day_rect.height + DAY_ROW_SPACE;
        }
    }

    if (priv->hover_day_row == row &&
        priv->hover_day_col == col)
    {
        day_rect.height = day_rect.height - DAY_ROW_SPACE;
	gtk_style_context_lookup_color(context, "day_box_hover_color", &color);
	paint_round_rectangle (cr, &day_rect, 0.33, 0.54, 0.98, 1.0, DAY_RECT_LINE_WIDTH, color.red, color.green, color.blue, color.alpha, 2, FALSE, TRUE);
	day_rect.height = day_rect.height + DAY_ROW_SPACE;
    }

    markup = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);
    markup_lunar = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont_lunar, "\">", buffer_lunar, "</span>", NULL);

    gtk_style_context_set_state (context, state);

    layout = gtk_widget_create_pango_layout (widget, buffer);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup (layout, markup, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    x_loc = day_rect.x + (day_rect.width - logical_rect.width) / 2;
    y_loc = day_rect.y;

    gtk_render_layout (context, cr, x_loc, y_loc, layout);
    g_free (markup);
    g_object_unref (layout);

    top_dis =  logical_rect.height;
    layout = gtk_widget_create_pango_layout (widget, buffer_lunar);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup (layout, markup_lunar, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    x_loc = day_rect.x + (day_rect.width - logical_rect.width) / 2;
    y_loc = day_rect.y + top_dis - 3;

    gtk_render_layout (context, cr, x_loc, y_loc, layout);
    g_free (markup_lunar);
    g_object_unref (layout);

    g_free (tfont);
    g_free (tcolor);
    g_free (tfont_lunar);

    gtk_style_context_restore (context);
}

static void
calendar_paint_main (KiranCalendar *calendar,
                     cairo_t       *cr)
{
    gint row, col;

    cairo_save (cr);

    for (col = 0; col < DAY_COL; col++)
	for (row = 0; row < DAY_ROW; row++)
	    calendar_paint_day (calendar, cr, row, col);

    cairo_restore (cr);
}

static void calendar_paint_background (KiranCalendar *calendar, 
			               cairo_t       *cr)
{
    GdkRGBA *bg_rgba = NULL;
    GtkStyleContext *context;
    GtkAllocation allocation;
    GdkRectangle rect;
    GdkRGBA color;

    gtk_widget_get_allocation (GTK_WIDGET (calendar), &allocation);
    context = gtk_widget_get_style_context (GTK_WIDGET (calendar));

    gtk_style_context_save (context);
    cairo_save (cr);

    rect.x = allocation.x;
    rect.y = allocation.y;
    rect.width = allocation.width;
    rect.height = allocation.height;

    gtk_style_context_lookup_color (context, "calendar_background_border_color", &color);
    gtk_style_context_get (context, gtk_style_context_get_state (context),
                             "background-color", &bg_rgba,
                              NULL);

    paint_round_rectangle (cr, &rect, color.red, color.green, color.blue, color.alpha, CALENDA_LINE_WIDTH, bg_rgba->red, bg_rgba->green, bg_rgba->blue, bg_rgba->alpha, 6, TRUE, TRUE);
    
    cairo_restore (cr);
    gtk_style_context_restore (context);
}

static void 
calendar_paint_lunar (KiranCalendar *calendar, 
		      cairo_t       *cr)
{
    GtkWidget *widget = GTK_WIDGET (calendar);
    KiranCalendarPrivate *priv = calendar->priv;
    GdkRectangle rect;
    GtkStyleContext *context;
    PangoLayout *layout;
    PangoRectangle logical_rect;
    gint x_loc, y_loc;
    gchar *markup;
    gint i;
    gint x;
    gchar *tcolor;
    gchar *tfont;

    cairo_save (cr);

    context = gtk_widget_get_style_context (widget);
    gtk_style_context_save (context);

    gtk_widget_style_get(GTK_WIDGET (calendar), 
	    	         "today-font", &tfont,
		         NULL);

    x = 0;
    for (i = 0; i < 3; i++)
    {
        gchar buffer[32];

	g_snprintf (buffer, sizeof (buffer), "%s", priv->lunar[i]);

	if (i == 0)
        {

	    if (priv->today_state == KIRAN_HOVER)
	        gtk_widget_style_get(GTK_WIDGET (calendar), 
	   	         	     "today-calendar-button-hover-color", &tcolor,
				     NULL);
	    else if (priv->today_state == KIRAN_PRESS)
	        gtk_widget_style_get(GTK_WIDGET (calendar), 
	   	         	     "today-calendar-button-press-color", &tcolor,
				     NULL);
	    else
	        gtk_widget_style_get(GTK_WIDGET (calendar), 
	   	         	     "today-calendar-button-normal-color", &tcolor,
				     NULL);

        }
	else
	{

	    gtk_widget_style_get(GTK_WIDGET (calendar), 
	   		    	 "today-calendar-color", &tcolor,
				 NULL);

	}

	markup = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);
	g_free (tcolor);


	layout = gtk_widget_create_pango_layout (widget, buffer);
        pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
        pango_layout_set_markup (layout, markup, -1);
        pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

        rect.width = logical_rect.width;
        rect.height = logical_rect.height;
	if ( i == 0)
            rect.x = LUNAR_LEFT_DIS;
	else
	    rect.x = LUNAR_SPACE + x;

	rect.y = LUNAR_TOP_DIS;

        x_loc = rect.x;
        y_loc = rect.y;

        gtk_render_layout (context, cr, x_loc, y_loc, layout);
        x = x_loc + rect.width;

        g_free (markup);
        g_object_unref (layout);
    }
    g_free (tfont);

    gtk_style_context_restore (context);
    cairo_restore (cr);
}

static void 
calendar_paint_setting_btn (KiranCalendar *calendar, 
		      cairo_t       *cr)
{
    KiranCalendarPrivate *priv = calendar->priv;
    cairo_pattern_t *pattern;
    gdouble pixbuf_x, pixbuf_y;
    GdkRGBA color;
    GtkStyleContext *context;


    if (!priv->setting_btn_svg)
	return;

    context = gtk_widget_get_style_context (GTK_WIDGET (calendar));
    cairo_save (cr);
    gtk_style_context_save (context);

    pixbuf_x = CALENDA_WIDTH - SETTING_BTN_RIGHT_DIS - SETTING_BTN_SVG_W;
    pixbuf_y = SETTING_BTN_TOP_DIS;

    if (priv->setting_btn_state == KIRAN_NORMAL)
    {
        gdk_cairo_set_source_pixbuf (cr, priv->setting_btn_svg, pixbuf_x, pixbuf_y);
        cairo_paint (cr);
    }
    else
    {
	if (priv->setting_btn_state == KIRAN_PRESS)
	    gtk_style_context_lookup_color(context, "calendar_setting_btn_press_color", &color);
	else
	    gtk_style_context_lookup_color(context, "calendar_setting_btn_hover_color", &color);

        cairo_push_group (cr);
        gdk_cairo_set_source_pixbuf (cr, priv->setting_btn_svg, pixbuf_x, pixbuf_y);
        cairo_paint (cr);
        pattern = cairo_pop_group (cr);
        cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
        cairo_mask (cr, pattern);
    }

    gtk_style_context_restore (context);
    cairo_restore (cr);
}

static void
paint_one_arrow_with_color (cairo_t *cr,
		            gint x,
			    gint y,
			    gint w,
			    gint h,
			    double red,
			    double green,
			    double blue,
			    double alpha,
			    gboolean is_prev)
{
    cairo_save (cr);
    cairo_set_source_rgba(cr, red, green, blue, alpha);
    cairo_set_line_width (cr, 1);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);

    if (is_prev)
    {
        cairo_move_to (cr, x + w, y);
        cairo_line_to (cr, x, y + h/2);

        cairo_move_to (cr, x, y + h/2);
        cairo_line_to (cr, x + w, y + h);
    }
    else
    {
        cairo_move_to (cr, x, y);
        cairo_line_to (cr, x + w, y + h/2);

        cairo_move_to (cr, x + w, y + h/2);
        cairo_line_to (cr, x, y + h);
    }

    cairo_stroke(cr);
    cairo_restore (cr);
}


static void
calendar_paint_arrows (KiranCalendar *calendar,
	               cairo_t       *cr)
{
    KiranCalendarPrivate *priv = calendar->priv;
    GtkStyleContext *context;
    GdkRGBA pycolor;
    GdkRGBA pmcolor;
    GdkRGBA nycolor;
    GdkRGBA nmcolor;
    gint x;

    context = gtk_widget_get_style_context (GTK_WIDGET (calendar));
    cairo_save (cr);
    gtk_style_context_save (context);


    if (priv->prev_year_state == KIRAN_PRESS)
        gtk_style_context_lookup_color(context, "calendar_arrow_press_color", &pycolor);
    else if (priv->prev_year_state == KIRAN_HOVER)
        gtk_style_context_lookup_color(context, "calendar_arrow_hover_color", &pycolor);
    else
        gtk_style_context_lookup_color(context, "calendar_arrow_normal_color", &pycolor);

    if (priv->next_year_state == KIRAN_PRESS)
        gtk_style_context_lookup_color(context, "calendar_arrow_press_color", &nycolor);
    else if (priv->next_year_state == KIRAN_HOVER)
        gtk_style_context_lookup_color(context, "calendar_arrow_hover_color", &nycolor);
    else
        gtk_style_context_lookup_color(context, "calendar_arrow_normal_color", &nycolor);

    if (priv->prev_month_state == KIRAN_PRESS)
        gtk_style_context_lookup_color(context, "calendar_arrow_press_color", &pmcolor);
    else if (priv->prev_month_state == KIRAN_HOVER)
        gtk_style_context_lookup_color(context, "calendar_arrow_hover_color", &pmcolor);
    else
        gtk_style_context_lookup_color(context, "calendar_arrow_normal_color", &pmcolor);

    if (priv->next_month_state == KIRAN_PRESS)
        gtk_style_context_lookup_color(context, "calendar_arrow_press_color", &nmcolor);
    else if (priv->next_month_state == KIRAN_HOVER)
        gtk_style_context_lookup_color(context, "calendar_arrow_hover_color", &nmcolor);
    else
        gtk_style_context_lookup_color(context, "calendar_arrow_normal_color", &nmcolor);

    x = HEADER_LEFT_DIS;
    paint_one_arrow_with_color (cr, x, HEADER_TOP_DIS, ARROW_WIDTH, ARROW_HEIGHT, pycolor.red, pycolor.green, pycolor.blue, pycolor.alpha, TRUE); 
    paint_one_arrow_with_color (cr, x + ARROW_SPACE, HEADER_TOP_DIS, ARROW_WIDTH, ARROW_HEIGHT, pycolor.red, pycolor.green, pycolor.blue, pycolor.alpha, TRUE); 

    x = x + ARROW_SPACE + ARROW_WIDTH + HEADER_ARROW_SPACE;
    paint_one_arrow_with_color (cr, x, HEADER_TOP_DIS, ARROW_WIDTH, ARROW_HEIGHT, pmcolor.red, pmcolor.green, pmcolor.blue, pmcolor.alpha, TRUE); 

    x = CALENDA_WIDTH - HEADER_RIGHT_DIS - ARROW_WIDTH;
    paint_one_arrow_with_color (cr, x, HEADER_TOP_DIS, ARROW_WIDTH, ARROW_HEIGHT, nycolor.red, nycolor.green, nycolor.blue, nycolor.alpha, FALSE); 
    paint_one_arrow_with_color (cr, x - ARROW_SPACE, HEADER_TOP_DIS, ARROW_WIDTH, ARROW_HEIGHT, nycolor.red, nycolor.green, nycolor.blue, nycolor.alpha, FALSE); 

    x = x - HEADER_ARROW_SPACE - ARROW_WIDTH - ARROW_SPACE;
    paint_one_arrow_with_color (cr, x, HEADER_TOP_DIS, ARROW_WIDTH, ARROW_HEIGHT, nmcolor.red, nmcolor.green, nmcolor.blue, nmcolor.alpha, FALSE); 

    gtk_style_context_restore (context);
    cairo_restore (cr);

}

static void
calendar_paint_header_text (KiranCalendar *calendar,
			    cairo_t       *cr)
{
    GtkWidget *widget = GTK_WIDGET (calendar);
    KiranCalendarPrivate *priv = calendar->priv;
    GtkStyleContext *context;
    PangoLayout *layout;
    PangoRectangle logical_rect;
    gint x_loc, y_loc;
    gchar *markup;
    gchar buffer[32];
    GdkRectangle  rect;
    gint year_text_width;
    gint month_text_width;
    gchar *tcolor;
    gchar *tfont;
    GdkRGBA color;
    GdkRGBA ncolor;

    gtk_widget_style_get(GTK_WIDGET (calendar), 
	         	 "day-head-color", &tcolor,
			 NULL);

    gtk_widget_style_get(GTK_WIDGET (calendar), 
	    	         "day-head-font", &tfont,
		         NULL);

    cairo_save (cr);

    context = gtk_widget_get_style_context (widget);
    gtk_style_context_save (context);

    gtk_style_context_lookup_color(context, "day_input_entry_select_color", &color);
    gtk_style_context_lookup_color(context, "day_input_entry_normal_color", &ncolor);
  
    if (priv->year_input)
    {
        g_snprintf (buffer, sizeof (buffer),  "%s", priv->year_text);
    }
    else
        g_snprintf (buffer, sizeof (buffer),  "%.4d", priv->year);

    markup = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);
    
    layout = gtk_widget_create_pango_layout (widget, buffer);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup (layout, markup, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    if (year_text_length == 0)
	year_text_length = logical_rect.width;

    if (year_text_height == 0)
	year_text_height = logical_rect.height;

    year_text_width = logical_rect.width;
    logical_rect.width = year_text_length;
    logical_rect.height = year_text_height;

    x_loc = HEADER_TEXT_LEFT_DIS;
    y_loc = HEADER_TEXT_TOP_DIS;

    if (!(g_getenv ("LANG") && g_strrstr (g_getenv ("LANG"), "zh_CN")))
	x_loc = x_loc + 16;
	
    if (priv->year_text_select)
    {
	rect.x = x_loc;
	rect.y = y_loc + SELECT_BOX_COLOR_Y_DIS;
	rect.width = logical_rect.width;
	rect.height = logical_rect.height - 2 * SELECT_BOX_COLOR_Y_DIS;
        paint_round_rectangle (cr, &rect, 0.33, 0.54, 0.98, 0.5, 1.0, color.red, color.green, color.blue, color.alpha, 2, FALSE, TRUE);
    }


    rect.x = x_loc - HEADER_TEXT_SPACE / 2;
    rect.y = y_loc - 2;
    rect.width = logical_rect.width + HEADER_TEXT_SPACE;
    rect.height = logical_rect.height + 4;

    if (priv->year_input)
        paint_round_rectangle (cr, &rect, color.red, color.green, color.blue, color.alpha, 0.5, ncolor.red, ncolor.green, ncolor.blue, ncolor.alpha, 2, TRUE, TRUE);
    else	
        paint_round_rectangle (cr, &rect, 1.0, 1.0, 1.0, 1.0, 1, ncolor.red, ncolor.green, ncolor.blue, ncolor.alpha, 2, FALSE, TRUE);

    if (priv->year_text_input && priv->cursor_visible && !priv->year_text_select)
    {
	cairo_save (cr);

	cairo_set_source_rgb (cr, ncolor.red, ncolor.green, ncolor.blue);
	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, x_loc + year_text_width + 1, y_loc);
	cairo_line_to (cr, x_loc + year_text_width + 1, y_loc + year_text_height);
	cairo_stroke (cr);

	cairo_restore (cr);
    }

    gtk_render_layout (context, cr, x_loc, y_loc, layout);
    g_free (markup);
    g_object_unref (layout);

    x_loc = x_loc + logical_rect.width;

    if (g_getenv ("LANG") && g_strrstr (g_getenv ("LANG"), "zh_CN"))
        g_snprintf (buffer, sizeof (buffer),  "%s", "年");
    else
        g_snprintf (buffer, sizeof (buffer),  "%s", "-");
	
    markup = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);
    
    layout = gtk_widget_create_pango_layout (widget, buffer);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup (layout, markup, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    x_loc = x_loc + HEADER_TEXT_SPACE;
    gtk_render_layout (context, cr, x_loc, y_loc, layout);
    g_free (markup);
    g_object_unref (layout);
    x_loc = x_loc + logical_rect.width;

    if (priv->month_input)
    {
        g_snprintf (buffer, sizeof (buffer),  "%s", priv->month_text);
    }
    else
        g_snprintf (buffer, sizeof (buffer),  "%.2d", priv->month + 1);

    markup = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);
    
    layout = gtk_widget_create_pango_layout (widget, buffer);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup (layout, markup, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    if (month_text_length == 0)
	month_text_length = logical_rect.width;

    month_text_width = logical_rect.width;
    logical_rect.width = month_text_length;
    logical_rect.height = year_text_height;

    x_loc = x_loc + HEADER_TEXT_SPACE;

    if (priv->month_text_select)
    {
	rect.x = x_loc;
	rect.y = y_loc + SELECT_BOX_COLOR_Y_DIS;
	rect.width = logical_rect.width;
	rect.height = logical_rect.height - 2 * SELECT_BOX_COLOR_Y_DIS;
        paint_round_rectangle (cr, &rect, 0.33, 0.54, 0.98, 0.5, 1.0, color.red, color.green, color.blue, color.alpha, 2, FALSE, TRUE);
    }

    rect.x = x_loc - HEADER_TEXT_SPACE/2;
    rect.y = y_loc - 2;
    rect.width = logical_rect.width + HEADER_TEXT_SPACE;
    rect.height = logical_rect.height + 4;

    if (priv->month_input)
        paint_round_rectangle (cr, &rect, color.red, color.green, color.blue, color.alpha, 0.5, ncolor.red, ncolor.green, ncolor.blue, ncolor.alpha, 2, TRUE, TRUE);
    else	
        paint_round_rectangle (cr, &rect, 1.0, 1.0, 1.0, 1.0, 1,ncolor.red, ncolor.green, ncolor.blue, ncolor.alpha, 2, FALSE, TRUE);

    if (priv->month_text_input && priv->cursor_visible && !priv->month_text_select)
    {
	cairo_save (cr);

	cairo_set_source_rgb (cr, ncolor.red, ncolor.green, ncolor.blue);
	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, x_loc + month_text_width + 1, y_loc);
	cairo_line_to (cr, x_loc + month_text_width + 1, y_loc + year_text_height);
	cairo_stroke (cr);

	cairo_restore (cr);
    }

    gtk_render_layout (context, cr, x_loc, y_loc, layout);
    g_free (markup);
    g_object_unref (layout);

    x_loc = x_loc + logical_rect.width;

    if (g_getenv ("LANG") && g_strrstr (g_getenv ("LANG"), "zh_CN"))
        g_snprintf (buffer, sizeof (buffer),  "%s", "月");
    else
        g_snprintf (buffer, sizeof (buffer),  "%s", " ");
	
    markup = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);
    
    layout = gtk_widget_create_pango_layout (widget, buffer);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    pango_layout_set_markup (layout, markup, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

    x_loc = x_loc + HEADER_TEXT_SPACE;
    gtk_render_layout (context, cr, x_loc, y_loc, layout);
    g_free (markup);
    g_object_unref (layout);

    gtk_style_context_restore (context);
    cairo_restore (cr);

    g_free (tcolor);
    g_free (tfont);
}

static void 
calendar_paint_header (KiranCalendar *calendar, 
		       cairo_t       *cr)
{
    calendar_paint_arrows (calendar, cr);
    calendar_paint_header_text (calendar, cr);
}

static void 
calendar_paint_daynames (KiranCalendar *calendar, 
			 cairo_t       *cr)
{
    GtkWidget *widget = GTK_WIDGET (calendar);
    GtkStyleContext *context;
    PangoLayout *layout;
    PangoRectangle logical_rect;
    gchar *markup;
    gint i;
    gint x;
    gint space;
    gchar *tcolor;
    gchar *tfont;

    gtk_widget_style_get(GTK_WIDGET (calendar), 
	         	 "day-names-color", &tcolor,
			 NULL);

    gtk_widget_style_get(GTK_WIDGET (calendar), 
	    	         "day-names-font", &tfont,
		         NULL);

    cairo_save (cr);

    context = gtk_widget_get_style_context (widget);
    gtk_style_context_save (context);
    
    space = 0;
    x = 0;
    for (i = 0; i < 7; i++)
    {
        gchar buffer[32];

	g_snprintf (buffer, sizeof (buffer), "%s", default_abbreviated_dayname[i]);
        markup = g_strconcat ("<span foreground=\"", tcolor, "\"", "font_desc=\"", tfont, "\">", buffer, "</span>", NULL);

	layout = gtk_widget_create_pango_layout (widget, buffer);
        pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
        pango_layout_set_markup (layout, markup, -1);
        pango_layout_get_pixel_extents (layout, NULL, &logical_rect);

        if (i == 0)
        {
            x = DAY_NAME_LEFT_DIS;
	    space = (CALENDA_WIDTH - 2 * DAY_NAME_LEFT_DIS - 7 * logical_rect.width) / 6;
        }
        else
	    x = x + space;

        gtk_render_layout (context, cr, x, DAY_NAME_TOP_DIS, layout);
	x = x + logical_rect.width;

        g_free (markup);
        g_object_unref (layout);

    }

    gtk_style_context_restore (context);
    cairo_restore (cr);
    g_free (tcolor);
    g_free (tfont);
}

static void
calendar_paint_separate_line (KiranCalendar *calendar,
		              cairo_t       *cr,
		              gint           top_dis)
{
    int width;

    cairo_save (cr);

    cairo_set_source_rgba (cr, 0.92, 0.92, 0.94, 0.1);
    width = CALENDA_WIDTH - 2*SEPARATE_LINE_LR_DIS;

    cairo_rectangle (cr, SEPARATE_LINE_LR_DIS, top_dis, width, 1);
    cairo_fill (cr);

    cairo_restore (cr);
}

static gboolean
kiran_calendar_draw (GtkWidget *widget,
	             cairo_t   *cr)
{
    KiranCalendar *calendar = KIRAN_CALENDAR (widget);

    calendar_paint_background (calendar, cr);
    calendar_paint_lunar (calendar, cr);
    calendar_paint_setting_btn (calendar, cr);
    calendar_paint_separate_line (calendar, cr, SEPARATE_LINE_TOP_DIS);
    calendar_paint_header (calendar, cr);
    calendar_paint_daynames (calendar, cr);
    calendar_paint_separate_line (calendar, cr, SEPARATE_LINE_TOP_DIS_1);
    calendar_paint_main (calendar, cr);

    return FALSE;
}

static void
calendar_compute_event_day (gint win_x,
		            gint win_y,
			    gint x,
                            gint y,
                            gint *row,
                            gint *col)
{
    GdkRectangle day_rect;
    gint coll, roww;

    *col = -1;
    *row = -1;
    
    x = x + win_x;
    y = y + win_y;

    for (coll = 0; coll < DAY_COL; coll++)
        for (roww = 0; roww < DAY_ROW; roww++)
    {
    	calendar_day_rectangle (NULL, roww, coll, &day_rect);
	if ( (x >= day_rect.x) &&
	     (y >= day_rect.y) &&
	     (x <= day_rect.x + day_rect.width) &&
	     (y <= day_rect.y + day_rect.height))
	{
	    *row = roww;
	    *col = coll;
	    break;
	}
    }
}

static void
calendar_set_month_prev (KiranCalendar *calendar)
{
    KiranCalendarPrivate *priv = calendar->priv;
    gint month_len;

    if (priv->month == 0)
    {
	priv->month = 11;
        priv->year--;
    }
    else
	priv->month--;

    month_len = month_length[leap (priv->year)][priv->month + 1];
    
    calendar_compute_days (calendar);

    if (month_len < priv->selected_day)
    {
	priv->selected_day = 0;
        kiran_calendar_select_day (calendar, month_len);
    }
    else
    {
	if (priv->selected_day < 0)
	    priv->selected_day = priv->selected_day + 1 + month_length[leap (priv->year)][priv->month + 1];
	kiran_calendar_select_day (calendar, priv->selected_day);
    } 

    gtk_widget_queue_draw (GTK_WIDGET (calendar));
}

static void
calendar_set_month_next (KiranCalendar *calendar)
{
    KiranCalendarPrivate *priv = calendar->priv;
    gint month_len;

    if (priv->month == 11)
    {
	priv->month = 0;
	priv->year++;
    }
    else
	priv->month++;

    calendar_compute_days (calendar);

    month_len = month_length[leap (priv->year)][priv->month + 1];

    if (month_len < priv->selected_day)
    {
	priv->selected_day = 0;
        kiran_calendar_select_day (calendar, priv->selected_day);
    }
    else
	kiran_calendar_select_day (calendar, priv->selected_day);

    gtk_widget_queue_draw (GTK_WIDGET (calendar));
}

static void
calendar_set_year_prev (KiranCalendar *calendar)
{
    KiranCalendarPrivate *priv = calendar->priv;
    gint month_len;

    priv->year--;

    calendar_compute_days (calendar);

    month_len = month_length[leap (priv->year)][priv->month + 1];

    if (month_len < priv->selected_day)
    {
	priv->selected_day = 0;
        kiran_calendar_select_day (calendar, priv->selected_day);
    }
    else
	kiran_calendar_select_day (calendar, priv->selected_day);

    gtk_widget_queue_draw (GTK_WIDGET (calendar));
}

static void
calendar_set_year_next (KiranCalendar *calendar)
{
    KiranCalendarPrivate *priv = calendar->priv;
    gint month_len;

    priv->year++;

    calendar_compute_days (calendar);

    month_len = month_length[leap (priv->year)][priv->month + 1];

    if (month_len < priv->selected_day)
    {
	priv->selected_day = 0;
        kiran_calendar_select_day (calendar, priv->selected_day);
    }
    else
	kiran_calendar_select_day (calendar, priv->selected_day);

    gtk_widget_queue_draw (GTK_WIDGET (calendar));
}

static void 
calendar_main_button_press (KiranCalendar  *calendar,
                            GdkEventButton *event)
{
    GtkWidget *widget = GTK_WIDGET (calendar);
    KiranCalendarPrivate *priv = calendar->priv;
    gint x, y;
    gint win_x, win_y;
    gint row, col;
    gint day_month;
    gint day;
    GtkAllocation allocation;
  
    x = (gint) (event->x);
    y = (gint) (event->y);
  
    gdk_window_get_position (priv->day_win, &win_x, &win_y);
    gtk_widget_get_allocation (widget, &allocation);
  
    calendar_compute_event_day (win_x, win_y, x, y, &row, &col);
  
    /* If row or column isn't found, just return. */
    if (row == -1 || col == -1 || row >= DAY_ROW || col >= DAY_COL )
        return;
  
    day_month = priv->day_month[row][col];
  
    if (event->type == GDK_BUTTON_PRESS)
    {
        day = priv->day[row][col];
        if (day_month == MONTH_PREV)
            calendar_set_month_prev (calendar);

        if (day_month == MONTH_NEXT)
            calendar_set_month_next (calendar);

        if (!gtk_widget_has_focus (widget))
            gtk_widget_grab_focus (widget);

        kiran_calendar_select_day (calendar, day);
    }
}

static void 
calendar_today_button_press (KiranCalendar  *calendar,
                            GdkEventButton  *event)
{
    KiranCalendarPrivate *priv = calendar->priv;
    time_t secs;
    struct tm *tm;

    secs = time (NULL);
    tm = localtime (&secs);
    priv->month = tm->tm_mon;
    priv->year  = 1900 + tm->tm_year;
    priv->selected_day = tm->tm_mday;

    calendar_compute_days (calendar);    
    gtk_widget_queue_draw (GTK_WIDGET (calendar));
}

static void 
calendar_setting_button_press (KiranCalendar  *calendar,
                               GdkEventButton  *event)
{
    GError *error = NULL;

    if (!g_spawn_command_line_async ("kiran-timedate-manager", &error))
    {
	g_warning ("kiran calendar: %s\n", error->message);	
    }
}

static void
show_cursor (KiranCalendar *calendar)
{
    KiranCalendarPrivate *priv = calendar->priv;
    GtkWidget *widget;

    if (!priv->cursor_visible)
    {
	priv->cursor_visible = TRUE;

	widget = GTK_WIDGET (calendar);
	if (gtk_widget_has_focus (widget))
	{
	    if (priv->year_text_input)
		calendar_redraw_component (calendar, priv->year_text_area);
	    if (priv->month_text_input)
		calendar_redraw_component (calendar, priv->month_text_area);
	}
    }
}

static void
hide_cursor (KiranCalendar *calendar)
{
    KiranCalendarPrivate *priv = calendar->priv;
    GtkWidget *widget;

    if (priv->cursor_visible)
    {
	priv->cursor_visible = FALSE;

	widget = GTK_WIDGET (calendar);
	if (gtk_widget_has_focus (widget))
	{
	    if (priv->year_text_input)
		calendar_redraw_component (calendar, priv->year_text_area);
	    if (priv->month_text_input)
		calendar_redraw_component (calendar, priv->month_text_area);
	}
    }
}

static gint
blink_cb (gpointer data)
{
    KiranCalendar *calendar = KIRAN_CALENDAR (data);
    KiranCalendarPrivate *priv = calendar->priv;

    if (!gtk_widget_has_focus (GTK_WIDGET (calendar)))
    {
	return G_SOURCE_REMOVE;
    }

    if (priv->cursor_visible)
    {
	hide_cursor (calendar);
	priv->blink_timeout = gdk_threads_add_timeout (BLINK_TIMEOUT,
	    				           blink_cb,
	    				           calendar);	
	g_source_set_name_by_id (priv->blink_timeout, "[kiran]blink_cb");
    }
    else
    {
	show_cursor (calendar);
	priv->blink_timeout = gdk_threads_add_timeout (BLINK_TIMEOUT,
	    				           blink_cb,
	    				           calendar);	
	g_source_set_name_by_id (priv->blink_timeout, "[kiran]blink_cb");
    }
  
    return G_SOURCE_REMOVE; 
}

static void
kiran_calendar_check_cursor_blink (KiranCalendar *calendar)
{
    KiranCalendarPrivate *priv = calendar->priv;
    
    if (priv->year_text_input || priv->month_text_input)
    {
	if (!priv->blink_timeout)
	{
	    show_cursor (calendar);
	    priv->blink_timeout = gdk_threads_add_timeout (BLINK_TIMEOUT,
						           blink_cb,
						           calendar);	
	    g_source_set_name_by_id (priv->blink_timeout, "[kiran]blink_cb");
	}
    }
    else
    {
	if (priv->blink_timeout)
        {
	   g_source_remove (priv->blink_timeout);
           priv->blink_timeout = 0;
        }
	hide_cursor (calendar);
    }
}

static gboolean 
kiran_calendar_button_press (GtkWidget      *widget,
                             GdkEventButton *event)
{
    KiranCalendar *calendar = KIRAN_CALENDAR (widget);
    KiranCalendarPrivate *priv = calendar->priv;

    if (!gtk_widget_has_focus (widget))
	gtk_widget_grab_focus (widget);

    calendar_arrows_change_state (calendar, event->window, KIRAN_PRESS);

    if (event->window ==  priv->arrow_win[ARROW_YEAR_PREV])
    {
        calendar_set_year_prev (calendar);
    }

    if (event->window ==  priv->arrow_win[ARROW_MONTH_PREV])
    {
        calendar_set_month_prev (calendar);
    }

    if (event->window ==  priv->arrow_win[ARROW_YEAR_NEXT])
    {
        calendar_set_year_next (calendar);
    }

    if (event->window ==  priv->arrow_win[ARROW_MONTH_NEXT])
    {
        calendar_set_month_next (calendar);
    }

    if (event->window == priv->day_win)
    {
	calendar_main_button_press (calendar, event);
    }

    if (event->window == priv->today_win)
    {
        priv->today_state = KIRAN_PRESS;
	calendar_redraw_component (calendar, priv->today_win);
        calendar_today_button_press (calendar, event);
    }

    if (event->window == priv->setting_btn_win)
    {
        priv->setting_btn_state = KIRAN_PRESS;
	calendar_setting_button_press (calendar, event);
	calendar_redraw_component (calendar, priv->setting_btn_win);
    }

    if (event->window == priv->year_text_area)
    {
	if (!priv->year_input)
	    priv->year_text_select = TRUE;
	else
	    priv->year_text_select = FALSE;

        priv->year_input = TRUE;
        priv->year_text_input = TRUE;
        priv->month_text_input = FALSE;
	g_snprintf(priv->year_text, YEAR_INPUT_MAX, "%d", priv->year);
    }

    if (event->window == priv->month_text_area)
    {
	if (!priv->month_input)
	    priv->month_text_select = TRUE;
	else
	    priv->month_text_select = FALSE;

        priv->month_input = TRUE;
        priv->year_text_input = FALSE;
        priv->month_text_input = TRUE;
	g_snprintf(priv->month_text, MONTH_INPUT_MAX, "%.2d", priv->month + 1);
    }

    if (event->window != priv->year_text_area &&
       event->window != priv->month_text_area)
    {
	priv->year_input = FALSE;
	priv->month_input = FALSE;
        priv->year_text_input = FALSE;
        priv->month_text_input = FALSE;
	priv->year_text_select = FALSE;
	priv->month_text_select = FALSE;
    }
    
    calendar_redraw_component (calendar, priv->year_text_area);
    calendar_redraw_component (calendar, priv->month_text_area);
    kiran_calendar_check_cursor_blink (calendar);

    return TRUE;
}

static gboolean 
kiran_calendar_button_release (GtkWidget      *widget,
                             GdkEventButton *event)
{
    KiranCalendar *calendar = KIRAN_CALENDAR (widget);
    KiranCalendarPrivate *priv = calendar->priv;

    calendar_arrows_change_state (calendar, event->window, KIRAN_HOVER);

    if (event->window == priv->today_win)
    {
        priv->today_state = KIRAN_HOVER;
	calendar_redraw_component (calendar, priv->today_win);
    }

    if (event->window == priv->setting_btn_win)
    {
        priv->setting_btn_state = KIRAN_HOVER;
	calendar_redraw_component (calendar, priv->setting_btn_win);
    }

    return TRUE;
}

static void
calendar_main_enter_notify (KiranCalendar *calendar,
                            GdkEventCrossing *event)
{
    GtkWidget *widget = GTK_WIDGET (calendar);
    KiranCalendarPrivate *priv = calendar->priv;
    gint x, y;
    gint win_x, win_y;
    gint row, col;
    GtkAllocation allocation;

    x = (gint) (event->x);
    y = (gint) (event->y);

    gdk_window_get_position (priv->day_win, &win_x, &win_y);
    gtk_widget_get_allocation (widget, &allocation);

    calendar_compute_event_day (win_x, win_y, x, y, &row, &col);


    /* If row or column isn't found, just return. */
    if (row == -1 || col == -1 || row >= DAY_ROW || col >= DAY_COL )
        return;
  
    if (priv->hover_day_row != -1 &&
	priv->hover_day_col != -1)
    {
	if (gtk_widget_is_drawable (GTK_WIDGET (calendar)))
	    calendar_invalidate_day (calendar, priv->hover_day_row , priv->hover_day_col);
	priv->hover_day_row = -1;
	priv->hover_day_col = -1;
    }
	
    priv->hover_day_row = row;
    priv->hover_day_col = col;
    if (gtk_widget_is_drawable (GTK_WIDGET (calendar)))
        calendar_invalidate_day (calendar, priv->hover_day_row , priv->hover_day_col);

}

static gboolean 
kiran_calendar_enter_notify (GtkWidget        *widget,
                             GdkEventCrossing *event)
{
    KiranCalendar *calendar = KIRAN_CALENDAR (widget);
    KiranCalendarPrivate *priv = calendar->priv;

    calendar_arrows_change_state (calendar, event->window, KIRAN_HOVER);

    if (event->window == priv->day_win)
    {
	calendar_main_enter_notify (calendar, event);
    }

    if (event->window == priv->today_win)
    {
        priv->today_state = KIRAN_HOVER;
	calendar_redraw_component (calendar, priv->today_win);
    }

    if (event->window == priv->setting_btn_win)
    {
        priv->setting_btn_state = KIRAN_HOVER;
	calendar_redraw_component (calendar, priv->setting_btn_win);
    }

    return TRUE;
}

static gboolean 
kiran_calendar_leave_notify (GtkWidget        *widget,
                             GdkEventCrossing *event)
{
    KiranCalendar *calendar = KIRAN_CALENDAR (widget);
    KiranCalendarPrivate *priv = calendar->priv;

    calendar_arrows_change_state (calendar, event->window, KIRAN_NORMAL);

    if (event->window == priv->day_win)
    {
        if (priv->hover_day_row != -1 &&
            priv->hover_day_col != -1)
        {
            if (gtk_widget_is_drawable (GTK_WIDGET (calendar)))
                calendar_invalidate_day (calendar, priv->hover_day_row , priv->hover_day_col);
            priv->hover_day_row = -1;
            priv->hover_day_col = -1;
        }
    }

    if (event->window == priv->today_win)
    {
        priv->today_state = KIRAN_NORMAL;
	calendar_redraw_component (calendar, priv->today_win);
    }

    if (event->window == priv->setting_btn_win)
    {
        priv->setting_btn_state = KIRAN_NORMAL;
	calendar_redraw_component (calendar, priv->setting_btn_win);
    }

    return TRUE;
}

static void
calendar_main_motion_notify (KiranCalendar *calendar,
                            GdkEventMotion *event)
{
    GtkWidget *widget = GTK_WIDGET (calendar);
    KiranCalendarPrivate *priv = calendar->priv;
    gint x, y;
    gint win_x, win_y;
    gint row, col;
    GtkAllocation allocation;

    x = (gint) (event->x);
    y = (gint) (event->y);

    gdk_window_get_position (priv->day_win, &win_x, &win_y);
    gtk_widget_get_allocation (widget, &allocation);

    calendar_compute_event_day (win_x, win_y, x, y, &row, &col);


    /* If row or column isn't found, just return. */
    if (row == -1 || col == -1 || row >= DAY_ROW || col >= DAY_COL )
        return;

    if (priv->hover_day_row != -1 &&
        priv->hover_day_col != -1)
    {
        if (gtk_widget_is_drawable (GTK_WIDGET (calendar)))
            calendar_invalidate_day (calendar, priv->hover_day_row , priv->hover_day_col);
        priv->hover_day_row = -1;
        priv->hover_day_col = -1;
    }

    priv->hover_day_row = row;
    priv->hover_day_col = col;
    if (gtk_widget_is_drawable (GTK_WIDGET (calendar)))
        calendar_invalidate_day (calendar, priv->hover_day_row , priv->hover_day_col);
}

static gboolean 
kiran_calendar_motion_notify (GtkWidget        *widget,
                             GdkEventMotion    *event)
{
    KiranCalendar *calendar = KIRAN_CALENDAR (widget);
    KiranCalendarPrivate *priv = calendar->priv;

    if (event->window == priv->day_win)
    {
	calendar_main_motion_notify (calendar, event);
    }

    return TRUE;
}

static gboolean 
kiran_calendar_scroll (GtkWidget      *widget,
		       GdkEventScroll *event)
{
    KiranCalendar *calendar = KIRAN_CALENDAR (widget);

    if (event->direction == GDK_SCROLL_UP)
    {
	if (!gtk_widget_has_focus (widget))
	    gtk_widget_grab_focus (widget);

	calendar_set_month_prev (calendar);
    }
    else if (event->direction == GDK_SCROLL_DOWN)
    {
	if (!gtk_widget_has_focus (widget))
	    gtk_widget_grab_focus (widget);

	calendar_set_month_next (calendar);
    }
    else
	return FALSE;

    return TRUE;  
}

static gint
get_key_event_num (GdkEventKey    *event)
{
    int ret;

    switch (event->keyval)
    {
	case GDK_KEY_0 :	
	case GDK_KEY_KP_0 :
	    ret = 0;	
	    break;

	case GDK_KEY_1 :	
	case GDK_KEY_KP_1 :
	    ret = 1;	
	    break;

	case GDK_KEY_2 :	
	case GDK_KEY_KP_2 :
	    ret = 2;	
	    break;

	case GDK_KEY_3 :	
	case GDK_KEY_KP_3 :
	    ret = 3;	
	    break;

	case GDK_KEY_4 :	
	case GDK_KEY_KP_4 :
	    ret = 4;	
	    break;

	case GDK_KEY_5 :	
	case GDK_KEY_KP_5 :
	    ret = 5;	
	    break;

	case GDK_KEY_6 :	
	case GDK_KEY_KP_6 :
	    ret = 6;	
	    break;

	case GDK_KEY_7 :	
	case GDK_KEY_KP_7 :
	    ret = 7;	
	    break;

	case GDK_KEY_8 :	
	case GDK_KEY_KP_8 :
	    ret = 8;	
	    break;

	case GDK_KEY_9 :	
	case GDK_KEY_KP_9 :
	    ret = 9;	
	    break;

	default :
	    ret = INVALIDE_NUM;
	    break;
    }

    return ret;
}

static gboolean 
kiran_calendar_key_press (GtkWidget      *widget,
	                  GdkEventKey    *event)
{
    KiranCalendar *calendar = KIRAN_CALENDAR (widget);
    KiranCalendarPrivate *priv = calendar->priv;
    gint len;
    gint num;

    if (event->keyval == GDK_KEY_KP_Enter ||
	event->keyval == GDK_KEY_Return ||
	event->keyval == GDK_KEY_ISO_Enter)
    {
	if (priv->year_text_input || priv->month_text_input)
	{
	    gint year =  atoi (priv->year_text);
            gint month = atoi (priv->month_text);

	    priv->year_input = FALSE;
 	    priv->month_input = FALSE;

	    priv->year_text_input = FALSE;
 	    priv->month_text_input = FALSE;
	    priv->year_text_select = FALSE;
	    priv->month_text_select = FALSE;

	    if (year > 0)
	        priv->year = year;
	    if (month > 0 && month < 13)
                priv->month = month - 1;

            calendar_compute_days (calendar);
            gtk_widget_queue_draw (GTK_WIDGET (calendar));

            memset (priv->year_text, '\0', YEAR_INPUT_MAX);
            memset (priv->month_text, '\0', MONTH_INPUT_MAX);
	}
	return TRUE;	
    }

    if (event->keyval == GDK_KEY_Delete ||
	event->keyval == GDK_KEY_KP_Delete ||
	event->keyval == GDK_KEY_BackSpace)
    {
	if (priv->year_text_input)
	{
            if (priv->year_text_select)
            {
                memset (priv->year_text, '\0', YEAR_INPUT_MAX);
                return TRUE;
            } 

	    len = strlen (priv->year_text);
	    if (len > 0)
	    {
		priv->year_text[len - 1] = '\0';
        	calendar_redraw_component (calendar, priv->year_text_area);
	    }
	}

	if (priv->month_text_input)
	{
	    len = strlen (priv->month_text);
	    if (len > 0)
	    {
		priv->month_text[len - 1] = '\0';
        	calendar_redraw_component (calendar, priv->month_text_area);
	    }
	}
	return TRUE;	
    }

    num = get_key_event_num (event);
    if (num == INVALIDE_NUM)
	return TRUE;

    if (priv->year_text_input)
    {
        gchar tmp[YEAR_INPUT_MAX] = {'\0'};

	if (priv->year_text_select)
	{
	    memset (priv->year_text, '\0', YEAR_INPUT_MAX);
	    priv->year_text_select = FALSE;
	} 

	len = strlen (priv->year_text);
	if (len == YEAR_INPUT_MAX - 1)
	    return TRUE;

        strcpy (tmp, priv->year_text);
	g_snprintf(priv->year_text, YEAR_INPUT_MAX, "%s%d", tmp, num);
        calendar_redraw_component (calendar, priv->year_text_area);
    }

    if (priv->month_text_input)
    {
        gchar tmp[MONTH_INPUT_MAX] = {'\0'};

	if (priv->month_text_select)
	{
	    memset (priv->month_text, '\0', MONTH_INPUT_MAX);
	    priv->month_text_select = FALSE;
	} 

	len = strlen (priv->month_text);
	if (len == MONTH_INPUT_MAX - 1)
	    return TRUE;

        strcpy (tmp, priv->month_text);
	g_snprintf(priv->month_text, MONTH_INPUT_MAX, "%s%d", tmp, num);
        calendar_redraw_component (calendar, priv->month_text_area);
    }

    return TRUE;  
}



static gboolean 
kiran_calendar_focus_in (GtkWidget      *widget,
	                 GdkEventFocus  *event)
{
    KiranCalendar *calendar = KIRAN_CALENDAR (widget);
    KiranCalendarPrivate *priv = calendar->priv;

    priv->cursor_visible = TRUE;
    gtk_widget_queue_draw (widget);

    kiran_calendar_check_cursor_blink (calendar);

    return TRUE;
}

static gboolean 
kiran_calendar_focus_out (GtkWidget      *widget,
	                 GdkEventFocus  *event)
{
    KiranCalendar *calendar = KIRAN_CALENDAR (widget);
    KiranCalendarPrivate *priv = calendar->priv;

    if (priv->blink_timeout)
    {
         g_source_remove (priv->blink_timeout);
         priv->blink_timeout = 0;
    }

    priv->cursor_visible = FALSE;
    gtk_widget_queue_draw (widget);

    return TRUE;
}

static gchar *
get_lundar_strftime (LunarDate *lundate,
                     GDateYear  gyear,
                     GDateMonth gmonth,
                     GDateDay   gday,
                     guint8     hour,
                     const gchar *format)
{
    GError *error = NULL;
    gchar *value;

    if (lundate == NULL)
	return NULL;

    lunar_date_set_solar_date(lundate, gyear, gmonth, gday, hour, &error);

    if (error != NULL)
    {
        g_error_free(error);
	return NULL;
    }

    if (g_strcmp0 (format, "%(RI)") == 0)
    {
	value = lunar_date_strftime (lundate, "%(holiday)");
	if (value && strlen (value) > 0)
	    return value;
	else if (value)
	    g_free (value);
    }
    value = lunar_date_strftime(lundate, format);

    if (value && (g_strcmp0 (value, "初一") == 0))
    {
	g_free (value);
	value =  lunar_date_strftime(lundate, "%(YUE)月");
    }


    return value;
}

static void 
calendar_compute_today_lunar (KiranCalendar *calendar)
{
    KiranCalendarPrivate *priv = calendar->priv;
    time_t secs;
    struct tm *tm;
    gint month;
    gint year;
    gint day;
    gchar *value;

    secs = time (NULL);
    tm = localtime (&secs);
    month = tm->tm_mon + 1;
    year  = 1900 + tm->tm_year;
    day = tm->tm_mday;

    g_snprintf (priv->lunar[0], LUNAR_STR_LEN - 1, "%s", _("Today"));

    if (g_getenv ("LANG") && g_strrstr (g_getenv ("LANG"), "zh_CN"))
        value = get_lundar_strftime (priv->lundate, year, month, day, 12, "%(NIAN)%(shengxiao)年");
    else
   	value = get_lundar_strftime (priv->lundate, year, month, day, 12, "%(year)-%(month)-%(day)");

    if (value)
    {
        g_snprintf (priv->lunar[1], LUNAR_STR_LEN - 1, "%s", value);
        g_free (value);
    }

    if (g_getenv ("LANG") && g_strrstr (g_getenv ("LANG"), "zh_CN"))
        value = get_lundar_strftime (priv->lundate, year, month, day, 12, "%(YUE)月%(RI)");
    else
        value = get_lundar_strftime (priv->lundate, year, month, day, 12, " ");
	
    if (value)
    {
        g_snprintf (priv->lunar[2], LUNAR_STR_LEN - 1, "%s", value);
        g_free (value);
    }
}

static void 
calendar_compute_days (KiranCalendar *calendar)
{
    KiranCalendarPrivate *priv = calendar->priv;
    gint month;
    gint year;
    gint ndays_in_month;
    gint ndays_in_prev_month;
    gint first_day;
    gint row;
    gint col;
    gint day;
    
  
    year = priv->year;
    month = priv->month + 1;
    ndays_in_month = month_length[leap (year)][month];
  
    first_day = day_of_week (year, month, 1);
    first_day = (first_day + 7 - priv->week_start) % 7;
    if (first_day == 0)
        first_day = 7;
  
    /* Compute days of previous month */
    if (month > 1)
        ndays_in_prev_month = month_length[leap (year)][month - 1];
    else
        ndays_in_prev_month = month_length[leap (year - 1)][12];
    day = ndays_in_prev_month - first_day+ 1;

    for (col = 0; col < first_day; col++)
    {
        gchar *value;

        priv->day[0][col] = day;
        priv->day_month[0][col] = MONTH_PREV;
        value = get_lundar_strftime (priv->lundate, year, month - 1, day, 12, "%(RI)");
        if (value)
        {
	    g_snprintf (priv->day_lunar[0][col], LUNAR_STR_LEN - 1, "%s", value);
	    g_free (value);
        }
        day++;
    }
  
    /* Compute days of current month */
    row = first_day / 7;
    col = first_day % 7;
    for (day = 1; day <= ndays_in_month; day++)
    {
        gchar *value;

        priv->day[row][col] = day;
        priv->day_month[row][col] = MONTH_CURRENT;
        value = get_lundar_strftime (priv->lundate, year, month, day, 12, "%(RI)");
        if (value)
        {
	    g_snprintf (priv->day_lunar[row][col], LUNAR_STR_LEN - 1, "%s", value);
	    g_free (value);
        }
  
        col++;
        if (col == 7)
        {
            row++;
            col = 0;
        }
    }
  
    /* Compute days of next month */
    day = 1;
    for (; row <= 5; row++)
    {
        for (; col <= 6; col++)
        {
            gchar *value;
            priv->day[row][col] = day;
            priv->day_month[row][col] = MONTH_NEXT;
            value = get_lundar_strftime (priv->lundate, year, month + 1, day, 12, "%(RI)");
            if (value)
            {
                g_snprintf (priv->day_lunar[row][col], LUNAR_STR_LEN - 1, "%s", value);
                g_free (value);
            }
            day++;
        }
        col = 0;
    }
}

static void 
calendar_invalidate_day_num (KiranCalendar *calendar,
			     gint           day)
{
    KiranCalendarPrivate *priv = calendar->priv;
    gint r, c, row, col;
  
    row = -1;
    col = -1;
    for (r = 0; r < 6; r++)
        for (c = 0; c < 7; c++)
            if (priv->day_month[r][c] == MONTH_CURRENT &&
                priv->day[r][c] == day)
              {
                  row = r;
                  col = c;
              }
  
    g_return_if_fail (row != -1);
    g_return_if_fail (col != -1);

    calendar_invalidate_day (calendar, row, col);
}

static void
calendar_invalidate_day (KiranCalendar *calendar,
                         gint         row,
                         gint         col)
{
    GdkRectangle day_rect;
    GtkAllocation allocation;
  
    gtk_widget_get_allocation (GTK_WIDGET (calendar), &allocation);
    calendar_day_rectangle (calendar, row, col, &day_rect);
    day_rect.x -= DAY_RECT_LINE_WIDTH / 2;
    day_rect.y -= DAY_RECT_LINE_WIDTH / 2;
    day_rect.width += DAY_RECT_LINE_WIDTH;
    day_rect.height += DAY_RECT_LINE_WIDTH;
    gtk_widget_queue_draw_area (GTK_WIDGET (calendar),
                                allocation.x + day_rect.x,
                                allocation.y + day_rect.y,
                                day_rect.width, day_rect.height);
}

static void 
calendar_redraw_component (KiranCalendar *calendar,
                       GdkWindow     *win)
{
    GtkAllocation allocation;
    gint x, y, w, h;

    gtk_widget_get_allocation (GTK_WIDGET (calendar), &allocation);
    gdk_window_get_geometry (win, &x, &y, &w, &h);

    gtk_widget_queue_draw_area (GTK_WIDGET (calendar),
                                allocation.x + x,
                                allocation.y + y,
                                w, h);
}

static void 
calendar_arrows_change_state (KiranCalendar *calendar,
                              GdkWindow *event_window,
			      KiranCalendarState state)
{
    KiranCalendarPrivate *priv = calendar->priv;

    if (event_window == priv->arrow_win[ARROW_YEAR_PREV])
    {
        priv->prev_year_state = state;
	calendar_redraw_component (calendar, priv->arrow_win[ARROW_YEAR_PREV]);
    }

    if (event_window == priv->arrow_win[ARROW_YEAR_NEXT])
    {
        priv->next_year_state = state;
	calendar_redraw_component (calendar, priv->arrow_win[ARROW_YEAR_NEXT]);
    }

    if (event_window == priv->arrow_win[ARROW_MONTH_PREV])
    {
        priv->prev_month_state = state;
	calendar_redraw_component (calendar, priv->arrow_win[ARROW_MONTH_PREV]);
    }

    if (event_window == priv->arrow_win[ARROW_MONTH_NEXT])
    {
        priv->next_month_state = state;
	calendar_redraw_component (calendar, priv->arrow_win[ARROW_MONTH_NEXT]);
    }
}


/***************************************************
 *                  空开的API                      *
 ***************************************************/

/**
 * kiran_calendar_new: 
 *
 * 创建一个日历，并选择当前日期
 *
 * Retutns: 
 **/                

GtkWidget *
kiran_calendar_new (void)
{
    return g_object_new (KIRAN_TYPE_CALENDAR, NULL);
}

/**
 * kiran_caleadar_select_day:
 * @calendar: 一个#KiranCalendar
 * @day: 1到31之间的天数， 或者0以取消当前选定的天数
 *
 * 从当前月份中选择一天
 **/
void kiran_calendar_select_day (KiranCalendar *calendar,
			      guint         day)
{
    KiranCalendarPrivate *priv;

    g_return_if_fail (KIRAN_IS_CALENDAR (calendar));
    g_return_if_fail (day <= 31);

    priv = calendar->priv;
  
    if (priv->selected_day != day)
    {
	if (priv->selected_day > 0)
        {
            if (gtk_widget_is_drawable (GTK_WIDGET (calendar)))
                calendar_invalidate_day_num (calendar, priv->selected_day);
            priv->selected_day = 0;
        }

	priv->selected_day = day;
	if (gtk_widget_is_drawable (GTK_WIDGET (calendar)))
	    calendar_invalidate_day_num (calendar,  priv->selected_day);    
    }
}

/**
 * kiran_calendar_refresh:
 * @calendar: 一个#KiranCalendar
 *
 */
void 
kiran_calendar_refresh (KiranCalendar *calendar)
{
    KiranCalendarPrivate *priv;
    time_t secs;
    struct tm *tm;


    g_return_if_fail (KIRAN_IS_CALENDAR (calendar));
    priv = calendar->priv;
    
    secs = time (NULL);
    tm = localtime (&secs);
    priv->month = tm->tm_mon;
    priv->year  = 1900 + tm->tm_year;
    priv->selected_day = tm->tm_mday;

    priv->hover_day_row = -1;
    priv->hover_day_col = -1;

    priv->lundate = lunar_date_new ();

    calendar_compute_today_lunar (calendar);    
    calendar_compute_days (calendar);    

    priv->prev_year_state = KIRAN_NORMAL;
    priv->next_year_state = KIRAN_NORMAL;
    priv->prev_month_state = KIRAN_NORMAL;
    priv->next_month_state = KIRAN_NORMAL;
    priv->today_state = KIRAN_NORMAL;
    priv->year_text_input = FALSE;
    priv->month_text_input = FALSE;
    priv->year_input = FALSE;
    priv->month_input = FALSE;
    priv->cursor_visible = FALSE;
    priv->blink_timeout = 0;
    priv->year_text_select = FALSE;
    priv->year_text_select = FALSE;

    memset (priv->year_text, '\0', YEAR_INPUT_MAX);
    memset (priv->month_text, '\0', MONTH_INPUT_MAX);
}
