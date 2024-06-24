#include "idlist.h"
#include <glib.h>
#include <stdio.h>

id_list *id_list_new() {
    id_list *list = g_malloc(sizeof(id_list));
    list->root = NULL;
    list->tail = NULL;
    list->cache_id = -1;
    list->cache_elem = NULL;
    return list;
}

void id_list_free(id_list *list) { g_free(list); }

void id_list_append(id_list *list, int id, void *data) {
    id_list_elem *e = g_malloc(sizeof(id_list_elem));
    e->id = id;
    e->data = data;
    e->next = NULL;
    if (list->root == NULL)
        list->root = e;
    if (list->tail != NULL)
        list->tail->next = e;
    list->tail = e;
}

void *id_list_get_data(id_list *list, int id) {
    if (list->cache_id == id) {
        return list->cache_elem->data;
    }
    for (id_list_elem *e = list->root; e != NULL; e = e->next) {
        if (e->id == id) {
            list->cache_id = id;
            list->cache_elem = e;
            return e->data;
        }
    }
    printf("id_list_get_data, ID = %d not found\n", id);
    exit(EXIT_FAILURE);
    return NULL;
}

void *id_list_remove(id_list *list, int id) {
    list->cache_id = -1;
    list->cache_elem = NULL;
    void *data = NULL;
    id_list_elem *prev = NULL;
    for (id_list_elem *e = list->root; e != NULL; e = e->next) {
        if (e->id == id) {
            data = e->data;
            if (prev == NULL)
                list->root = e->next;
            else
                prev->next = e->next;
            if (e->next == NULL)
                list->tail = prev;
            g_free(e);
            break;
        }
        prev = e;
    }
    if (data == NULL) {
        printf("id_list_remove, ID = %d not found\n", id);
        exit(EXIT_FAILURE);
    }
    return data;
}

void id_list_remove_all(id_list *list, void (*elem_destroy_func)(void *data)) {
    list->cache_id = -1;
    list->cache_elem = NULL;
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
