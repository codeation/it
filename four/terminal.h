#ifndef _terminal_h_
#define _terminal_h_

#include <gtk/gtk.h>

// main

extern char *it_api_version;

extern GtkApplication *app;
extern GtkWidget *top;

// network

void pipe_init(char *pipe_suffix);
void pipe_done();
void pipe_output_write(const void *data, const int length);
void pipe_output_write_string(const char *data);
void pipe_output_flush();
void pipe_event_write(const void *data, const int length);
void pipe_event_write_string(const char *data);
void pipe_event_flush();

// io

typedef struct _pipe_buffer pipe_buffer;
void parameters_to_call(pipe_buffer *target, void *buffer, int size, void (*f)());
void parameters_alloc_to_call(pipe_buffer *target, void *buffer, int size, void (*f)(void *));
void io_input_start(FILE *source);
void io_stream_start(FILE *source);
void io_stop(pipe_buffer *target);
gboolean io_exited();

// event

void top_signal_connect();
void top_signal_disconnect();
void layout_signal_connect(GtkWidget *scrolled, GtkAdjustment *adjustment);
void layout_signal_disconnect(GtkWidget *scrolled, GtkAdjustment *adjustment);

void s_menu_action(char *action);
void request_clipboard(int clipboardtypeid);
void set_clipboard(int clipboardtypeid, void *data);

// call

void callcommand(char command, pipe_buffer *target);

// layout

void layout_create(int id, int parent_id);
void layout_destroy(int id);
void layout_raise(int id);
void layout_size(int id, int x, int y, int width, int height);
void layout_main_grab_focus();

GtkWidget *layout_get_widget(int id);

// window

void window_create(int id, int layout_id);
void window_destroy(int id);
void window_raise(int id);
void window_size(int id, int x, int y, int width, int height);
void window_redraw(int id);
void window_clear(int id);

void window_add_draw(int id, gpointer e);

// bitmap

void bitmap_add(int id, int width, int height, unsigned char *data);
void bitmap_rem(int id);

// font

void font_elem_add(int id, int height, char *family, int style, int variant, int weight, int stretch);
void font_elem_rem(int id);
void get_font_metrics(int fontid, int16_t *lineheight, int16_t *baseline, int16_t *ascent, int16_t *descent);
int16_t *font_split_text(int fontid, char *text, int edge, int indent);
void font_rect_text(int fontid, char *text, int16_t *width, int16_t *height);

// draw

void elem_fill_add(int id, int x, int y, int width, int height, int r, int g, int b, int a);
void elem_line_add(int id, int x0, int y0, int x1, int y1, int r, int g, int b, int a);
void elem_image_add(int id, int x, int y, int width, int height, int imageid);
void elem_text_add(int id, int x, int y, char *text, int fontid, int r, int g, int b, int a);

void draw_any_elem(gpointer e, gpointer cr);
void elem_draw_destroy(gpointer e);

// menu

void menubar_create();
void menu_node_add(int id, int parent, char *label);
void menu_item_add(int id, int parent, char *label, char *action);

#endif
