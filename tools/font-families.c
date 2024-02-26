#include <gtk/gtk.h>
#include <stdio.h>

GtkApplication *app = NULL;

static void on_app_activate(GApplication *application, gpointer data) {
    GtkWidget *top = gtk_application_window_new(GTK_APPLICATION(app));

    PangoContext *context = gtk_widget_get_pango_context(GTK_WIDGET(top));
    PangoFontFamily **families;
    int family_length;
    pango_context_list_families(context, &families, &family_length);
    for (int i = 0; i < family_length; i++) {
        printf("family: %s\n", pango_font_family_get_name(families[i]));
    }
    g_free(families);

    g_application_quit(G_APPLICATION(app));
}

int main(int argc, char **argv) {
#ifndef GLIB_DEPRECATED_ENUMERATOR_IN_2_74_FOR
    app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
#else
    app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
#endif
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), 1, argv);
    g_object_unref(app);
    return status;
}
