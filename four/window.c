#include "terminal.h"
#include <gtk/gtk.h>

typedef struct {
    GtkWidget *draw;
    GtkWidget *layout;
    int x, y;
    GPtrArray *draw_list;
} window_elem;

static GHashTable *window_table = NULL;

void window_create(int id, int layout_id) {
    window_elem *w = g_malloc(sizeof(window_elem));
    w->draw = gtk_drawing_area_new();
    w->layout = layout_get_widget(layout_id);
    w->x = 0;
    w->y = 0;
    w->draw_list = g_ptr_array_new_with_free_func(elem_draw_destroy);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(w->draw), draw_callback, w->draw_list, NULL);
    gtk_widget_set_parent(w->draw, w->layout);
    gtk_widget_set_visible(w->draw, TRUE);
    if (window_table == NULL) {
        window_table = g_hash_table_new(g_direct_hash, g_direct_equal);
    }
    g_hash_table_insert(window_table, GINT_TO_POINTER(id), w);
}

void window_destroy(int id) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    g_hash_table_remove(window_table, GINT_TO_POINTER(id));
    g_ptr_array_free(w->draw_list, TRUE);
    gtk_widget_unparent(w->draw);
    g_free(w);
}

void window_clear(int id) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    g_ptr_array_remove_range(w->draw_list, 0, w->draw_list->len);
}

void window_add_draw(int id, void *data) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    g_ptr_array_add(w->draw_list, data);
}

void window_size(int id, int x, int y, int width, int height) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    w->x = x;
    w->y = y;
    gtk_fixed_move(GTK_FIXED(w->layout), w->draw, x, y);
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(w->draw), width);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(w->draw), height);
}

void window_raise(int id) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    g_object_ref(w->draw);
    gtk_widget_unparent(w->draw);
    gtk_widget_set_parent(w->draw, w->layout);
    g_object_unref(w->draw);
    gtk_fixed_move(GTK_FIXED(w->layout), w->draw, w->x, w->y);
}

void window_redraw(int id) {
    window_elem *w = g_hash_table_lookup(window_table, GINT_TO_POINTER(id));
    gtk_widget_queue_draw(w->draw);
}
