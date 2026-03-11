#include "terminal.h"
#include <gtk/gtk.h>

#define PTR_ARRAY_DEFAULT 16

static GPtrArray *menu_list = NULL;

void menubar_create() {
    menu_list = g_ptr_array_sized_new(PTR_ARRAY_DEFAULT);
    GMenu *menu = g_menu_new();
    g_ptr_array_add(menu_list, menu);
    gtk_application_set_menubar(app, G_MENU_MODEL(menu));
    g_object_unref(menu);
}

void menu_node_add(int id, int parent, char *label) {
    GMenu *menu = g_menu_new();
    for (int i = menu_list->len; i <= id; i++) {
        g_ptr_array_add(menu_list, NULL);
    }
    menu_list->pdata[id] = menu;
    GMenu *parent_menu = menu_list->pdata[parent];
    g_assert(parent_menu);
    g_menu_append_submenu(parent_menu, label, G_MENU_MODEL(menu));
    g_object_unref(menu);
}

static void menu_item_click(GSimpleAction *self, GVariant *parameter, gpointer data) {
    char *action = data;
    s_menu_action(action);
}

void menu_item_add(int id, int parent, char *label, char *action) {
    GMenuItem *item = g_menu_item_new(label, action);
    GMenu *menu = menu_list->pdata[parent];
    g_assert(menu);
    g_menu_append_item(menu, item);
    g_object_unref(item);
    GSimpleAction *sa = g_simple_action_new(action + 4, NULL);
    g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(sa));
    g_signal_connect(sa, "activate", G_CALLBACK(menu_item_click), action);
}
