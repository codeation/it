#include "glib-object.h"
#include "terminal.h"
#include <gtk/gtk.h>
#include <math.h>

static int layoutOffsetX = 0, layoutOffsetY = 0;

// General events

typedef struct {
    uint32_t id;
} GeneralEvent;

_Static_assert(sizeof(GeneralEvent) == 4, "wrong GeneralEvent align");

#define GENERAL_EVENT_DESTROY 1

static gboolean on_delete(GtkWidget *widget, GdkEvent *event, gpointer data) {
    char command_type = 'g';
    GeneralEvent e;
    e.id = GENERAL_EVENT_DESTROY;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
    return TRUE;
}

typedef struct {
    uint16_t width, height;
    uint16_t inner_width, inner_height;
} ConfigureEvent;

_Static_assert(sizeof(ConfigureEvent) == 8, "wrong ConfigureEvent align");

static void write_configure_event_once(ConfigureEvent *e) {
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

static void on_size_allocate(GtkWidget *widget, GtkAllocation *allocation, gpointer data) {
    layoutOffsetX = allocation->x;
    layoutOffsetY = allocation->y;
    gint width, height;
    gtk_window_get_size(GTK_WINDOW(top), &width, &height);
    ConfigureEvent e;
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
} KeyboardEvent;

_Static_assert(sizeof(KeyboardEvent) == 8, "wrong KeyboardEvent align");

static gboolean s_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    char command_type = 'k';
    KeyboardEvent e;
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
} ButtonEvent;

_Static_assert(sizeof(ButtonEvent) == 6, "wrong ButtonEvent align");

static gboolean s_button(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    static guint32 prev_time = 0;
    if (event->time == prev_time) {
        return FALSE;
    }
    prev_time = event->time;
    char command_type = 'b';
    ButtonEvent e;
    e.type = event->type;
    e.button = event->button;
    e.x = (int16_t)(lrint(event->x) - layoutOffsetX);
    e.y = (int16_t)(lrint(event->y) - layoutOffsetY);
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
} MotionEvent;

_Static_assert(sizeof(MotionEvent) == 8, "wrong MotionEvent align");

static gboolean s_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    static guint32 prev_time = 0;
    if (event->time == prev_time) {
        return FALSE;
    }
    prev_time = event->time;
    char command_type = 'm';
    MotionEvent e;
    e.x = (int16_t)(lrint(event->x) - layoutOffsetX);
    e.y = (int16_t)(lrint(event->y) - layoutOffsetY);
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
} ScrollEvent;

_Static_assert(sizeof(ScrollEvent) == 6, "wrong ScrollEvent align");

static gboolean s_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer data) {
    char command_type = 's';
    ScrollEvent e;
    e.direction = event->direction;
    e.delta_x = (int16_t)lrint(event->delta_x);
    e.delta_y = (int16_t)lrint(event->delta_y);
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
    return TRUE;
}

// Menu events

void s_menu_action(char *action) {
    char command_type = 'u';
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write_string(action);
    pipe_event_flush();
}

// Clipboard events

typedef struct {
    int16_t format;
} ClipboardEvent;

_Static_assert(sizeof(ClipboardEvent) == 2, "wrong ClipboardEvent align");

static void s_text_received(GtkClipboard *clipboard, const gchar *text, gpointer data) {
    if (text == NULL) {
        return;
    }
    char command_type = 'c';
    pipe_event_write(&command_type, sizeof command_type);
    ClipboardEvent e;
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

void set_clipboard(int clipboardtypeid, gpointer data) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, data, -1); // TODO text only
}

// Signal connect

void top_signal_connect() {
    g_signal_connect(top, "delete-event", G_CALLBACK(on_delete), NULL);
    g_signal_connect(top, "key_press_event", G_CALLBACK(s_keypress), NULL);
    g_signal_connect(top, "button_press_event", G_CALLBACK(s_button), NULL);
    g_signal_connect(top, "button_release_event", G_CALLBACK(s_button), NULL);
    g_signal_connect(top, "motion_notify_event", G_CALLBACK(s_motion), NULL);
    g_signal_connect(top, "scroll-event", G_CALLBACK(s_scroll), NULL);
}

void top_signal_disconnect() {
    g_signal_handlers_disconnect_by_func(top, G_CALLBACK(on_delete), NULL);
    g_signal_handlers_disconnect_by_func(top, G_CALLBACK(s_keypress), NULL);
    g_signal_handlers_disconnect_by_func(top, G_CALLBACK(s_button), NULL);
    g_signal_handlers_disconnect_by_func(top, G_CALLBACK(s_motion), NULL);
    g_signal_handlers_disconnect_by_func(top, G_CALLBACK(s_scroll), NULL);
}

void layout_signal_connect(GtkWidget *layout) {
    g_signal_connect(layout, "size-allocate", G_CALLBACK(on_size_allocate), NULL);
}

void layout_signal_disconnect(GtkWidget *layout) {
    g_signal_handlers_disconnect_by_func(layout, G_CALLBACK(on_size_allocate), NULL);
}
