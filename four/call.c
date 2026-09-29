#include "terminal.h"
#include <gtk/gtk.h>

static const double color_scale = 1.0 / 0xFFFFu;

static inline double dc(double c) { return c * color_scale; }

// driver version

static void cmd_a_version() {
    pipe_output_write_string(it_api_version);
    pipe_output_flush();
}

// exit

static void cmd_a_exit(PipeBuffer *target) {
    if (!io_is_sync(target)) {
        top_signal_disconnect();
    }
    io_stop(target);
    if (io_exited()) {
        gtk_window_destroy(GTK_WINDOW(top));
        g_application_quit(G_APPLICATION(app));
    }
}

// clear command

static struct {
    int16_t id;
} w_clear_p;

_Static_assert(sizeof w_clear_p == 2, "wrong w_clear_p align");

static void w_clear() { window_clear(w_clear_p.id); }

static void cmd_w_clear(PipeBuffer *target) { io_buffer_call(target, &w_clear_p, sizeof w_clear_p, w_clear); }

// show command

static struct {
    int16_t id;
} w_show_p;

_Static_assert(sizeof w_show_p == 2, "wrong w_show_p align");

static void w_show() { window_redraw(w_show_p.id); }

static void cmd_w_show(PipeBuffer *target) { io_buffer_call(target, &w_show_p, sizeof w_show_p, w_show); }

// fill command

static struct {
    int16_t id;
    int16_t x, y, width, height;
    uint16_t r, g, b, a;
} w_fill_p;

_Static_assert(sizeof w_fill_p == 18, "wrong w_fill_p align");

static void w_fill() {
    elem_fill_add(w_fill_p.id, w_fill_p.x, w_fill_p.y, w_fill_p.width, w_fill_p.height, dc(w_fill_p.r), dc(w_fill_p.g),
                  dc(w_fill_p.b), dc(w_fill_p.a));
}

static void cmd_w_fill(PipeBuffer *target) { io_buffer_call(target, &w_fill_p, sizeof w_fill_p, w_fill); }

// draw line

static struct {
    int16_t id;
    int16_t x0, y0;
    int16_t x1, y1;
    uint16_t r, g, b, a;
} w_line_p;

_Static_assert(sizeof w_line_p == 18, "wrong w_line_p align");

static void w_line() {
    elem_line_add(w_line_p.id, w_line_p.x0, w_line_p.y0, w_line_p.x1, w_line_p.y1, dc(w_line_p.r), dc(w_line_p.g),
                  dc(w_line_p.b), dc(w_line_p.a));
}

static void cmd_w_line(PipeBuffer *target) { io_buffer_call(target, &w_line_p, sizeof w_line_p, w_line); }

// draw string

static struct {
    int16_t id;
    int16_t x, y;
    uint16_t r, g, b, a;
    int16_t fontid;
} w_text_p;

_Static_assert(sizeof w_text_p == 16, "wrong w_text_p align");

static void w_text(void *text) {
    elem_text_add(w_text_p.id, w_text_p.x, w_text_p.y, text, w_text_p.fontid, dc(w_text_p.r), dc(w_text_p.g),
                  dc(w_text_p.b), dc(w_text_p.a));
    // g_free(text) in elem_text_destroy
}

void cmd_w_text(PipeBuffer *target) { io_buffer_malloc_call(target, &w_text_p, sizeof w_text_p, w_text); }

// draw image

static struct {
    int16_t id;
    int16_t x, y;
    int16_t width, height;
    int16_t imageid;
} w_image_p;

_Static_assert(sizeof w_image_p == 12, "wrong w_image_p align");

static void w_image() {
    elem_image_add(w_image_p.id, w_image_p.x, w_image_p.y, w_image_p.width, w_image_p.height, w_image_p.imageid);
}

static void cmd_w_image(PipeBuffer *target) { io_buffer_call(target, &w_image_p, sizeof w_image_p, w_image); }

// load image

static struct {
    int16_t id;
    int16_t width, height;
} image_new_p;

_Static_assert(sizeof image_new_p == 6, "wrong image_new_p align");

