#include "terminal.h"
#include <gtk/gtk.h>

// draw elem

#define DRAW_ELEM_FILL 1
#define DRAW_ELEM_LINE 2
#define DRAW_ELEM_TEXT 3
#define DRAW_ELEM_IMAGE 4

typedef struct {
    int type;
    union {
        struct {
            double x, y, width, height;
            double r, g, b, a;
        } fill;
        struct {
            double x0, y0, x1, y1;
            double r, g, b, a;
        } line;
        struct {
            double x, y;
            double scale_x, scale_y;
            cairo_surface_t *bitmap;
        } image;
        struct {
            double x, y;
            char *text;
            PangoLayout *layout;
            double r, g, b, a;
        } text;
    } data;
} DrawElem;

// fill

static inline void elem_fill_draw(cairo_t *cr, DrawElem *e) {
    cairo_set_source_rgba(cr, e->data.fill.r, e->data.fill.g, e->data.fill.b, e->data.fill.a);
    cairo_rectangle(cr, e->data.fill.x, e->data.fill.y, e->data.fill.width, e->data.fill.height);
    cairo_fill(cr);
}

void elem_fill_add(int id, double x, double y, double width, double height, double r, double g, double b, double a) {
    DrawElem *e = g_malloc(sizeof(DrawElem));
    e->type = DRAW_ELEM_FILL;
    e->data.fill.x = x;
    e->data.fill.y = y;
    e->data.fill.width = width;
    e->data.fill.height = height;
    e->data.fill.r = r;
    e->data.fill.g = g;
    e->data.fill.b = b;
    e->data.fill.a = a;
    window_add_draw(id, e);
}

// line

static inline void elem_line_draw(cairo_t *cr, DrawElem *e) {
    cairo_set_source_rgba(cr, e->data.line.r, e->data.line.g, e->data.line.b, e->data.line.a);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, e->data.line.x0, e->data.line.y0);
    cairo_line_to(cr, e->data.line.x1, e->data.line.y1);
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
    DrawElem *e = g_malloc(sizeof(DrawElem));
    e->type = DRAW_ELEM_LINE;
    e->data.line.x0 = x0;
    e->data.line.y0 = y0;
    e->data.line.x1 = x1;
    e->data.line.y1 = y1;
    e->data.line.r = r;
    e->data.line.g = g;
    e->data.line.b = b;
    e->data.line.a = a;
    window_add_draw(id, e);
}

// bitmap cache

typedef struct {
    unsigned char *data;
    double width, height;
    cairo_surface_t *bitmap;
} BitmapElem;

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
    BitmapElem *e = g_malloc(sizeof(BitmapElem));
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
    BitmapElem *e = g_hash_table_lookup(bitmap_table, GINT_TO_POINTER(id));
    g_assert(e);
    g_hash_table_remove(bitmap_table, GINT_TO_POINTER(id));
    cairo_surface_destroy(e->bitmap);
    g_free(e->data);
    g_free(e);
}

// image drawing

static inline void elem_image_draw(cairo_t *cr, DrawElem *e) {
    cairo_surface_set_device_scale(e->data.image.bitmap, e->data.image.scale_x, e->data.image.scale_y);
    cairo_set_source_surface(cr, e->data.image.bitmap, e->data.image.x, e->data.image.y);
    cairo_paint(cr);
}

void elem_image_add(int id, double x, double y, double width, double height, int imageid) {
    BitmapElem *b = g_hash_table_lookup(bitmap_table, GINT_TO_POINTER(imageid));
    g_assert(b);
    DrawElem *e = g_malloc(sizeof(DrawElem));
    e->type = DRAW_ELEM_IMAGE;
    e->data.image.x = x;
    e->data.image.y = y;
    e->data.image.scale_x = b->width / width;
    e->data.image.scale_y = b->height / height;
    e->data.image.bitmap = b->bitmap;
    window_add_draw(id, e);
}

// font

typedef struct {
    int height;
    PangoFontDescription *desc;
    PangoLayout *layout;
} FontElem;

static PangoContext *top_pango_context = NULL;

static FontElem *font_elem_new(int height, char *family, int style, int variant, int weight, int stretch) {
    if (top_pango_context == NULL) {
        top_pango_context = gtk_widget_get_pango_context(top);
    }
    FontElem *e = g_malloc(sizeof(FontElem));
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
    return e;
}

static void font_elem_free(FontElem *e) {
    g_object_unref(e->layout);
    pango_font_description_free(e->desc);
    g_free(e);
}

static GHashTable *font_elem_table = NULL;

void font_elem_add(int id, int height, char *family, int style, int variant, int weight, int stretch) {
    FontElem *e = font_elem_new(height, family, style, variant, weight, stretch);
    if (font_elem_table == NULL) {
        font_elem_table = g_hash_table_new(g_direct_hash, g_direct_equal);
    }
    g_hash_table_insert(font_elem_table, GINT_TO_POINTER(id), e);
}

