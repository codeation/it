#include "idlist.h"
#include "terminal.h"
#include <gtk/gtk.h>
#include <stdlib.h>

typedef struct _layout_elem layout_elem;

struct _layout_elem {
    GtkWidget *layout;
    GtkWidget *parent;
    int x, y;
};

static id_list *layout_list = NULL;

layout_elem *layout_get(int id) { return (layout_elem *)id_list_get_data(layout_list, id); }

GtkWidget *layout_get_widget(int id) { return layout_get(id)->layout; }

void layout_create(int id, int parent_id) {
    layout_elem *l = malloc(sizeof(layout_elem));
    if (id == 1 && parent_id == 0) {
        l->layout = gtk_layout_new(NULL, NULL);
        l->parent = top;
    } else {
        l->layout = gtk_fixed_new();
        l->parent = layout_get_widget(parent_id);
    }
    l->x = 0;
    l->y = 0;
    gtk_container_add(GTK_CONTAINER(l->parent), l->layout);
    gtk_widget_show(l->layout);
    if (layout_list == NULL)
        layout_list = id_list_new();
    id_list_append(layout_list, id, l);
    if (id == 1) {
        gtk_widget_show_all(top);
        g_signal_connect(top, "configure-event", G_CALLBACK(on_configure), NULL);
    }
}

void layout_destroy(int id) {
    layout_elem *l = id_list_remove(layout_list, id);
    gtk_widget_destroy(GTK_WIDGET(l->layout));
    free(l);
}

void layout_size(int id, int x, int y, int width, int height) {
    layout_elem *l = layout_get(id);
    if (!GTK_IS_LAYOUT(l->layout)) {
        l->x = x;
        l->y = y;
        if (GTK_IS_LAYOUT(l->parent))
            gtk_layout_move(GTK_LAYOUT(l->parent), l->layout, l->x, l->y);
        else
            gtk_fixed_move(GTK_FIXED(l->parent), l->layout, l->x, l->y);
        gtk_widget_set_size_request(l->layout, width, height);
    }
}

void layout_raise(int id) {
    layout_elem *l = layout_get(id);
    if (!GTK_IS_LAYOUT(l->layout)) {
        g_object_ref(l->layout);
        gtk_container_remove(GTK_CONTAINER(l->parent), l->layout);
        gtk_container_add(GTK_CONTAINER(l->parent), l->layout);
        if (GTK_IS_LAYOUT(l->parent))
            gtk_layout_move(GTK_LAYOUT(l->parent), l->layout, l->x, l->y);
        else
            gtk_fixed_move(GTK_FIXED(l->parent), l->layout, l->x, l->y);
    }
}
