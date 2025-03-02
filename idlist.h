#ifndef _idlist_h_
#define _idlist_h_

typedef struct id_list_elem {
    void *data;
    struct id_list_elem *next;
} id_list_elem;

typedef struct {
    id_list_elem *root;
    id_list_elem *tail;
} id_list;

id_list *id_list_new();
void id_list_free(id_list *list);

void id_list_append(id_list *list, void *data);
void id_list_remove_all(id_list *list, void (*elem_destroy_func)(void *data));

static inline id_list_elem *id_list_root(id_list *list) { return list->root; }
static inline id_list_elem *id_list_elem_next(id_list_elem *elem) { return elem->next; }
static inline void *id_list_elem_data(id_list_elem *elem) { return elem->data; }

#endif
