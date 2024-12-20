#include "idlist.h"
#include "terminal.h"
#include <gtk/gtk.h>
#include <math.h>

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

void elem_fill_draw(cairo_t *cr, fill_elem *e) {
    cairo_set_source_rgba(cr, e->r, e->g, e->b, e->a);
    cairo_rectangle(cr, e->x, e->y, e->width, e->height);
    cairo_fill(cr);
}

void elem_fill_add(int id, int x, int y, int width, int height, int r, int g, int b, int a) {
    fill_elem *e = g_malloc(sizeof(fill_elem));
    e->type = DRAW_ELEM_FILL;
    e->x = x;
    e->y = y;
    e->width = width;
    e->height = height;
    e->r = (double)r / (double)0xFFFF;
    e->g = (double)g / (double)0xFFFF;
    e->b = (double)b / (double)0xFFFF;
    e->a = (double)a / (double)0xFFFF;
    window_add_draw(id, e);
}

// line

typedef struct {
    int type;
    double x0, y0, x1, y1;
    double r, g, b, a;
} line_elem;

void elem_line_draw(cairo_t *cr, line_elem *e) {
    cairo_set_source_rgba(cr, e->r, e->g, e->b, e->a);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, e->x0, e->y0);
    cairo_line_to(cr, e->x1, e->y1);
    cairo_stroke(cr);
}

void elem_line_add(int id, int x0, int y0, int x1, int y1, int r, int g, int b, int a) {
    line_elem *e = g_malloc(sizeof(line_elem));
    e->type = DRAW_ELEM_LINE;
    e->x0 = x0;
    e->y0 = y0;
    e->x1 = x1;
    e->y1 = y1;
    if (x0 == x1) {
        e->x0 += 0.5;
        e->x1 += 0.5;
    } else if (y0 == y1) {
        e->y0 += 0.5;
        e->y1 += 0.5;
    }
    e->r = (double)r / (double)0xFFFF;
    e->g = (double)g / (double)0xFFFF;
    e->b = (double)b / (double)0xFFFF;
    e->a = (double)a / (double)0xFFFF;
    window_add_draw(id, e);
}

// bitmap cache

typedef struct {
    unsigned char *data;
    double width, height;
    cairo_surface_t *bitmap;
} bitmap_elem;

static id_list *bitmap_list = NULL;

void bitmap_add(int id, int width, int height, unsigned char *data) {
    int cairo_format = CAIRO_FORMAT_ARGB32;
    int stride = cairo_format_stride_for_width(cairo_format, width);
    unsigned char *row = data;
    for (int i = 0; i < height; i++) {
        unsigned char *p0 = row;
        unsigned char *p2 = p0 + 2;
        for (int j = 0; j < width; j++) {
            unsigned char r = *p0;
            *p0 = *p2;
            *p2 = r;
            p0 += 4;
            p2 += 4;
        }
        row += stride;
    }
    bitmap_elem *e = g_malloc(sizeof(bitmap_elem));
    e->data = data;
    e->width = width;
    e->height = height;
    e->bitmap = cairo_image_surface_create_for_data(e->data, cairo_format, width, height, stride);
    if (bitmap_list == NULL)
        bitmap_list = id_list_new();
    id_list_append(bitmap_list, id, e);
}

