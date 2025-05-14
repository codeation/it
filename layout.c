#include "terminal.h"
#include <gtk/gtk.h>

typedef struct {
    GtkWidget *layout;
    GtkWidget *parent;
    int x, y;
} layout_elem;

static GHashTable *layout_table = NULL;

GtkWidget *layout_get_widget(int id) {
    layout_elem *l = g_hash_table_lookup(layout_table, GINT_TO_POINTER(id));
    return l->layout;
}

void layout_create(int id, int parent_id) {
    layout_elem *l = g_malloc(sizeof(layout_elem));
    if (id == 1 && parent_id == 0) {
        l->layout = gtk_layout_new(NULL, NULL);
        l->parent = top;
        g_signal_connect(l->layout, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
        gtk_container_add(GTK_CONTAINER(l->parent), l->layout);
        gtk_widget_show_all(top);
    } else {
        l->layout = gtk_fixed_new();
        l->parent = layout_get_widget(parent_id);
        gtk_container_add(GTK_CONTAINER(l->parent), l->layout);
        gtk_widget_show(l->layout);
    }
    l->x = 0;
    l->y = 0;
    if (layout_table == NULL) {
        layout_table = g_hash_table_new(g_direct_hash, g_direct_equal);
    }
    g_hash_table_insert(layout_table, GINT_TO_POINTER(id), l);
}

void layout_destroy(int id) {
    layout_elem *l = g_hash_table_lookup(layout_table, GINT_TO_POINTER(id));
    g_hash_table_remove(layout_table, GINT_TO_POINTER(id));
    gtk_widget_destroy(GTK_WIDGET(l->layout));
    g_free(l);
}

void layout_size(int id, int x, int y, int width, int height) {
    layout_elem *l = g_hash_table_lookup(layout_table, GINT_TO_POINTER(id));
    if (!GTK_IS_LAYOUT(l->layout)) {
        l->x = x;
        l->y = y;
        if (GTK_IS_LAYOUT(l->parent)) {
            gtk_layout_move(GTK_LAYOUT(l->parent), l->layout, l->x, l->y);
        } else {
            gtk_fixed_move(GTK_FIXED(l->parent), l->layout, l->x, l->y);
        }
        gtk_widget_set_size_request(l->layout, width, height);
    }
}

void layout_raise(int id) {
    layout_elem *l = g_hash_table_lookup(layout_table, GINT_TO_POINTER(id));
    if (!GTK_IS_LAYOUT(l->layout)) {
        g_object_ref(l->layout);
        gtk_container_remove(GTK_CONTAINER(l->parent), l->layout);
        gtk_container_add(GTK_CONTAINER(l->parent), l->layout);
        g_object_unref(l->layout);
        if (GTK_IS_LAYOUT(l->parent)) {
            gtk_layout_move(GTK_LAYOUT(l->parent), l->layout, l->x, l->y);
        } else {
            gtk_fixed_move(GTK_FIXED(l->parent), l->layout, l->x, l->y);
        }
    }
}