static void image_new(void *data) {
    bitmap_add(image_new_p.id, image_new_p.width, image_new_p.height, data);
    // g_free(data) in image_rem
}

static void cmd_image_new(PipeBuffer *target) {
    io_buffer_malloc_call(target, &image_new_p, sizeof image_new_p, image_new);
}

// remove image

static struct {
    int16_t id;
} image_drop_p;

_Static_assert(sizeof image_drop_p == 2, "wrong image_drop_p align");

static void image_drop() { bitmap_rem(image_drop_p.id); }

static void cmd_image_drop(PipeBuffer *target) {
    io_buffer_call(target, &image_drop_p, sizeof image_drop_p, image_drop);
}

// load font

static struct {
    int16_t id;
    int16_t height;
    int16_t style, variant, weight, stretch;
} font_new_p;

_Static_assert(sizeof font_new_p == 12, "wrong font_new_p align");

static void font_new(void *family) {
    font_elem_add(font_new_p.id, font_new_p.height, family, font_new_p.style, font_new_p.variant, font_new_p.weight,
                  font_new_p.stretch);
    g_free(family);
}

static void font_m_new(void *family) {
    font_metric_add(font_new_p.id, font_new_p.height, family, font_new_p.style, font_new_p.variant, font_new_p.weight,
                    font_new_p.stretch);
    int16_t lineheight, baseline, ascent, descent;
    get_font_metrics(font_new_p.id, &lineheight, &baseline, &ascent, &descent);
    pipe_output_write(&lineheight, sizeof lineheight);
    pipe_output_write(&baseline, sizeof baseline);
    pipe_output_write(&ascent, sizeof ascent);
    pipe_output_write(&descent, sizeof descent);
    pipe_output_flush();
    g_free(family);
}

static void cmd_font_new(PipeBuffer *target) {
    if (io_is_sync(target)) {
        io_buffer_malloc_call(target, &font_new_p, sizeof font_new_p, font_m_new);
    } else {
        io_buffer_malloc_call(target, &font_new_p, sizeof font_new_p, font_new);
    }
}

// remove font

static struct {
    int16_t id;
} font_drop_p;

_Static_assert(sizeof font_drop_p == 2, "wrong font_drop_p align");

static void font_drop() { font_elem_rem(font_drop_p.id); }

static void font_m_drop() { font_metric_rem(font_drop_p.id); }

static void cmd_font_drop(PipeBuffer *target) {
    if (io_is_sync(target)) {
        io_buffer_call(target, &font_drop_p, sizeof font_drop_p, font_m_drop);
    } else {
        io_buffer_call(target, &font_drop_p, sizeof font_drop_p, font_drop);
    }
}

// split text

static struct {
    int16_t fontid;
    int16_t edge;
    int16_t indent;
} font_split_p;

_Static_assert(sizeof font_split_p == 6, "wrong font_split_p align");

static void font_split(void *text) {
    int16_t *out = font_metric_split_text(font_split_p.fontid, text, font_split_p.edge, font_split_p.indent);
    if (out == NULL) {
        int16_t value = 0;
        pipe_output_write(&value, sizeof value);
        pipe_output_flush();
    } else {
        int length = (1 + *out) * (int)sizeof(int16_t);
        pipe_output_write(out, length);
        pipe_output_flush();
        g_free(out);
    }
    g_free(text);
}

static void cmd_font_split(PipeBuffer *target) {
    io_buffer_malloc_call(target, &font_split_p, sizeof font_split_p, font_split);
}

// text rect

static struct {
    int16_t fontid;
} font_size_p;

_Static_assert(sizeof font_size_p == 2, "wrong font_size_p align");

static void font_size(void *text) {
    int16_t width, height;
    font_metric_rect_text(font_size_p.fontid, text, &width, &height);
    pipe_output_write(&width, sizeof width);
    pipe_output_write(&height, sizeof height);
    pipe_output_flush();
    g_free(text);
}

static void cmd_font_size(PipeBuffer *target) {
    io_buffer_malloc_call(target, &font_size_p, sizeof font_size_p, font_size);
}

// app window size

static struct {
    int16_t x, y;
    int16_t width, height;
} a_size_p;

