#include "terminal.h"
#include <gtk/gtk.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

static int layoutOffsetX = 0, layoutOffsetY = 0;

// General events

typedef struct {
    uint32_t id;
} general_event;

#define GENERAL_EVENT_DESTROY 1

gboolean on_delete(GtkWidget *widget G_GNUC_UNUSED, gpointer data G_GNUC_UNUSED) {
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

gboolean on_configure(GtkWindow *window, GdkEventConfigure *event, gpointer data G_GNUC_UNUSED) {
    static GtkWidget *layout;
    if (layout == NULL) {
        layout = layout_get_widget(1);
    }
    gtk_widget_translate_coordinates(layout, top, 0, 0, &layoutOffsetX, &layoutOffsetY);
    char command_type = 'f';
    configure_event e;
    e.width = event->width;
    e.height = event->height;
    e.inner_width = event->width - layoutOffsetX;
    e.inner_height = event->height - layoutOffsetY;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
    return TRUE;
}

// Keyboard events

typedef struct {
    uint32_t unicode;
    uint8_t shift;
    uint8_t control;
    uint8_t alt;
    uint8_t meta;
} keyboard_event;

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
    int16_t length = strlen(name);
    pipe_event_write(&length, sizeof length);
    pipe_event_write(name, length);
    pipe_event_flush();
    return TRUE;
}

// Mouse events

typedef struct {
    uint8_t type;
    uint8_t button;
    uint16_t x, y;
} button_event;

gboolean s_button(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    char command_type = 'b';
    button_event e;
    e.type = event->type;
    e.button = event->button;
    e.x = (int16_t)(rintl(event->x) - layoutOffsetX);
    e.y = (int16_t)(rintl(event->y) - layoutOffsetY);
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
    return TRUE;
}

typedef struct {
    uint16_t x, y;
    uint8_t shift;
    uint8_t control;
    uint8_t alt;
    uint8_t meta;
} motion_event;

gboolean s_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data) {
    char command_type = 'm';
    motion_event e;
    e.x = (int16_t)(rintl(event->x) - layoutOffsetX);
    e.y = (int16_t)(rintl(event->y) - layoutOffsetY);
    e.shift = event->state & GDK_SHIFT_MASK ? 1 : 0;
    e.control = event->state & GDK_CONTROL_MASK ? 1 : 0;
    e.alt = event->state & GDK_MOD1_MASK ? 1 : 0;
    e.meta = event->state & GDK_META_MASK ? 1 : 0;
    pipe_event_write(&command_type, sizeof command_type);
    pipe_event_write(&e, sizeof e);
    pipe_event_flush();
    return TRUE;
}

typedef struct {
    uint16_t direction;
    uint16_t delta_x, delta_y;
} scroll_event;

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
    int16_t length = strlen(name);
    pipe_event_write(&length, sizeof length);
    pipe_event_write(name, length);
    pipe_event_flush();
}
