#include "terminal.h"
#include <gtk/gtk.h>

#define PTR_ARRAY_DEFAULT 16

typedef struct {
    GtkWidget *draw;
    GtkWidget *layout;
    int x, y;
    GPtrArray *draw_list;
} WindowElem;

static GPtrArray *window_list = NULL;

static void draw_callback(GtkDrawingArea *widget, cairo_t *cr, int width, int height, gpointer draw_list) {
    cairo_save(cr);
    g_ptr_array_foreach(draw_list, draw_any_elem, cr);
    cairo_restore(cr);
}

void window_create(int id, int layout_id) {
    WindowElem *w = g_malloc(sizeof(WindowElem));
    w->draw = gtk_drawing_area_new();
    w->layout = layout_get_widget(layout_id);
    w->x = 0;
    w->y = 0;
    w->draw_list = g_ptr_array_new_full(PTR_ARRAY_DEFAULT, elem_draw_destroy);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(w->draw), draw_callback, w->draw_list, NULL);
    gtk_fixed_put(GTK_FIXED(w->layout), w->draw, 0, 0);
    gtk_widget_set_visible(w->draw, TRUE);
    if (window_list == NULL) {
        window_list = g_ptr_array_sized_new(PTR_ARRAY_DEFAULT);
    }
    for (int i = window_list->len; i <= id; i++) {
        g_ptr_array_add(window_list, NULL);
    }
    window_list->pdata[id] = w;
}

void window_destroy(int id) {
    WindowElem *w = window_list->pdata[id];
    g_assert(w);
    window_list->pdata[id] = NULL;
    g_ptr_array_free(w->draw_list, TRUE);
    gtk_fixed_remove(GTK_FIXED(w->layout), w->draw);
    g_free(w);
}

void window_clear(int id) {
    WindowElem *w = window_list->pdata[id];
    g_assert(w);
    g_ptr_array_remove_range(w->draw_list, 0, w->draw_list->len);
}

void window_add_draw(int id, gpointer e) {
    WindowElem *w = window_list->pdata[id];
    g_assert(w);
    g_ptr_array_add(w->draw_list, e);
}

void window_size(int id, int x, int y, int width, int height) {
    WindowElem *w = window_list->pdata[id];
    g_assert(w);
    w->x = x;
    w->y = y;
    gtk_fixed_move(GTK_FIXED(w->layout), w->draw, w->x, w->y);
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(w->draw), width);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(w->draw), height);
}

void window_raise(int id) {
    WindowElem *w = window_list->pdata[id];
    g_assert(w);
    g_object_ref(w->draw);
    gtk_fixed_remove(GTK_FIXED(w->layout), w->draw);
    gtk_fixed_put(GTK_FIXED(w->layout), w->draw, w->x, w->y);
    g_object_unref(w->draw);
}

void window_redraw(int id) {
    WindowElem *w = window_list->pdata[id];
    g_assert(w);
    gtk_widget_queue_draw(w->draw);
}
