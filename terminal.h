#ifndef _terminal_h_
#define _terminal_h_

#include "idlist.h"
#include <gtk/gtk.h>
#include <stdint.h>

// main

extern char *it_version;

extern GtkApplication *app;
extern GtkWidget *top;

extern GMenu *appmenu;
extern GMenu *barmenu;

// network

void pipe_init();
void pipe_done();
void pipe_output_write(void *data, int length);
void pipe_output_flush();
void pipe_event_write(void *data, int length);
void pipe_event_flush();

// io

typedef struct pipe_buffer pipe_buffer;
pipe_buffer *io_input();
pipe_buffer *io_stream();
void parameters_to_call(pipe_buffer *target, void *buffer, int size, void (*f)());
void parameters_alloc_to_call(pipe_buffer *target, void *buffer, int size, void (*f)(void *));
gboolean async_read_chan(GIOChannel *source, GIOCondition condition, gpointer data);

// event

gboolean on_delete(GtkWidget *widget G_GNUC_UNUSED, gpointer data G_GNUC_UNUSED);
gboolean on_configure(GtkWindow *window, GdkEventConfigure *event, gpointer data G_GNUC_UNUSED);
gboolean s_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data);
gboolean s_button(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean s_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data);
gboolean s_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer data);
void s_menu_action(char *action);

// call

void callcommand(char command, pipe_buffer *target);

// layout

void layout_create(int id, int parent_id);
void layout_destroy(int id);
void layout_raise(int id);
void layout_size(int id, int x, int y, int width, int height);

GtkWidget *layout_get_widget(int id);

// window

void window_create(int id, int layout_id);
void window_destroy(int id);
void window_raise(int id);
void window_size(int id, int x, int y, int width, int height);
void window_redraw(int id);

void *window_get_data(int id);

// image

void image_add(int id, int width, int height, unsigned char *data);
void image_rem(int id);

// font

void font_elem_add(int id, int height, char *family, int style, int variant, int weight,
                   int stretch);
void get_font_metrics(int fontid, int16_t *baseline, int16_t *ascent, int16_t *descent);
int16_t *font_split_text(int fontid, char *text, int edge, int indent);
void font_rect_text(int fontid, char *text, int16_t *width, int16_t *height);

// draw

void *draw_data_new();
void draw_data_free(void *v);
void draw_destroy(void *v);

void elem_clear(int id);
void elem_fill_add(int id, int x, int y, int width, int height, int r, int g, int b);
void elem_line_add(int id, int x0, int y0, int x1, int y1, int r, int g, int b);
void elem_image_add(int id, int x, int y, int width, int height, int imageid);
void elem_text_add(int id, int x, int y, char *text, int fontid, int r, int g, int b);

// menu

void menu_node_add(int id, int parent, char *label);
void menu_item_add(int id, int parent, char *label, char *action);

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data);

#endif
