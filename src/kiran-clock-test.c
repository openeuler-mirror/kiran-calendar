#include <gtk/gtk.h>
#include "kiran-clock.h"
#include "kiran-calendar-window.h"

static gboolean
button_press (GtkWidget      *widget,
	      GdkEventButton *event,
	      gpointer       user_data)
{
    KiranCalendarWindow *window  = KIRAN_CALENDAR_WINDOW (user_data);

    if (gtk_widget_is_visible (GTK_WIDGET (window)))
        kiran_calendar_window_hide (window);
    else
        kiran_calendar_window_show (window);

    return TRUE;
}

int
main (int argc,
      char **argv)
{
    GtkWidget *window;
    GtkWidget *calendar_window;
    GtkWidget *clock;
    GError *error = NULL;

    if (!gtk_init_with_args (&argc, &argv, NULL, NULL, NULL, &error))
    {
	fprintf (stderr, "%s", error->message);
	g_error_free (error);
        exit (1);
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Test Clock");
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

    calendar_window = kiran_calendar_window_new ();

    clock = kiran_clock_new ();
    gtk_container_add (GTK_CONTAINER (window), clock);

    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    g_signal_connect (clock, "button-press-event", G_CALLBACK(button_press), calendar_window);

    gtk_widget_show (clock);
    gtk_widget_show (window);

    gtk_main ();

    gtk_widget_destroy (calendar_window);

    return EXIT_SUCCESS;
}
