#include "terminal.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif

GtkApplication *app = NULL;
GtkWidget *top = NULL;

GMenu *barmenu = NULL;

static char *pipe_suffix;

gboolean is_wayland_backend = FALSE;

static void on_app_activate(GApplication *application, gpointer data) {
    barmenu = g_menu_new();
    gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(barmenu));
    g_object_unref(barmenu);

    top = gtk_application_window_new(GTK_APPLICATION(app));

    g_signal_connect(top, "delete-event", G_CALLBACK(on_delete), NULL);
    g_signal_connect(top, "key_press_event", G_CALLBACK(s_keypress), NULL);
    g_signal_connect(top, "button_press_event", G_CALLBACK(s_button), NULL);
    g_signal_connect(top, "button_release_event", G_CALLBACK(s_button), NULL);
    g_signal_connect(top, "motion_notify_event", G_CALLBACK(s_motion), NULL);
    g_signal_connect(top, "scroll-event", G_CALLBACK(s_scroll), NULL);

#ifdef GDK_WINDOWING_WAYLAND
    GdkDisplayManager *display_manager = gdk_display_manager_get();
    if (GDK_IS_WAYLAND_DISPLAY(gdk_display_manager_get_default_display(display_manager))) {
        is_wayland_backend = TRUE;
    }
#endif

    pipe_init(pipe_suffix, async_read_chan);
}

static void on_app_shutdown(GApplication *application, gpointer data) { pipe_done(); }

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("suffix parameter missing\n");
        return 1;
    }
    pipe_suffix = argv[1];
    app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
    g_set_application_name("Impress");
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    g_signal_connect(app, "shutdown", G_CALLBACK(on_app_shutdown), NULL);

    int status = g_application_run(G_APPLICATION(app), 1, argv);
    g_object_unref(app);
    return status;
}
