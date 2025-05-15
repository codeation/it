#include "terminal.h"
#include <gtk/gtk.h>

// General events

typedef struct {
    uint32_t id;
} general_event;

_Static_assert(sizeof(general_event) == 4, "wrong general_event align");

#define GENERAL_EVENT_DESTROY 1

gboolean on_delete(GtkWidget *widget, gpointer data) {
    char command_type = 'g';
    general_event e;
    e.id = GENERAL_EVENT_DESTROY;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
    return TRUE;
}

// Configure event

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

static int idle_count = 0;
static GtkWidget *inner_size_widget = NULL;

void set_inner_size_widget(GtkWidget *widget) { inner_size_widget = widget; }

static void on_configure_event(gpointer user_data) {
    int inner_width = gtk_widget_get_width(inner_size_widget);
    int inner_height = gtk_widget_get_height(inner_size_widget);
    if (inner_width == 0 && inner_height == 0) {
        return;
    }
    gint width, height;
    gtk_window_get_default_size(GTK_WINDOW(top), &width, &height);
    configure_event e;
    e.width = width;
    e.height = height;
    e.inner_width = inner_width;
    e.inner_height = inner_height;
    write_configure_event_once(&e);
}

static gboolean idle_func(gpointer user_data) {
    idle_count--;
    if (idle_count > 0) {
        return G_SOURCE_CONTINUE;
    }
    on_configure_event(user_data);
    return G_SOURCE_REMOVE;
}

void size_notify(GObject *self, GParamSpec *pspec, gpointer user_data) {
    if (idle_count <= 0) {
        g_idle_add(idle_func, user_data);
    }
    idle_count++;
}

void adjustment_notify(GtkAdjustment *self, gpointer user_data) {
    if (idle_count <= 0) {
        g_idle_add(idle_func, user_data);
    }
    idle_count++;
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

gboolean key_pressed(GtkEventControllerKey *self, guint keyval, guint keycode, GdkModifierType state,
                     gpointer user_data) {
    char command_type = 'k';
    keyboard_event e;
    e.unicode = gdk_keyval_to_unicode(keyval);
    e.shift = state & GDK_SHIFT_MASK ? 1 : 0;
    e.control = state & GDK_CONTROL_MASK ? 1 : 0;
    e.alt = state & GDK_ALT_MASK ? 1 : 0;
    e.meta = state & GDK_META_MASK ? 1 : 0;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    const char *name = gdk_keyval_name(keyval);
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

static int buttonType(int n_press) {
    switch (n_press) {
    case 1:
        return 4;
    case 2:
        return 5;
    case 3:
        return 6;
    default:
        return 7;
    }
}

static guint32 prev_button_time = 0;
static GdkEventType prev_button_type = 0;

void button_pressed(GtkGestureClick *self, gint n_press, gdouble x, gdouble y, gpointer user_data) {
    GdkEvent *event = gtk_event_controller_get_current_event(GTK_EVENT_CONTROLLER(self));
    if (gdk_event_get_time(event) == prev_button_time && gdk_event_get_event_type(event) == prev_button_type) {
        return;
    }
    prev_button_time = gdk_event_get_time(event);
    prev_button_type = gdk_event_get_event_type(event);
    char command_type = 'b';
    button_event e;
    e.type = buttonType(n_press);
    e.button = gdk_button_event_get_button(event);
    e.x = (int16_t)x;
    e.y = (int16_t)y;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
}

void button_released(GtkGestureClick *self, gint n_press, gdouble x, gdouble y, gpointer user_data) {
    GdkEvent *event = gtk_event_controller_get_current_event(GTK_EVENT_CONTROLLER(self));
    if (gdk_event_get_time(event) == prev_button_time && gdk_event_get_event_type(event) == prev_button_type) {
        return;
    }
    prev_button_time = gdk_event_get_time(event);
    prev_button_type = gdk_event_get_event_type(event);
    char command_type = 'b';
    button_event e;
    e.type = 7;
    e.button = gdk_button_event_get_button(event);
    e.x = (int16_t)x;
    e.y = (int16_t)y;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
}

typedef struct {
    uint16_t x, y;
    uint8_t shift;
    uint8_t control;
    uint8_t alt;
    uint8_t meta;
} motion_event;

_Static_assert(sizeof(motion_event) == 8, "wrong motion_event align");

void motion_notify(GtkEventControllerMotion *self, gdouble x, gdouble y, gpointer user_data) {
    GdkEvent *event = gtk_event_controller_get_current_event(GTK_EVENT_CONTROLLER(self));
    if (gdk_event_get_time(event) == prev_button_time && gdk_event_get_event_type(event) == prev_button_type) {
        return;
    }
    prev_button_time = gdk_event_get_time(event);
    prev_button_type = gdk_event_get_event_type(event);
    char command_type = 'm';
    GdkModifierType state = gdk_event_get_modifier_state(event);
    motion_event e;
    e.x = (int16_t)x;
    e.y = (int16_t)y;
    e.shift = state & GDK_SHIFT_MASK ? 1 : 0;
    e.control = state & GDK_CONTROL_MASK ? 1 : 0;
    e.alt = state & GDK_ALT_MASK ? 1 : 0;
    e.meta = state & GDK_META_MASK ? 1 : 0;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
}

typedef struct {
    int16_t direction;
    int16_t delta_x, delta_y;
} scroll_event;

_Static_assert(sizeof(scroll_event) == 6, "wrong scroll_event align");

gboolean scroll_notify(GtkEventControllerScroll *self, gdouble dx, gdouble dy, gpointer user_data) {
    char command_type = 's';
    scroll_event e;
    e.direction = 4;
    e.delta_x = (int16_t)dx;
    e.delta_y = (int16_t)dy;
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
} clipboard_event;

_Static_assert(sizeof(clipboard_event) == 2, "wrong clipboard_event align");

void clipboard_text_received(GObject *source_object, GAsyncResult *res, gpointer data) {
    char *text = gdk_clipboard_read_text_finish(GDK_CLIPBOARD(source_object), res, NULL);
    char command_type = 'c';
    clipboard_event e;
    e.format = 1;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_write_string(text);
    pipe_event_flush();
}

void request_clipboard(int clipboardtypeid) {
    GdkClipboard *clipboard = gdk_display_get_clipboard(gdk_display_get_default());
    gdk_clipboard_read_text_async(clipboard, NULL, clipboard_text_received, NULL);
}

// Clipboard funcs

void set_clipboard(int clipboardtypeid, void *data) {
    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_STRING);
    g_value_set_string(&value, data);
    GdkClipboard *clipboard = gtk_widget_get_clipboard(top);
    gdk_clipboard_set_value(clipboard, &value);
    g_value_unset(&value);
}
