#include "idlist.h"
#include "terminal.h"
#include <gtk/gtk.h>
#include <stdlib.h>

typedef struct _window_elem window_elem;

struct _window_elem {
    GtkWidget *draw;
    GtkWidget *layout;
    int x, y;
    void *data;
};

static id_list *window_list = NULL;

void window_create(int id, int layout_id) {
    window_elem *w = malloc(sizeof(window_elem));
    w->draw = gtk_drawing_area_new();
    w->layout = layout_get_widget(layout_id);
    w->x = 0;
    w->y = 0;
    w->data = draw_data_new();
    g_signal_connect(G_OBJECT(w->draw), "draw", G_CALLBACK(draw_callback), w->data);
    gtk_container_add(GTK_CONTAINER(w->layout), w->draw);
    gtk_widget_show(w->draw);
    if (window_list == NULL)
        window_list = id_list_new();
    id_list_append(window_list, id, w);
}

void window_destroy(int id) {
    window_elem *w = id_list_remove(window_list, id);
    draw_destroy(w->data);
    draw_data_free(w->data);
    gtk_widget_destroy(GTK_WIDGET(w->draw));
    free(w);
}

window_elem *window_get(int id) { return (window_elem *)id_list_get_data(window_list, id); }

void *window_get_data(int id) { return window_get(id)->data; }

void window_size(int id, int x, int y, int width, int height) {
    window_elem *w = window_get(id);
    w->x = x;
    w->y = y;
    if (GTK_IS_LAYOUT(w->layout))
        gtk_layout_move(GTK_LAYOUT(w->layout), w->draw, w->x, w->y);
    else
        gtk_fixed_move(GTK_FIXED(w->layout), w->draw, w->x, w->y);
    gtk_widget_set_size_request(w->draw, width, height);
}

void window_raise(int id) {
    window_elem *w = window_get(id);
    g_object_ref(w->draw);
    gtk_container_remove(GTK_CONTAINER(w->layout), w->draw);
    gtk_container_add(GTK_CONTAINER(w->layout), w->draw);
    if (GTK_IS_LAYOUT(w->layout))
        gtk_layout_move(GTK_LAYOUT(w->layout), w->draw, w->x, w->y);
    else
        gtk_fixed_move(GTK_FIXED(w->layout), w->draw, w->x, w->y);
}

void window_redraw(int id) { gtk_widget_queue_draw(window_get(id)->draw); }
