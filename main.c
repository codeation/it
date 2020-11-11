#include "terminal.h"
#include <gtk/gtk.h>

GtkApplication *app = NULL;

GtkWidget *top = NULL;
GtkWidget *layout = NULL;

GMenu *appmenu = NULL;
GMenu *barmenu = NULL;

static void on_app_activate(GApplication *application, gpointer data) {
    appmenu = g_menu_new();
    gtk_application_set_app_menu(GTK_APPLICATION(app), G_MENU_MODEL(appmenu));
    g_object_unref(appmenu);
    barmenu = g_menu_new();
    gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(barmenu));
    g_object_unref(barmenu);

    top = gtk_application_window_new(GTK_APPLICATION(app));
    g_signal_connect(top, "delete-event", G_CALLBACK(on_delete), NULL);
    g_signal_connect(top, "key_press_event", G_CALLBACK(s_keypress), NULL);
    g_signal_connect(top, "button_press_event", G_CALLBACK(s_button), NULL);
    g_signal_connect(top, "button_release_event", G_CALLBACK(s_button), NULL);
    g_signal_connect(top, "motion_notify_event", G_CALLBACK(s_motion), NULL);

    layout = gtk_layout_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(top), layout);

    network_init();
}

static void on_app_shutdown(GApplication *application, gpointer data) { network_done(); }

int main(int argc, char **argv) {
    app = gtk_application_new("com.github.codeation.impress", G_APPLICATION_FLAGS_NONE);
    g_set_application_name("Impress");
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    g_signal_connect(app, "shutdown", G_CALLBACK(on_app_shutdown), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
