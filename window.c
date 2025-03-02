#include "idlist.h"
#include "terminal.h"
#include <gtk/gtk.h>

typedef struct {
    GtkWidget *draw;
    GtkWidget *layout;
    int x, y;
    id_list *draw_list;
} window_elem;

static GHashTable *window_table = NULL;

void window_create(int id, int layout_id) {
    window_elem *w = g_malloc(sizeof(window_elem));
    w->draw = gtk_drawing_area_new();
    w->layout = layout_get_widget(layout_id);
    w->x = 0;
    w->y = 0;
    w->draw_list = id_list_new();
    g_signal_connect(G_OBJECT(w->draw), "draw", G_CALLBACK(draw_callback), w->draw_list);
    gtk_container_add(GTK_CONTAINER(w->layout), w->draw);
    gtk_widget_show(w->draw);
    if (window_table == NULL)
        window_table = g_hash_table_new(g_direct_hash, g_direct_equal);
    g_hash_table_insert(window_table, GINT_TO_POINTER(id), w);
}

void window_destroy(int id) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    g_hash_table_remove(window_table, GINT_TO_POINTER(id));
    id_list_remove_all(w->draw_list, elem_draw_destroy);
    id_list_free(w->draw_list);
    gtk_widget_destroy(GTK_WIDGET(w->draw));
    g_free(w);
}

void window_clear(int id) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    id_list_remove_all(w->draw_list, elem_draw_destroy);
}

void window_add_draw(int id, void *data) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    id_list_append(w->draw_list, data);
}

void window_size(int id, int x, int y, int width, int height) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    w->x = x;
    w->y = y;
    if (GTK_IS_LAYOUT(w->layout))
        gtk_layout_move(GTK_LAYOUT(w->layout), w->draw, w->x, w->y);
    else
        gtk_fixed_move(GTK_FIXED(w->layout), w->draw, w->x, w->y);
    gtk_widget_set_size_request(w->draw, width, height);
}

void window_raise(int id) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    g_object_ref(w->draw);
    gtk_container_remove(GTK_CONTAINER(w->layout), w->draw);
    gtk_container_add(GTK_CONTAINER(w->layout), w->draw);
    g_object_unref(w->draw);
    if (GTK_IS_LAYOUT(w->layout))
        gtk_layout_move(GTK_LAYOUT(w->layout), w->draw, w->x, w->y);
    else
        gtk_fixed_move(GTK_FIXED(w->layout), w->draw, w->x, w->y);
}

void window_redraw(int id) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    gtk_widget_queue_draw(w->draw);
}
