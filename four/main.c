#include "terminal.h"
#include <gtk/gtk.h>

GtkApplication *app = NULL;
GtkWidget *top = NULL;

GMenu *barmenu = NULL;

static char *pipe_suffix;

static void on_app_activate(GApplication *application, gpointer data) {
    barmenu = g_menu_new();
    gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(barmenu));
    g_object_unref(barmenu);

    top = gtk_application_window_new(GTK_APPLICATION(app));
    top_signal_connect();

    pipe_init(pipe_suffix);
}

static void on_app_shutdown(GApplication *application, gpointer data) { pipe_done(); }

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("suffix parameter missing\n");
        return 1;
    }
    pipe_suffix = argv[1];

#ifndef GLIB_DEPRECATED_ENUMERATOR_IN_2_74_FOR
    app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
#else
    app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
#endif
    g_set_application_name("Impress");
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    g_signal_connect(app, "shutdown", G_CALLBACK(on_app_shutdown), NULL);

    int status = g_application_run(G_APPLICATION(app), 1, argv);

    g_signal_handlers_disconnect_by_func(app, G_CALLBACK(on_app_activate), NULL);
    g_signal_handlers_disconnect_by_func(app, G_CALLBACK(on_app_shutdown), NULL);

    g_object_unref(app);
    return status;
}
