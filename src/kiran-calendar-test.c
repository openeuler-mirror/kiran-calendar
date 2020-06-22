#include <gtk/gtk.h>

int
main (int argc,
      char **argv)
{
    GtkWidget *window;
    GError *error = NULL;

    if (!gtk_init_with_args (&argc, &argv, NULL, NULL, NULL, &error))
    {
	fprintf (stderr, "%s", error->message);
	g_error_free (error);
        exit (1);
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Test Calendar");
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request (window, 400, 400);

    gtk_widget_show (window);

    gtk_main ();

    return EXIT_SUCCESS;
}
