#include "terminal.h"
#include <gtk/gtk.h>

#define APP_MENU_ELEM 1
#define TOP_MENU_ELEM 2
#define SUB_MENU_ELEM 3

typedef struct {
    GMenu *menu;
    char *label;
} menu_elem;

static GHashTable *menu_table = NULL;

typedef struct {
    GMenuItem *item;
    char *label;
    char *action;
} menu_item;

void menu_node_add(int id, int parent, char *label) {
    if (menu_table == NULL) {
        menu_table = g_hash_table_new(g_direct_hash, g_direct_equal);
    }
    menu_elem *m = g_malloc(sizeof(menu_elem));
    m->menu = g_menu_new();
    m->label = label;
    g_hash_table_insert(menu_table, GINT_TO_POINTER(id), m);
    if (parent == 0) {
        g_menu_append_submenu(barmenu, m->label, G_MENU_MODEL(m->menu));
    } else {
        menu_elem *p = g_hash_table_lookup(menu_table, GINT_TO_POINTER(parent));
        g_menu_append_submenu(p->menu, m->label, G_MENU_MODEL(m->menu));
    }
    g_object_unref(m->menu);
}

void menu_item_click(GSimpleAction *action, GVariant *parameter, gpointer p) {
    menu_item *m = (menu_item *)p;
    s_menu_action(m->action);
}

void menu_item_add(int id, int parent, char *label, char *action) {
    menu_item *i = g_malloc(sizeof(menu_item));
    i->item = g_menu_item_new(label, action);
    i->label = label;
    i->action = action;
    menu_elem *m = g_hash_table_lookup(menu_table, GINT_TO_POINTER(parent));
    g_menu_append_item(G_MENU(m->menu), i->item);
    g_object_unref(i->item);
    GSimpleAction *sa = g_simple_action_new(action + 4, NULL);
    g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(sa));
    g_signal_connect(sa, "activate", G_CALLBACK(menu_item_click), i);
}
