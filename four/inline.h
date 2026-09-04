#ifndef _INLINE_H_
#define _INLINE_H_

#include <gtk/gtk.h>

#define PTR_ARRAY_DEFAULT 16

static inline void ptr_array_grow(GPtrArray **a, int index) {
    if (*a == NULL) {
        *a = g_ptr_array_sized_new(PTR_ARRAY_DEFAULT);
    }
    for (int i = (*a)->len; i <= index; i++) {
        g_ptr_array_add(*a, NULL);
    }
}

#endif