_Static_assert(sizeof a_size_p == 8, "wrong a_size_p align");

static void a_size() { gtk_window_set_default_size(GTK_WINDOW(top), a_size_p.width, a_size_p.height); }

static void cmd_a_size(PipeBuffer *target) { io_buffer_call(target, &a_size_p, sizeof a_size_p, a_size); }

// app window title

static void a_title(void *title) {
    gtk_window_set_title(GTK_WINDOW(top), title);
    g_free(title);
}

static void cmd_a_title(PipeBuffer *target) { io_buffer_malloc_call(target, NULL, 0, a_title); }

// layout

static struct {
    int16_t id;
    int16_t parent_id;
    int16_t x, y;
    int16_t width, height;
} frame_new_p;

_Static_assert(sizeof frame_new_p == 12, "wrong frame_new_p align");

static void frame_new() {
    layout_create(frame_new_p.id, frame_new_p.parent_id);
    layout_size(frame_new_p.id, frame_new_p.x, frame_new_p.y, frame_new_p.width, frame_new_p.height);
}

static void cmd_frame_new(PipeBuffer *target) { io_buffer_call(target, &frame_new_p, sizeof frame_new_p, frame_new); }

// layout drop

static struct {
    int16_t id;
} frame_drop_p;

_Static_assert(sizeof frame_drop_p == 2, "wrong frame_drop_p align");

static void frame_drop() { layout_destroy(frame_drop_p.id); }

static void cmd_frame_drop(PipeBuffer *target) {
    io_buffer_call(target, &frame_drop_p, sizeof frame_drop_p, frame_drop);
}

// layout size

static struct {
    int16_t id;
    int16_t x, y;
    int16_t width, height;
} frame_size_p;

_Static_assert(sizeof frame_size_p == 10, "wrong frame_size_p align");

static void frame_size() {
    layout_size(frame_size_p.id, frame_size_p.x, frame_size_p.y, frame_size_p.width, frame_size_p.height);
}

static void cmd_frame_size(PipeBuffer *target) {
    io_buffer_call(target, &frame_size_p, sizeof frame_size_p, frame_size);
}

// layout raise

static struct {
    int16_t id;
} frame_raise_p;

_Static_assert(sizeof frame_raise_p == 2, "wrong frame_raise_p align");

static void frame_raise() { layout_raise(frame_raise_p.id); }

static void cmd_frame_raise(PipeBuffer *target) {
    io_buffer_call(target, &frame_raise_p, sizeof frame_raise_p, frame_raise);
}

// window

static struct {
    int16_t id;
    int16_t layout_id;
    int16_t x, y;
    int16_t width, height;
} w_new_p;

_Static_assert(sizeof w_new_p == 12, "wrong w_new_p align");

static void w_new() {
    window_create(w_new_p.id, w_new_p.layout_id);
    window_size(w_new_p.id, w_new_p.x, w_new_p.y, w_new_p.width, w_new_p.height);
}

static void cmd_w_new(PipeBuffer *target) { io_buffer_call(target, &w_new_p, sizeof w_new_p, w_new); }

// window size

static struct {
    int16_t id;
    int16_t x, y;
    int16_t width, height;
} w_size_p;

_Static_assert(sizeof w_size_p == 10, "wrong w_size_p align");

static void w_size() { window_size(w_size_p.id, w_size_p.x, w_size_p.y, w_size_p.width, w_size_p.height); }

static void cmd_w_size(PipeBuffer *target) { io_buffer_call(target, &w_size_p, sizeof w_size_p, w_size); }

// drop window command

static struct {
    int16_t id;
} w_drop_p;

_Static_assert(sizeof w_drop_p == 2, "wrong w_drop_p align");

static void w_drop() { window_destroy(w_drop_p.id); }

static void cmd_w_drop(PipeBuffer *target) { io_buffer_call(target, &w_drop_p, sizeof w_drop_p, w_drop); }

// window raise

static struct {
    int16_t id;
} w_raise_p;

_Static_assert(sizeof w_raise_p == 2, "wrong w_raise_p align");

static void w_raise() { window_raise(w_raise_p.id); }

