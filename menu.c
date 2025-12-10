#include "terminal.h"
#include <gtk/gtk.h>

static GHashTable *menu_table = NULL;

void menubar_create() {
    menu_table = g_hash_table_new(g_direct_hash, g_direct_equal);
    GMenu *menu = g_menu_new();
    g_hash_table_insert(menu_table, GINT_TO_POINTER(0), menu);
    gtk_application_set_menubar(app, G_MENU_MODEL(menu));
    g_object_unref(menu);
}

void menu_node_add(int id, int parent, char *label) {
    GMenu *menu = g_menu_new();
    g_hash_table_insert(menu_table, GINT_TO_POINTER(id), menu);
    GMenu *parent_menu = g_hash_table_lookup(menu_table, GINT_TO_POINTER(parent));
    g_menu_append_submenu(parent_menu, label, G_MENU_MODEL(menu));
    g_object_unref(menu);
}

static void menu_item_click(GSimpleAction *self, GVariant *parameter, gpointer data) {
    char *action = data;
    s_menu_action(action);
}

void menu_item_add(int id, int parent, char *label, char *action) {
    GMenuItem *item = g_menu_item_new(label, action);
    GMenu *menu = g_hash_table_lookup(menu_table, GINT_TO_POINTER(parent));
    g_menu_append_item(menu, item);
    g_object_unref(item);
    GSimpleAction *sa = g_simple_action_new(action + 4, NULL);
    g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(sa));
    g_signal_connect(sa, "activate", G_CALLBACK(menu_item_click), action);
}
