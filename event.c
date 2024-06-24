#include "terminal.h"
#include <gtk/gtk.h>
#include <math.h>

static int layoutOffsetX = 0, layoutOffsetY = 0;

// General events

typedef struct {
    uint32_t id;
} general_event;

_Static_assert(sizeof(general_event) == 4, "wrong general_event align");

#define GENERAL_EVENT_DESTROY 1

gboolean on_delete(GtkWidget *widget, GdkEvent *event, gpointer data) {
    char command_type = 'g';
    general_event e;
    e.id = GENERAL_EVENT_DESTROY;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
    return TRUE;
}

typedef struct {
    uint16_t width, height;
    uint16_t inner_width, inner_height;
} configure_event;

_Static_assert(sizeof(configure_event) == 8, "wrong configure_event align");

static void write_configure_event_once(configure_event *e) {
    static gint width = 0, height = 0, inner_width = 0, inner_height = 0;
    if (e->width == width && e->height == height && e->inner_width == inner_width && e->inner_height == inner_height) {
        return;
    }
    width = e->width;
    height = e->height;
    inner_width = e->inner_width;
    inner_height = e->inner_height;
    char command_type = 'f';
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(e, sizeof *e);
    pipe_event_flush();
}

void on_size_allocate(GtkWidget *widget, GtkAllocation *allocation, void *data) {
    layoutOffsetX = allocation->x;
    layoutOffsetY = allocation->y;
    gint width, height;
    gtk_window_get_size(GTK_WINDOW(top), &width, &height);
    configure_event e;
    e.width = width;
    e.height = height;
    e.inner_width = allocation->width;
    e.inner_height = allocation->height;
    write_configure_event_once(&e);
}

// Keyboard events

typedef struct {
    uint32_t unicode;
    uint8_t shift;
    uint8_t control;
    uint8_t alt;
    uint8_t meta;
} keyboard_event;

_Static_assert(sizeof(keyboard_event) == 8, "wrong keyboard_event align");

gboolean s_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    char command_type = 'k';
    keyboard_event e;
    e.unicode = gdk_keyval_to_unicode(event->keyval);
    e.shift = event->state & GDK_SHIFT_MASK ? 1 : 0;
    e.control = event->state & GDK_CONTROL_MASK ? 1 : 0;
    e.alt = event->state & GDK_MOD1_MASK ? 1 : 0;
    e.meta = event->state & GDK_META_MASK ? 1 : 0;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    char *name = gdk_keyval_name(event->keyval);
    pipe_event_write_string(name);
    pipe_event_flush();
    return TRUE;
}

// Mouse events

typedef struct {
    uint8_t type;
    uint8_t button;
    uint16_t x, y;
} button_event;

_Static_assert(sizeof(button_event) == 6, "wrong button_event align");

gboolean s_button(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    static guint32 prev_time = 0;
    if (event->time == prev_time) {
        return FALSE;
    }
    prev_time = event->time;
    char command_type = 'b';
    button_event e;
    e.type = event->type;
    e.button = event->button;
    e.x = (int16_t)(rint(event->x) - layoutOffsetX);
    e.y = (int16_t)(rint(event->y) - layoutOffsetY);
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
    return FALSE;
}

typedef struct {
    uint16_t x, y;
    uint8_t shift;
    uint8_t control;
    uint8_t alt;
    uint8_t meta;
} motion_event;

_Static_assert(sizeof(motion_event) == 8, "wrong motion_event align");

gboolean s_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    static guint32 prev_time = 0;
    if (event->time == prev_time) {
        return FALSE;
    }
    prev_time = event->time;
    char command_type = 'm';
    motion_event e;
    e.x = (int16_t)(rint(event->x) - layoutOffsetX);
    e.y = (int16_t)(rint(event->y) - layoutOffsetY);
    e.shift = event->state & GDK_SHIFT_MASK ? 1 : 0;
    e.control = event->state & GDK_CONTROL_MASK ? 1 : 0;
    e.alt = event->state & GDK_MOD1_MASK ? 1 : 0;
    e.meta = event->state & GDK_META_MASK ? 1 : 0;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
    return FALSE;
}

typedef struct {
    int16_t direction;
    int16_t delta_x, delta_y;
} scroll_event;

_Static_assert(sizeof(scroll_event) == 6, "wrong scroll_event align");

gboolean s_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer data) {
    char command_type = 's';
    scroll_event e;
    e.direction = event->direction;
    e.delta_x = event->delta_x;
    e.delta_y = event->delta_y;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
    return TRUE;
}

// Menu events

void s_menu_action(char *name) {
    char command_type = 'u';
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write_string(name);
    pipe_event_flush();
}

// Clipboard events

typedef struct {
    int16_t format;
} clipboard_event;

_Static_assert(sizeof(clipboard_event) == 2, "wrong clipboard_event align");

void s_text_received(GtkClipboard *clipboard, const gchar *text, gpointer data) {
    if (text == NULL) {
        return;
    }
    char command_type = 'c';
    pipe_event_write(&command_type, sizeof command_type);
    clipboard_event e;
    e.format = 1;
    pipe_event_write(&e, sizeof e);
    pipe_event_write_string(text);
    pipe_event_flush();
}

void request_clipboard(int clipboardtypeid) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_request_text(clipboard, s_text_received, NULL); // TODO text only
}

// Clipboard funcs

void set_clipboard(int clipboardtypeid, void *data) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, data, -1); // TODO text only
}
