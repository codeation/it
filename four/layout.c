#include "glib-object.h"
#include "terminal.h"
#include <gtk/gtk.h>

static GHashTable *layout_table = NULL;

typedef struct {
    GtkWidget *layout;
    GtkWidget *parent;
    GtkWidget *scrolled;
    GtkAdjustment *adjustment;
} layout_main;

static GtkWidget *layout_main_get_widget(int id) {
    layout_main *l = g_hash_table_lookup(layout_table, GINT_TO_POINTER(id));
    return l->layout;
}

static void layout_main_create(int id) {
    layout_main *l = g_malloc(sizeof(layout_main));
    l->layout = gtk_fixed_new();
    gtk_widget_set_overflow(l->layout, GTK_OVERFLOW_VISIBLE);
    l->parent = top;
    l->scrolled = gtk_scrolled_window_new();
    gtk_window_set_child(GTK_WINDOW(top), l->scrolled);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(l->scrolled), l->layout);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(l->scrolled), GTK_POLICY_EXTERNAL, GTK_POLICY_EXTERNAL);
    l->adjustment = gtk_adjustment_new(0, 0, 1, 0, 0, 1);
    gtk_scrolled_window_set_hadjustment(GTK_SCROLLED_WINDOW(l->scrolled), l->adjustment);
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(l->scrolled), l->adjustment);

    layout_signal_connect(l->scrolled, l->adjustment);

    gtk_widget_set_visible(l->scrolled, TRUE);
    gtk_widget_set_visible(l->layout, TRUE);
    gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(top), TRUE);
    gtk_window_present(GTK_WINDOW(top));

    g_hash_table_insert(layout_table, GINT_TO_POINTER(id), l);
}

static void layout_main_destroy(int id) {
    layout_main *l = g_hash_table_lookup(layout_table, GINT_TO_POINTER(id));
    g_hash_table_remove(layout_table, GINT_TO_POINTER(id));
    layout_signal_disconnect(l->scrolled, l->adjustment);
    gtk_scrolled_window_set_hadjustment(GTK_SCROLLED_WINDOW(l->scrolled), NULL);
    gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(l->scrolled), NULL);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(l->scrolled), NULL);
    gtk_window_set_child(GTK_WINDOW(top), NULL);
    g_free(l);
}

typedef struct {
    GtkWidget *layout;
    GtkWidget *parent;
    int x, y;
} layout_elem;

static GtkWidget *layout_node_get_widget(int id) {
    layout_elem *l = g_hash_table_lookup(layout_table, GINT_TO_POINTER(id));
    return l->layout;
}

static void layout_node_create(int id, int parent_id) {
    layout_elem *l = g_malloc(sizeof(layout_elem));
    l->layout = gtk_fixed_new();
    gtk_widget_set_overflow(l->layout, GTK_OVERFLOW_HIDDEN);
    l->parent = layout_get_widget(parent_id);
    gtk_fixed_put(GTK_FIXED(l->parent), l->layout, 0, 0);
    l->x = 0;
    l->y = 0;
    gtk_widget_set_visible(l->layout, TRUE);
    g_hash_table_insert(layout_table, GINT_TO_POINTER(id), l);
}

static void layout_node_destroy(int id) {
    layout_elem *l = g_hash_table_lookup(layout_table, GINT_TO_POINTER(id));
    g_hash_table_remove(layout_table, GINT_TO_POINTER(id));
    gtk_fixed_remove(GTK_FIXED(l->parent), l->layout);
    g_free(l);
}

static void layout_node_size(int id, int x, int y, int width, int height) {
    layout_elem *l = g_hash_table_lookup(layout_table, GINT_TO_POINTER(id));
    l->x = x;
    l->y = y;
    gtk_fixed_move(GTK_FIXED(l->parent), l->layout, x, y);
    gtk_widget_set_size_request(l->layout, width, height);
}

static void layout_node_raise(int id) {
    layout_elem *l = g_hash_table_lookup(layout_table, GINT_TO_POINTER(id));
    g_object_ref(l->layout);
    gtk_fixed_remove(GTK_FIXED(l->parent), l->layout);
    gtk_fixed_put(GTK_FIXED(l->parent), l->layout, l->x, l->y);
    g_object_unref(l->layout);
}

GtkWidget *layout_get_widget(int id) {
    if (id == 1) {
        return layout_main_get_widget(id);
    }
    return layout_node_get_widget(id);
}

void layout_create(int id, int parent_id) {
    if (layout_table == NULL) {
        layout_table = g_hash_table_new(g_direct_hash, g_direct_equal);
    }
    if (id == 1 && parent_id == 0) {
        layout_main_create(id);
    } else {
        layout_node_create(id, parent_id);
    }
}

void layout_destroy(int id) {
    if (id == 1) {
        layout_main_destroy(id);
    } else {
        layout_node_destroy(id);
    }
}

void layout_size(int id, int x, int y, int width, int height) {
    if (id != 1) {
        layout_node_size(id, x, y, width, height);
    }
}

void layout_raise(int id) {
    if (id != 1) {
        layout_node_raise(id);
    }
}