void font_elem_rem(int id) {
    FontElem *e = g_hash_table_lookup(font_elem_table, GINT_TO_POINTER(id));
    g_assert(e);
    g_hash_table_remove(font_elem_table, GINT_TO_POINTER(id));
    font_elem_free(e);
}

static GHashTable *font_metric_table = NULL;

void font_metric_add(int id, int height, char *family, int style, int variant, int weight, int stretch) {
    FontElem *e = font_elem_new(height, family, style, variant, weight, stretch);
    if (font_metric_table == NULL) {
        font_metric_table = g_hash_table_new(g_direct_hash, g_direct_equal);
    }
    g_hash_table_insert(font_metric_table, GINT_TO_POINTER(id), e);
}

void font_metric_rem(int id) {
    FontElem *e = g_hash_table_lookup(font_metric_table, GINT_TO_POINTER(id));
    g_assert(e);
    g_hash_table_remove(font_metric_table, GINT_TO_POINTER(id));
    font_elem_free(e);
}

void get_font_metrics(int fontid, int16_t *lineheight, int16_t *baseline, int16_t *ascent, int16_t *descent) {
    FontElem *f = g_hash_table_lookup(font_metric_table, GINT_TO_POINTER(fontid));
    g_assert(f);
    *baseline = (int16_t)(pango_layout_get_baseline(f->layout) / PANGO_SCALE);
    PangoFontMetrics *metrics = pango_context_get_metrics(pango_layout_get_context(f->layout), f->desc, NULL);
    *lineheight = (int16_t)(pango_font_metrics_get_height(metrics) / PANGO_SCALE);
    *ascent = (int16_t)(pango_font_metrics_get_ascent(metrics) / PANGO_SCALE);
    *descent = (int16_t)(pango_font_metrics_get_descent(metrics) / PANGO_SCALE);
    pango_font_metrics_unref(metrics);
}

// text split

int16_t *font_metric_split_text(int fontid, char *text, int edge, int indent) {
    FontElem *f = g_hash_table_lookup(font_metric_table, GINT_TO_POINTER(fontid));
    g_assert(f);
    pango_layout_set_wrap(f->layout, PANGO_WRAP_WORD_CHAR);
    pango_layout_set_width(f->layout, PANGO_SCALE * edge);
    pango_layout_set_indent(f->layout, PANGO_SCALE * indent);
    pango_layout_set_text(f->layout, text, -1);
    GSList *top = pango_layout_get_lines_readonly(f->layout);
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
#ifdef PANGO_AVAILABLE_ENUMERATOR_IN_1_56
    pango_layout_set_wrap(f->layout, PANGO_WRAP_NONE);
#endif
    pango_layout_set_width(f->layout, -1);
    pango_layout_set_indent(f->layout, 0);
    return out;
}

// text rect

void font_metric_rect_text(int fontid, char *text, int16_t *width, int16_t *height) {
    FontElem *f = g_hash_table_lookup(font_metric_table, GINT_TO_POINTER(fontid));
    g_assert(f);
    pango_layout_set_text(f->layout, text, -1);
    int w, h;
    pango_layout_get_pixel_size(f->layout, &w, &h);
    *width = (int16_t)w;
    *height = (int16_t)h;
}

// text drawing

static inline void elem_text_draw(cairo_t *cr, DrawElem *e) {
    pango_layout_set_text(e->data.text.layout, e->data.text.text, -1);
    cairo_set_source_rgba(cr, e->data.text.r, e->data.text.g, e->data.text.b, e->data.text.a);
    cairo_move_to(cr, e->data.text.x, e->data.text.y);
    pango_cairo_show_layout(cr, e->data.text.layout);
}

static inline void elem_text_destroy(DrawElem *e) { g_free(e->data.text.text); }

void elem_text_add(int id, double x, double y, char *text, int fontid, double r, double g, double b, double a) {
    FontElem *f = g_hash_table_lookup(font_elem_table, GINT_TO_POINTER(fontid));
    g_assert(f);
    DrawElem *e = g_malloc(sizeof(DrawElem));
    e->type = DRAW_ELEM_TEXT;
    e->data.text.x = x;
    e->data.text.y = y;
    e->data.text.text = text;
    e->data.text.layout = f->layout;
    e->data.text.r = r;
    e->data.text.g = g;
    e->data.text.b = b;
    e->data.text.a = a;
    window_add_draw(id, e);
}

// any elem

void draw_any_elem(gpointer data, gpointer cr) {
    DrawElem *e = data;
    switch (e->type) {
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

void elem_draw_destroy(gpointer data) {
    DrawElem *e = data;
    switch (e->type) {
    case DRAW_ELEM_TEXT:
        elem_text_destroy(e);
        break;
    }
    g_free(e);
}
