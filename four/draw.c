#include "terminal.h"
#include <gtk/gtk.h>

#define DRAW_ELEM_FILL 1
#define DRAW_ELEM_LINE 2
#define DRAW_ELEM_TEXT 3
#define DRAW_ELEM_IMAGE 4

// general

typedef struct {
    int type;
} type_elem;

// fill

typedef struct {
    int type;
    double x, y, width, height;
    double r, g, b, a;
} fill_elem;

static inline void elem_fill_draw(cairo_t *cr, fill_elem *e) {
    cairo_set_source_rgba(cr, e->r, e->g, e->b, e->a);
    cairo_rectangle(cr, e->x, e->y, e->width, e->height);
    cairo_fill(cr);
}

void elem_fill_add(int id, double x, double y, double width, double height, double r, double g, double b, double a) {
    fill_elem *e = g_malloc(sizeof(fill_elem));
    e->type = DRAW_ELEM_FILL;
    e->x = x;
    e->y = y;
    e->width = width;
    e->height = height;
    e->r = r;
    e->g = g;
    e->b = b;
    e->a = a;
    window_add_draw(id, e);
}

// line

typedef struct {
    int type;
    double x0, y0, x1, y1;
    double r, g, b, a;
} line_elem;

static inline void elem_line_draw(cairo_t *cr, line_elem *e) {
    cairo_set_source_rgba(cr, e->r, e->g, e->b, e->a);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, e->x0, e->y0);
    cairo_line_to(cr, e->x1, e->y1);
    cairo_stroke(cr);
}

void elem_line_add(int id, double x0, double y0, double x1, double y1, double r, double g, double b, double a) {
    if (x0 == x1) {
        x0 += 0.5;
        x1 += 0.5;
    } else if (y0 == y1) {
        y0 += 0.5;
        y1 += 0.5;
    }
    line_elem *e = g_malloc(sizeof(line_elem));
    e->type = DRAW_ELEM_LINE;
    e->x0 = x0;
    e->y0 = y0;
    e->x1 = x1;
    e->y1 = y1;
    e->r = r;
    e->g = g;
    e->b = b;
    e->a = a;
    window_add_draw(id, e);
}

// bitmap cache

typedef struct {
    unsigned char *data;
    double width, height;
    cairo_surface_t *bitmap;
} bitmap_elem;

static GHashTable *bitmap_table = NULL;

void bitmap_add(int id, int width, int height, unsigned char *data) {
    int cairo_format = CAIRO_FORMAT_ARGB32;
    int stride = cairo_format_stride_for_width(cairo_format, width);
    unsigned char *row = data;
    for (int i = 0; i < height; i++) {
        unsigned char *p = row;
        for (int j = 0; j < width; j++) {
            unsigned char r = p[0];
            p[0] = p[2];
            p[2] = r;
            p += 4;
        }
        row += stride;
    }
    bitmap_elem *e = g_malloc(sizeof(bitmap_elem));
    e->data = data;
    e->width = width;
    e->height = height;
    e->bitmap = cairo_image_surface_create_for_data(e->data, cairo_format, width, height, stride);
    if (bitmap_table == NULL) {
        bitmap_table = g_hash_table_new(g_direct_hash, g_direct_equal);
    }
    g_hash_table_insert(bitmap_table, GINT_TO_POINTER(id), e);
}

void bitmap_rem(int id) {
    bitmap_elem *e = g_hash_table_lookup(bitmap_table, GINT_TO_POINTER(id));
    g_hash_table_remove(bitmap_table, GINT_TO_POINTER(id));
    cairo_surface_destroy(e->bitmap);
    g_free(e->data);
    g_free(e);
}

// image drawing

typedef struct {
    int type;
    double x, y;
    double scale_x, scale_y;
    cairo_surface_t *bitmap;
} image_elem;

static inline void elem_image_draw(cairo_t *cr, image_elem *e) {
    cairo_surface_set_device_scale(e->bitmap, e->scale_x, e->scale_y);
    cairo_set_source_surface(cr, e->bitmap, e->x, e->y);
    cairo_paint(cr);
}

void elem_image_add(int id, double x, double y, double width, double height, int imageid) {
    image_elem *e = g_malloc(sizeof(image_elem));
    bitmap_elem *b = g_hash_table_lookup(bitmap_table, GINT_TO_POINTER(imageid));
    e->type = DRAW_ELEM_IMAGE;
    e->x = x;
    e->y = y;
    e->scale_x = b->width / width;
    e->scale_y = b->height / height;
    e->bitmap = b->bitmap;
    window_add_draw(id, e);
}

// font

typedef struct {
    int height;
    PangoFontDescription *desc;
    PangoLayout *layout;
    PangoLayout *split_layout;
} font_elem;

static GHashTable *font_table = NULL;

static PangoContext *top_pango_context = NULL;

