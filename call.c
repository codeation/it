#include "terminal.h"
#include <gtk/gtk.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// driver version

static void commandVersion() {
    int16_t length = strlen(it_version);
    socket_input_write(&length, sizeof length);
    socket_input_write(it_version, length);
}

// clear command

static struct { int16_t id; } windowid;

static void setClear() { elem_clear(windowid.id); }

static void commandClear() { readbuffcall(&windowid, sizeof windowid, setClear); }

// show command

static void setShow() { window_redraw(windowid.id); }

static void commandShow() { readbuffcall(&windowid, sizeof windowid, setShow); }

// fill command

static struct {
    int16_t id;
    int16_t x, y, width, height;
    int16_t r, g, b;
} fill;

static void setFill() {
    elem_fill_add(fill.id, fill.x, fill.y, fill.width, fill.height, fill.r, fill.g, fill.b);
}

static void commandFill() { readbuffcall(&fill, sizeof fill, setFill); }

// draw line

static struct {
    int16_t id;
    int16_t x0, y0;
    int16_t x1, y1;
    int16_t r, g, b;
} line;

static void setLine() {
    elem_line_add(line.id, line.x0, line.y0, line.x1, line.y1, line.r, line.g, line.b);
}

static void commandLine() { readbuffcall(&line, sizeof line, setLine); }

// draw string

static struct {
    int16_t id;
    int16_t x, y;
    int16_t r, g, b;
    int16_t fontid;
    int16_t fontsize;
} point;

static void setText(void *text) {
    elem_text_add(point.id, point.x, point.y, text, point.fontid, point.r, point.g, point.b);
}

void commandText() { readalloccall(&point, sizeof point, setText); }

// draw image

static struct {
    int16_t id;
    int16_t x, y;
    int16_t imageid;
} image;

static void setImage() { elem_image_add(image.id, image.x, image.y, image.imageid); }

static void commandImage() { readbuffcall(&image, sizeof image, setImage); }

// load image

static struct {
    int16_t id;
    int16_t width, height;
} imageAdd;

static void setImageAdd(void *data) {
    image_add(imageAdd.id, imageAdd.width, imageAdd.height, data);
}

static void commandImageAdd() { readalloccall(&imageAdd, sizeof imageAdd, setImageAdd); }

// remove image

static struct { int16_t id; } imageid;

static void setImageRem() { image_rem(imageid.id); }

static void commandImageRem() { readbuffcall(&imageid, sizeof imageid, setImageRem); }

// load font

static struct {
    int16_t id;
    int16_t height;
    int16_t style, variant, weight, stretch;
} font;

static void setFont(void *family) {
    font_elem_add(font.id, font.height, family, font.style, font.variant, font.weight,
                  font.stretch);
    int16_t baseline, ascent, descent;
    get_font_metrics(font.id, &baseline, &ascent, &descent);
    socket_input_write(&baseline, sizeof baseline);
    socket_input_write(&ascent, sizeof ascent);
    socket_input_write(&descent, sizeof descent);
}

static void commandFont() { readalloccall(&font, sizeof font, setFont); }

// split text

static struct {
    int16_t fontid;
    int16_t edge;
} split;

static void splitText(void *text) {
    int16_t *out = font_split_text(split.fontid, text, split.edge);
    if (out == NULL) {
        int16_t value = 0;
        socket_input_write(&value, sizeof value);
    } else {
        int length = (1 + *out) * sizeof(int16_t);
        socket_input_write(out, length);
        free(out);
    }
}

static void commandSplit() { readalloccall(&split, sizeof split, splitText); }

// text rect

static struct { int16_t fontid; } textrect;

static void rectText(void *text) {
    int16_t width, height;
    font_rect_text(textrect.fontid, text, &width, &height);
    socket_input_write(&width, sizeof width);
    socket_input_write(&height, sizeof width);
}

static void commandRect() { readalloccall(&textrect, sizeof textrect, rectText); }

// app window size

static struct {
    int16_t x, y;
    int16_t width, height;
} size;

static void setSize() {
    gtk_window_move(GTK_WINDOW(top), size.x, size.y);
    gtk_window_resize(GTK_WINDOW(top), size.width, size.height);
    gtk_widget_show_all(top);
}

static void commandSize() { readbuffcall(&size, sizeof size, setSize); }

// app window title

static void setTitle(void *buff) {
    gtk_window_set_title(GTK_WINDOW(top), buff);
    free(buff);
}

static void commandTitle() { readalloccall(NULL, 0, setTitle); }

// window

static struct {
    int16_t id;
    int16_t x, y;
    int16_t width, height;
    int16_t r, g, b;
} window;

static void setWindow() {
    window_create(window.id);
    window_size(window.id, window.x, window.y, window.width, window.height);
}

static void commandWindow() { readbuffcall(&window, sizeof window, setWindow); }

// window size

static void setWindowSize() {
    window_size(window.id, window.x, window.y, window.width, window.height);
}

static void commandWindowSize() { readbuffcall(&window, sizeof window, setWindowSize); }

// drop window command

static void dropWindow() { window_destroy(windowid.id); }

static void commandWindowDrop() { readbuffcall(&windowid, sizeof windowid, dropWindow); }

// window raise

static void raiseWindow() { window_raise(windowid.id); }

static void commandWindowRaise() { readbuffcall(&windowid, sizeof windowid, raiseWindow); }

// menu node

static struct {
    int16_t id;
    int16_t parent;
} menu;

static void addMenu(void *label) { menu_node_add(menu.id, menu.parent, label); }

static void commandMenu() { readalloccall(&menu, sizeof menu, addMenu); }

// menu item

static char *menuItemLabel = NULL;

static void addMenuItem(void *action) {
    menu_item_add(menu.id, menu.parent, menuItemLabel, action);
}

static void setMenuItem(void *label) {
    menuItemLabel = label;
    readalloccall(NULL, 0, addMenuItem);
}

static void commandMenuItem() { readalloccall(&menu, sizeof menu, setMenuItem); }

// dispatch

void callcommand(char command) {
    switch (command) {
    // application
    case 'S':
        commandSize();
        break;
    case 'T':
        commandTitle();
        break;
    case 'X':
        g_application_quit(G_APPLICATION(app));
        break;
    case 'V':
        commandVersion();
        break;

    // window
    case 'D':
        commandWindow();
        break;
    case 'Z':
        commandWindowSize();
        break;
    case 'O':
        commandWindowDrop();
        break;
    case 'A':
        commandWindowRaise();
        break;

    // draw
    case 'W':
        commandShow();
        break;
    case 'C':
        commandClear();
        break;
    case 'F':
        commandFill();
        break;
    case 'L':
        commandLine();
        break;
    case 'U':
        commandText();
        break;
    case 'I':
        commandImage();
        break;

    // image
    case 'B':
        commandImageAdd();
        break;
    case 'M':
        commandImageRem();
        break;

    // font
    case 'N':
        commandFont();
        break;
    case 'P':
        commandSplit();
        break;
    case 'R':
        commandRect();
        break;

    // menu
    case 'E':
        commandMenu();
        break;
    case 'G':
        commandMenuItem();
        break;
    }
}
