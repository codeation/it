#include "idlist.h"
#include "terminal.h"
#include <gtk/gtk.h>

#define APP_MENU_ELEM 1
#define TOP_MENU_ELEM 2
#define SUB_MENU_ELEM 3

typedef struct _menu_elem menu_elem;

struct _menu_elem {
    GMenu *menu;
    char *label;
};

static id_list *menu_list = NULL;

typedef struct _menu_item menu_item;

struct _menu_item {
    GMenuItem *item;
    char *label;
    char *action;
};

static id_list *menu_item_list = NULL;

void menu_node_add(int id, int parent, char *label) {
    if (menu_list == NULL) {
        menu_list = id_list_new();
    }
    menu_elem *m = malloc(sizeof(menu_elem));
    m->menu = g_menu_new();
    m->label = label;
    id_list_append(menu_list, id, m);
    if (parent == 0) {
        g_menu_append_submenu(barmenu, m->label, G_MENU_MODEL(m->menu));
    } else {
        menu_elem *p = (menu_elem *)id_list_get_data(menu_list, parent);
        g_menu_append_submenu(p->menu, m->label, G_MENU_MODEL(m->menu));
    }
    g_object_unref(m->menu);
}

void menu_item_click(GSimpleAction *action, GVariant *parameter, gpointer p) {
    menu_item *m = (menu_item *)p;
    s_menu_action(m->action);
}

void menu_item_add(int id, int parent, char *label, char *action) {
    if (menu_item_list == NULL) {
        menu_item_list = id_list_new();
    }
    menu_item *m = malloc(sizeof(menu_item));
    m->item = g_menu_item_new(label, action);
    m->label = label;
    m->action = action;
    id_list_append(menu_item_list, id, m);
    menu_elem *p = (menu_elem *)id_list_get_data(menu_list, parent);
    GSimpleAction *sa = g_simple_action_new(action + 4, NULL);
    g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(sa));
    g_signal_connect(G_OBJECT(sa), "activate", G_CALLBACK(menu_item_click), m);
    GMenuItem *item = g_menu_item_new(label, action);
    g_menu_append_item(G_MENU(p->menu), item);
    g_object_unref(item);
}
