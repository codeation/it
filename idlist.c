#include "idlist.h"
#include <glib.h>

id_list *id_list_new() {
    id_list *list = g_malloc(sizeof(id_list));
    list->root = NULL;
    list->tail = NULL;
    return list;
}

void id_list_free(id_list *list) { g_free(list); }

void id_list_append(id_list *list, void *data) {
    id_list_elem *e = g_malloc(sizeof(id_list_elem));
    e->data = data;
    e->next = NULL;
    if (list->root == NULL)
        list->root = e;
    if (list->tail != NULL)
        list->tail->next = e;
    list->tail = e;
}

void id_list_remove_all(id_list *list, void (*elem_destroy_func)(void *data)) {
    id_list_elem *e = list->root;
    while (e != NULL) {
        id_list_elem *next = e->next;
        elem_destroy_func(e->data);
        g_free(e);
        e = next;
    }
    list->root = NULL;
    list->tail = NULL;
}
