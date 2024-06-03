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
        PangoFontFace **faces;
        int face_length;
        pango_font_family_list_faces(families[i], &faces, &face_length);

        printf("%s [", pango_font_family_get_name(families[i]));
        for (int j = 0; j < face_length; j++) {
            if (j != 0)
                printf(", ");
            printf("%s", pango_font_face_get_face_name(faces[j]));
        }
        printf("]\n");

        g_free(faces);
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