void font_elem_add(int id, int height, char *family, int style, int variant, int weight, int stretch) {
    if (top_pango_context == NULL) {
        top_pango_context = gtk_widget_get_pango_context(top);
    }
    font_elem *e = g_malloc(sizeof(font_elem));
    e->height = height;
    e->desc = pango_font_description_new();
    pango_font_description_set_family(e->desc, family);
    pango_font_description_set_absolute_size(e->desc, PANGO_SCALE * height);
    pango_font_description_set_style(e->desc, style);
    pango_font_description_set_variant(e->desc, variant);
    pango_font_description_set_weight(e->desc, weight);
    pango_font_description_set_stretch(e->desc, stretch);
    e->layout = pango_layout_new(top_pango_context);
    pango_layout_set_font_description(e->layout, e->desc);
    e->split_layout = pango_layout_new(top_pango_context);
    pango_layout_set_font_description(e->split_layout, e->desc);
    pango_layout_set_wrap(e->split_layout, PANGO_WRAP_WORD_CHAR);
    if (font_table == NULL) {
        font_table = g_hash_table_new(g_direct_hash, g_direct_equal);
    }
    g_hash_table_insert(font_table, GINT_TO_POINTER(id), e);
}

void font_elem_rem(int id) {
    font_elem *e = g_hash_table_lookup(font_table, GINT_TO_POINTER(id));
    g_hash_table_remove(font_table, GINT_TO_POINTER(id));
    g_object_unref(e->split_layout);
    g_object_unref(e->layout);
    pango_font_description_free(e->desc);
    g_free(e);
}

void get_font_metrics(int fontid, int16_t *lineheight, int16_t *baseline, int16_t *ascent, int16_t *descent) {
    font_elem *f = g_hash_table_lookup(font_table, GINT_TO_POINTER(fontid));
    *baseline = (int16_t)(pango_layout_get_baseline(f->layout) / PANGO_SCALE);
    PangoFontMetrics *metrics = pango_context_get_metrics(pango_layout_get_context(f->layout), f->desc, NULL);
    *lineheight = (int16_t)(pango_font_metrics_get_height(metrics) / PANGO_SCALE);
    *ascent = (int16_t)(pango_font_metrics_get_ascent(metrics) / PANGO_SCALE);
    *descent = (int16_t)(pango_font_metrics_get_descent(metrics) / PANGO_SCALE);
    pango_font_metrics_unref(metrics);
}

// text split

int16_t *font_split_text(int fontid, char *text, int edge, int indent) {
    font_elem *f = g_hash_table_lookup(font_table, GINT_TO_POINTER(fontid));
    pango_layout_set_width(f->split_layout, PANGO_SCALE * edge);
    pango_layout_set_indent(f->split_layout, PANGO_SCALE * indent);
    pango_layout_set_text(f->split_layout, text, -1);
    GSList *top = pango_layout_get_lines_readonly(f->split_layout);
    int16_t length = 0;
    for (GSList *e = top; e != NULL; e = e->next) {
        length++;
    }
    int16_t *out = g_malloc(sizeof(int16_t) * (length + 1));
    int16_t *pos = out;
    *pos++ = length;
    for (GSList *e = top; e != NULL; e = e->next) {
        PangoLayoutLine *line = e->data;
        *pos++ = (int16_t)(line->length);
    }
    return out;
}

// text rect

void font_rect_text(int fontid, char *text, int16_t *width, int16_t *height) {
    font_elem *f = g_hash_table_lookup(font_table, GINT_TO_POINTER(fontid));
    pango_layout_set_text(f->layout, text, -1);
    int w, h;
    pango_layout_get_pixel_size(f->layout, &w, &h);
    *width = (int16_t)w;
    *height = (int16_t)h;
}

// text

typedef struct {
    int type;
    double x, y;
    char *text;
    PangoLayout *layout;
    double r, g, b, a;
} text_elem;

static inline void elem_text_draw(cairo_t *cr, text_elem *e) {
    pango_layout_set_text(e->layout, e->text, -1);
    cairo_set_source_rgba(cr, e->r, e->g, e->b, e->a);
    cairo_move_to(cr, e->x, e->y);
    pango_cairo_show_layout(cr, e->layout);
}

static inline void elem_text_destroy(text_elem *e) { g_free(e->text); }

void elem_text_add(int id, double x, double y, char *text, int fontid, double r, double g, double b, double a) {
    font_elem *f = g_hash_table_lookup(font_table, GINT_TO_POINTER(fontid));
    text_elem *e = g_malloc(sizeof(text_elem));
    e->type = DRAW_ELEM_TEXT;
    e->x = x;
    e->y = y;
    e->text = text;
    e->layout = f->layout;
    e->r = r;
    e->g = g;
    e->b = b;
    e->a = a;
    window_add_draw(id, e);
}

// any elem

void draw_any_elem(gpointer e, gpointer cr) {
    switch (((type_elem *)e)->type) {
    case DRAW_ELEM_FILL:
        elem_fill_draw(cr, e);
        break;
    case DRAW_ELEM_LINE:
        elem_line_draw(cr, e);
        break;
    case DRAW_ELEM_TEXT:
        elem_text_draw(cr, e);
        break;
    case DRAW_ELEM_IMAGE:
        elem_image_draw(cr, e);
        break;
    }
}

void elem_draw_destroy(gpointer e) {
    switch (((type_elem *)e)->type) {
    case DRAW_ELEM_TEXT:
        elem_text_destroy(e);
        break;
    }
    g_free(e);
}
