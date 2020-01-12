#ifndef _terminal_h_
#define _terminal_h_

#include "idlist.h"
#include <gtk/gtk.h>
#include <stdint.h>

// main

extern char *it_version;

extern GtkWidget *app;
extern GtkWidget *layout;

// network

void network_init();
void network_done();
void socket_input_write(void *data, int length);
void socket_output_write(void *data, int length);

// io

void readbuffcall(void *buffer, int size, void (*f)());
void readalloccall(void *buffer, int size, void (*f)(void *));
gboolean readchan(GIOChannel *source, GIOCondition condition, gpointer data);

// event

gboolean s_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data);
gboolean s_button(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean s_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data);
void on_destroy(GtkWidget *widget G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED);

// call

void callcommand(char command);

// window

void window_create(int id);
void window_destroy(int id);
void *window_get_data(int id);

void window_size(int id, int x, int y, int width, int height);
void window_redraw(int id);

// font

void font_elem_add(int id, int height, char *family, int style, int variant, int weight,
                   int stretch);
void get_font_metrics(int fontid, int16_t *baseline, int16_t *ascent, int16_t *descent);
int16_t *font_split_text(int fontid, char *text, int edge);
void font_rect_text(int fontid, char *text, int16_t *width, int16_t *height);

// draw

void *draw_data_new();
void draw_data_free(void *v);
void draw_destroy(void *v);

void elem_clear(int id);
void elem_fill_add(int id, int x, int y, int width, int height, int r, int g, int b);
void elem_line_add(int id, int x0, int y0, int x1, int y1, int r, int g, int b);
void elem_text_add(int id, int x, int y, char *text, int fontid, int r, int g, int b);

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data);

#endif
