#include "terminal.h"
#include <gtk/gtk.h>

#ifndef GLIB_DEPRECATED_ENUMERATOR_IN_2_74_FOR
static GApplicationFlags app_flags = G_APPLICATION_FLAGS_NONE;
#else
static GApplicationFlags app_flags = G_APPLICATION_DEFAULT_FLAGS;
#endif

GtkApplication *app = NULL;
GtkWidget *top = NULL;

static char *pipe_suffix = NULL;

static void on_app_activate(GApplication *application, gpointer data) {
    menubar_create();

    top = gtk_application_window_new(app);
    top_signal_connect();

    pipe_init(pipe_suffix);
}

static void on_app_shutdown(GApplication *application, gpointer data) { pipe_done(); }

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("suffix parameter missing\n");
        return EXIT_FAILURE;
    }
    pipe_suffix = argv[1];

    app = gtk_application_new(NULL, app_flags);
    g_set_application_name("Impress");
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    g_signal_connect(app, "shutdown", G_CALLBACK(on_app_shutdown), NULL);

    int status = g_application_run(G_APPLICATION(app), 1, argv);

    g_signal_handlers_disconnect_by_func(app, G_CALLBACK(on_app_activate), NULL);
    g_signal_handlers_disconnect_by_func(app, G_CALLBACK(on_app_shutdown), NULL);

    g_object_unref(app);
    return status;
}