void bitmap_rem(int id) {
    bitmap_elem *e = id_list_remove(bitmap_list, id);
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

void elem_image_draw(cairo_t *cr, image_elem *e) {
    cairo_surface_set_device_scale(e->bitmap, e->scale_x, e->scale_y);
    cairo_set_source_surface(cr, e->bitmap, e->x, e->y);
    cairo_paint(cr);
}

void elem_image_add(int id, int x, int y, int width, int height, int imageid) {
    image_elem *e = g_malloc(sizeof(image_elem));
    bitmap_elem *b = id_list_get_data(bitmap_list, imageid);
    e->type = DRAW_ELEM_IMAGE;
    e->x = x;
    e->y = y;
    e->scale_x = b->width / (double)width;
    e->scale_y = b->height / (double)height;
    e->bitmap = b->bitmap;
    window_add_draw(id, e);
}

// font

typedef struct {
    int height;
    PangoFontDescription *desc;
} font_elem;

static id_list *font_list = NULL;

void font_elem_add(int id, int height, char *family, int style, int variant, int weight, int stretch) {
    font_elem *e = g_malloc(sizeof(font_elem));
    e->height = height;
    e->desc = pango_font_description_new();
    pango_font_description_set_family(e->desc, family);
    pango_font_description_set_absolute_size(e->desc, PANGO_SCALE * height);
    pango_font_description_set_style(e->desc, style);
    pango_font_description_set_variant(e->desc, variant);
    pango_font_description_set_weight(e->desc, weight);
    pango_font_description_set_stretch(e->desc, stretch);
    if (font_list == NULL)
        font_list = id_list_new();
    id_list_append(font_list, id, e);
}

void font_elem_rem(int id) {
    font_elem *e = id_list_remove(font_list, id);
    pango_font_description_free(e->desc);
    g_free(e);
}

static PangoContext *top_pango_context = NULL;

void get_font_metrics(int fontid, int16_t *lineheight, int16_t *baseline, int16_t *ascent, int16_t *descent) {
    if (top_pango_context == NULL)
        top_pango_context = gtk_widget_get_pango_context(top);
    font_elem *f = id_list_get_data(font_list, fontid);
    PangoLayout *layout = pango_layout_new(top_pango_context);
    pango_layout_set_font_description(layout, f->desc);
    *baseline = (int16_t)rint((double)pango_layout_get_baseline(layout) / PANGO_SCALE);
    PangoFontMetrics *metrics = pango_context_get_metrics(pango_layout_get_context(layout), f->desc, NULL);
    *lineheight = (int16_t)rint((double)pango_font_metrics_get_height(metrics) / PANGO_SCALE);
    *ascent = (int16_t)rint((double)pango_font_metrics_get_ascent(metrics) / PANGO_SCALE);
    *descent = (int16_t)rint((double)pango_font_metrics_get_descent(metrics) / PANGO_SCALE);
    pango_font_metrics_unref(metrics);
    g_object_unref(layout);
}

// text split

int16_t *font_split_text(int fontid, char *text, int edge, int indent) {
    if (top_pango_context == NULL)
        top_pango_context = gtk_widget_get_pango_context(top);
    font_elem *f = id_list_get_data(font_list, fontid);
    PangoLayout *layout = pango_layout_new(top_pango_context);
    pango_layout_set_font_description(layout, f->desc);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
    pango_layout_set_width(layout, PANGO_SCALE * edge);
    pango_layout_set_indent(layout, PANGO_SCALE * indent);
    pango_layout_set_text(layout, text, -1);
    int length = pango_layout_get_line_count(layout);
    int16_t *out = g_malloc(sizeof(int16_t) * (length + 1));
    int16_t *pos = out;
    *pos++ = length;
    for (GSList *e = pango_layout_get_lines_readonly(layout); e != NULL; e = e->next) {
        PangoLayoutLine *line = e->data;
        *pos++ = (int16_t)(line->length);
    }
    g_object_unref(layout);
    return out;
}

// text rect

void font_rect_text(int fontid, char *text, int16_t *width, int16_t *height) {
    if (top_pango_context == NULL)
        top_pango_context = gtk_widget_get_pango_context(top);
    font_elem *f = id_list_get_data(font_list, fontid);
    PangoLayout *layout = pango_layout_new(top_pango_context);
    pango_layout_set_font_description(layout, f->desc);
    pango_layout_set_text(layout, text, -1);
    int w, h;
    pango_layout_get_pixel_size(layout, &w, &h);
    *width = (int16_t)w;
    *height = (int16_t)h;
    g_object_unref(layout);
}

// text

typedef struct {
    int type;
    double x, y;
    char *text;
    PangoFontDescription *desc;
    double r, g, b, a;
    PangoLayout *layout;
} text_elem;

void elem_text_draw(cairo_t *cr, text_elem *e) {
    if (e->layout == NULL) {
        e->layout = pango_cairo_create_layout(cr);
        pango_layout_set_font_description(e->layout, e->desc);
        pango_layout_set_text(e->layout, e->text, -1);
    }
    cairo_set_source_rgba(cr, e->r, e->g, e->b, e->a);
    cairo_move_to(cr, e->x, e->y);
    pango_cairo_show_layout(cr, e->layout);
}

void elem_text_destroy(text_elem *e) {
    if (e->layout != NULL)
        g_object_unref(e->layout);
    g_free(e->text);
}

void elem_text_add(int id, int x, int y, char *text, int fontid, int r, int g, int b, int a) {
    font_elem *f = id_list_get_data(font_list, fontid);
    text_elem *e = g_malloc(sizeof(text_elem));
    e->type = DRAW_ELEM_TEXT;
    e->x = x;
    e->y = y;
    e->text = text;
    e->desc = f->desc;
    e->r = (double)r / (double)0xFFFF;
    e->g = (double)g / (double)0xFFFF;
    e->b = (double)b / (double)0xFFFF;
    e->a = (double)a / (double)0xFFFF;
    e->layout = NULL;
    window_add_draw(id, e);
}

// callback

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, void *data) {
    cairo_save(cr);
    for (id_list_elem *e = id_list_root(data); e != NULL; e = id_list_elem_next(e)) {
        type_elem *t = id_list_elem_data(e);
        switch (t->type) {
        case DRAW_ELEM_FILL:
            elem_fill_draw(cr, id_list_elem_data(e));
            break;
        case DRAW_ELEM_LINE:
            elem_line_draw(cr, id_list_elem_data(e));
            break;
        case DRAW_ELEM_TEXT:
            elem_text_draw(cr, id_list_elem_data(e));
            break;
        case DRAW_ELEM_IMAGE:
            elem_image_draw(cr, id_list_elem_data(e));
            break;
        }
    }
    cairo_restore(cr);
    return FALSE;
}

void elem_draw_destroy(void *data) {
    type_elem *t = data;
    switch (t->type) {
    case DRAW_ELEM_TEXT:
        elem_text_destroy(data);
        break;
    }
    g_free(data);
}
