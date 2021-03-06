#include "terminal.h"
#include <gtk/gtk.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

// General events

typedef struct {
    uint32_t id;
} general_event;

#define GENERAL_EVENT_DESTROY 1

gboolean on_delete(GtkWidget *widget G_GNUC_UNUSED, gpointer user_data G_GNUC_UNUSED) {
    char command_type = 'g';
    general_event e;
    e.id = GENERAL_EVENT_DESTROY;
    socket_output_write(&command_type, sizeof command_type);
    socket_output_write(&e, sizeof e);
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
    socket_output_write(&command_type, sizeof command_type);
    socket_output_write(&e, sizeof e);
    char *name = gdk_keyval_name(event->keyval);
    int16_t length = strlen(name);
    socket_output_write(&length, sizeof length);
    socket_output_write(name, length);
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
    e.x = (int16_t)rintl(event->x);
    e.y = (int16_t)rintl(event->y);
    socket_output_write(&command_type, sizeof command_type);
    socket_output_write(&e, sizeof e);
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
    e.x = (int16_t)rintl(event->x);
    e.y = (int16_t)rintl(event->y);
    e.shift = event->state & GDK_SHIFT_MASK ? 1 : 0;
    e.control = event->state & GDK_CONTROL_MASK ? 1 : 0;
    e.alt = event->state & GDK_MOD1_MASK ? 1 : 0;
    e.meta = event->state & GDK_META_MASK ? 1 : 0;
    socket_output_write(&command_type, sizeof command_type);
    socket_output_write(&e, sizeof e);
    return TRUE;
}

// Menu events

void s_menu_action(char *name) {
    char command_type = 'u';
    socket_output_write(&command_type, sizeof command_type);
    int16_t length = strlen(name);
    socket_output_write(&length, sizeof length);
    socket_output_write(name, length);
}