static void cmd_w_raise(PipeBuffer *target) { io_buffer_call(target, &w_raise_p, sizeof w_raise_p, w_raise); }

// menu node

static struct {
    int16_t id;
    int16_t parent;
} menu_new_p;

_Static_assert(sizeof menu_new_p == 4, "wrong menu_new_p align");

static void menu_new(void *label) {
    menu_node_add(menu_new_p.id, menu_new_p.parent, label);
    // g_free(label) is eternal
}

static void cmd_menu_new(PipeBuffer *target) {
    io_buffer_malloc_call(target, &menu_new_p, sizeof menu_new_p, menu_new);
}

// menu item

static struct {
    int16_t id;
    int16_t parent;
} menu_item_p;

_Static_assert(sizeof menu_item_p == 4, "wrong menu_item_p align");

static char *menu_item_label = NULL;
static PipeBuffer *menu_target = NULL;

static void menu_item(void *action) {
    menu_item_add(menu_item_p.id, menu_item_p.parent, menu_item_label, action);
    // g_free(action) is eternal
    // g_free(menuItemLabel) is eternal
}

static void menu_item_item(void *label) {
    menu_item_label = label;
    io_buffer_malloc_call(menu_target, NULL, 0, menu_item);
}

static void cmd_menu_item(PipeBuffer *target) {
    menu_target = target;
    io_buffer_malloc_call(target, &menu_item_p, sizeof menu_item_p, menu_item_item);
}

// clipboard

static struct {
    int16_t id;
} clip_get_p;

_Static_assert(sizeof clip_get_p == 2, "wrong clip_get_p align");

static void clip_get() { request_clipboard(clip_get_p.id); }

static void cmd_clip_get(PipeBuffer *target) { io_buffer_call(target, &clip_get_p, sizeof clip_get_p, clip_get); }

static struct {
    int16_t id;
} clip_put_p;

_Static_assert(sizeof clip_put_p == 2, "wrong clip_put_p align");

static void clip_put(void *data) {
    set_clipboard(clip_put_p.id, data);
    g_free(data);
}

static void cmd_clip_put(PipeBuffer *target) {
    io_buffer_malloc_call(target, &clip_put_p, sizeof clip_put_p, clip_put);
}

// dispatch

void call_command(char command, PipeBuffer *target) {
    switch (command) {
    // application
    case 'S':
        cmd_a_size(target);
        break;
    case 'T':
        cmd_a_title(target);
        break;
    case 'X':
        cmd_a_exit(target);
        break;
    case 'V':
        cmd_a_version();
        break;

    // layout
    case 'Y':
        cmd_frame_new(target);
        break;
    case 'Q':
        cmd_frame_drop(target);
        break;
    case 'H':
        cmd_frame_size(target);
        break;
    case 'J':
        cmd_frame_raise(target);
        break;

    // window
    case 'D':
        cmd_w_new(target);
        break;
    case 'Z':
        cmd_w_size(target);
        break;
    case 'O':
        cmd_w_drop(target);
        break;
    case 'A':
        cmd_w_raise(target);
        break;

    // draw
    case 'W':
        cmd_w_show(target);
        break;
    case 'C':
        cmd_w_clear(target);
        break;
    case 'F':
        cmd_w_fill(target);
        break;
    case 'L':
        cmd_w_line(target);
        break;
    case 'U':
        cmd_w_text(target);
        break;
    case 'I':
        cmd_w_image(target);
        break;

    // image
    case 'B':
        cmd_image_new(target);
        break;
    case 'M':
        cmd_image_drop(target);
        break;

    // font
    case 'N':
        cmd_font_new(target);
        break;
    case 'K':
        cmd_font_drop(target);
        break;
    case 'P':
        cmd_font_split(target);
        break;
    case 'R':
        cmd_font_size(target);
        break;

    // menu
    case 'E':
        cmd_menu_new(target);
        break;
    case 'G':
        cmd_menu_item(target);
        break;

    // clipboard
    case '1':
        cmd_clip_get(target);
        break;
    case '2':
        cmd_clip_put(target);
        break;

    default:
        printf("unknown command, char = %d\n", command);
        exit(EXIT_FAILURE);
    }
}
