#include "idlist.h"
#include "terminal.h"
#include <gtk/gtk.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DRAW_ELEM_FILL 1
#define DRAW_ELEM_LINE 2
#define DRAW_ELEM_TEXT 3
#define DRAW_ELEM_IMAGE 4

// draw elements

void *draw_data_new() { return id_list_new(); }

void draw_data_free(void *v) { id_list_free(v); }

void draw_elem_add(id_list *list, void *data) { id_list_append(list, 0, data); }

// general

typedef struct {
    int type;
} draw_type;

// fill

typedef struct {
    int type;
    int x, y, width, height;
    double r, g, b;
} elem_fill;

void elem_fill_draw(cairo_t *cr, elem_fill *e) {
    cairo_save(cr);
    cairo_set_source_rgb(cr, e->r, e->g, e->b);
    cairo_rectangle(cr, e->x, e->y, e->width, e->height);
    cairo_fill(cr);
    cairo_restore(cr);
}

void elem_fill_destroy(elem_fill *e) {}

void elem_fill_add(int id, int x, int y, int width, int height, int r, int g, int b) {
    elem_fill *e = malloc(sizeof(elem_fill));
    e->type = DRAW_ELEM_FILL;
    e->x = x;
    e->y = y;
    e->width = width;
    e->height = height;
    e->r = (double)r / (double)0xFFFF;
    e->g = (double)g / (double)0xFFFF;
    e->b = (double)b / (double)0xFFFF;
    draw_elem_add(window_get_data(id), e);
}

// line

typedef struct {
    int type;
    int x0, y0, x1, y1;
    double r, g, b;
} elem_line;

void elem_line_draw(cairo_t *cr, elem_line *e) {
    cairo_save(cr);
    cairo_set_source_rgb(cr, e->r, e->g, e->b);
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, e->x0 + 0.5, e->y0 + 0.5);
    cairo_line_to(cr, e->x1 + 0.5, e->y1 + 0.5);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void elem_line_destroy(elem_line *e) {}

void elem_line_add(int id, int x0, int y0, int x1, int y1, int r, int g, int b) {
    elem_line *e = malloc(sizeof(elem_line));
    e->type = DRAW_ELEM_LINE;
    e->x0 = x0;
    e->y0 = y0;
    e->x1 = x1;
    e->y1 = y1;
    e->r = (double)r / (double)0xFFFF;
    e->g = (double)g / (double)0xFFFF;
    e->b = (double)b / (double)0xFFFF;
    draw_elem_add(window_get_data(id), e);
}

// image cache

typedef struct _image_elem image_elem;

struct _image_elem {
    unsigned char *data;
    int width, height;
    cairo_surface_t *image;
};

static id_list *image_list = NULL;

image_elem *get_image(int id) { return (image_elem *)id_list_get_data(image_list, id); }

void image_add(int id, int width, int height, unsigned char *data) {
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, width);
    for (int i = 0; i < height; i++) {
        unsigned char *p = data + i * stride;
        for (int j = 0; j < width; j++) {
            unsigned char r = p[0];
            p[0] = p[2];
            p[2] = r;
            p += 4;
        }
    }
    image_elem *e = malloc(sizeof(image_elem));
    e->data = data;
    e->width = width;
    e->height = height;
    e->image =
        cairo_image_surface_create_for_data(e->data, CAIRO_FORMAT_ARGB32, width, height, stride);
    if (image_list == NULL)
        image_list = id_list_new();
    id_list_append(image_list, id, e);
}

void image_rem(int id) {
    image_elem *e = id_list_remove(image_list, id);
    cairo_surface_destroy(e->image);
    free(e->data);
    free(e);
}

// image drawing

typedef struct {
    int type;
    int x, y;
    int width, height;
    int imageid;
} elem_image;

void elem_image_draw(cairo_t *cr, elem_image *e) {
    cairo_save(cr);
    image_elem *ie = get_image(e->imageid);
    double scale_x = (double)(ie->width) / (double)(e->width);
    double scale_y = (double)(ie->height) / (double)(e->height);
    cairo_surface_set_device_scale(ie->image, scale_x, scale_y);
    cairo_set_source_surface(cr, ie->image, e->x, e->y);
    cairo_paint(cr);
    cairo_restore(cr);
}

void elem_image_destroy(elem_image *e) {}

void elem_image_add(int id, int x, int y, int width, int height, int imageid) {
    elem_image *e = malloc(sizeof(elem_image));
    e->type = DRAW_ELEM_IMAGE;
    e->x = x;
    e->y = y;
    e->width = width;
    e->height = height;
    e->imageid = imageid;
    draw_elem_add(window_get_data(id), e);
}

// font

typedef struct _font_elem font_elem;

struct _font_elem {
    int height;
    PangoFontDescription *desc;
};

static id_list *font_list = NULL;

font_elem *get_font(int id) { return (font_elem *)id_list_get_data(font_list, id); }

void font_elem_add(int id, int height, char *family, int style, int variant, int weight,
                   int stretch) {
    font_elem *e = malloc(sizeof(font_elem));
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
    free(family);
}

void font_elem_destroy() {
    while (TRUE) {
        font_elem *e = id_list_remove_any(font_list);
        if (e == NULL)
            break;
        pango_font_description_free(e->desc);
        free(e);
    }
}

void get_font_metrics(int fontid, int16_t *baseline, int16_t *ascent, int16_t *descent) {
    PangoLayout *layout = pango_layout_new(gtk_widget_get_pango_context(top));
    pango_layout_set_font_description(layout, get_font(fontid)->desc);
    *baseline = (int16_t)rintl((double)pango_layout_get_baseline(layout) / PANGO_SCALE);
    PangoFontMetrics *metrics =
        pango_context_get_metrics(pango_layout_get_context(layout), get_font(fontid)->desc, NULL);
    *ascent = (int16_t)rintl((double)pango_font_metrics_get_ascent(metrics) / PANGO_SCALE);
    *descent = (int16_t)rintl((double)pango_font_metrics_get_descent(metrics) / PANGO_SCALE);
    pango_font_metrics_unref(metrics);
    g_object_unref(layout);
}

// text split

int16_t *font_split_text(int fontid, char *text, int edge) {
    PangoLayout *layout = pango_layout_new(gtk_widget_get_pango_context(top));
    pango_layout_set_font_description(layout, get_font(fontid)->desc);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
    pango_layout_set_width(layout, PANGO_SCALE * edge);
    pango_layout_set_text(layout, text, -1);
    int length = pango_layout_get_line_count(layout);
    int16_t *out = malloc(sizeof(int16_t) * (length + 1));
    int16_t *pos = out;
    *pos++ = length;
    for (GSList *e = pango_layout_get_lines_readonly(layout); e != NULL; e = e->next) {
        PangoLayoutLine *line = e->data;
        *pos++ = (int16_t)(line->length);
    }
    g_object_unref(layout);
    free(text);
    return out;
}

// text rect

void font_rect_text(int fontid, char *text, int16_t *width, int16_t *height) {
    PangoLayout *layout = pango_layout_new(gtk_widget_get_pango_context(top));
    pango_layout_set_font_description(layout, get_font(fontid)->desc);
    pango_layout_set_text(layout, text, -1);
    int w, h;
    pango_layout_get_pixel_size(layout, &w, &h);
    *width = (int16_t)w;
    *height = (int16_t)h;
    g_object_unref(layout);
    free(text);
}

// text

typedef struct {
    int type;
    int x, y;
    char *text;
    int fontid;
    double r, g, b;
    PangoLayout *layout;
} elem_text;

void elem_text_draw(cairo_t *cr, elem_text *e) {
    if (e->layout == NULL) {
        e->layout = pango_cairo_create_layout(cr);
        pango_layout_set_font_description(e->layout, get_font(e->fontid)->desc);
        pango_layout_set_text(e->layout, e->text, -1);
    }
    cairo_save(cr);
    cairo_set_source_rgb(cr, e->r, e->g, e->b);
    cairo_move_to(cr, e->x, e->y);
    pango_cairo_show_layout(cr, e->layout);
    cairo_restore(cr);
}

void elem_text_destroy(elem_text *e) {
    if (e->layout != NULL)
        g_object_unref(e->layout);
    free(e->text);
}

void elem_text_add(int id, int x, int y, char *text, int fontid, int r, int g, int b) {
    elem_text *e = malloc(sizeof(elem_text));
    e->type = DRAW_ELEM_TEXT;
    e->x = x;
    e->y = y;
    e->text = text;
    e->fontid = fontid;
    e->r = (double)r / (double)0xFFFF;
    e->g = (double)g / (double)0xFFFF;
    e->b = (double)b / (double)0xFFFF;
    e->layout = NULL;
    draw_elem_add(window_get_data(id), e);
}

// common func

void elem_clear(int id) { draw_destroy(window_get_data(id)); }

// callback

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    for (void *e = id_list_root(data); e != NULL; e = id_list_elem_next(e)) {
        draw_type *t = id_list_elem_data(e);
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
    return FALSE;
}

void draw_destroy(void *data) {
    id_list *list = data;
    while (TRUE) {
        void *e = id_list_remove_any(list);
        if (e == NULL)
            break;
        draw_type *t = e;
        switch (t->type) {
        case DRAW_ELEM_FILL:
            elem_fill_destroy(e);
            break;
        case DRAW_ELEM_LINE:
            elem_line_destroy(e);
            break;
        case DRAW_ELEM_TEXT:
            elem_text_destroy(e);
            break;
        case DRAW_ELEM_IMAGE:
            elem_image_destroy(e);
            break;
        }
        free(e);
    }
}
