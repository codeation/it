#include "terminal.h"
#include <gtk/gtk.h>

// driver version

static void commandVersion() {
    pipe_output_write_string(it_api_version);
    pipe_output_flush();
}

// exit

static void commandExit(pipe_buffer *target) {
    io_stop(target);
    if (io_exited()) {
        g_application_quit(G_APPLICATION(app));
    }
}

// clear command

static struct { int16_t id; } windowid;

_Static_assert(sizeof windowid == 2, "wrong windowid align");

static void setClear() { window_clear(windowid.id); }

static void commandClear(pipe_buffer *target) { parameters_to_call(target, &windowid, sizeof windowid, setClear); }

// show command

static void setShow() { window_redraw(windowid.id); }

static void commandShow(pipe_buffer *target) { parameters_to_call(target, &windowid, sizeof windowid, setShow); }

// fill command

static struct {
    int16_t id;
    int16_t x, y, width, height;
    uint16_t r, g, b, a;
} fill;

_Static_assert(sizeof fill == 18, "wrong fill align");

static void setFill() {
    elem_fill_add(fill.id, fill.x, fill.y, fill.width, fill.height, fill.r, fill.g, fill.b, fill.a);
}

static void commandFill(pipe_buffer *target) { parameters_to_call(target, &fill, sizeof fill, setFill); }

// draw line

static struct {
    int16_t id;
    int16_t x0, y0;
    int16_t x1, y1;
    uint16_t r, g, b, a;
} line;

_Static_assert(sizeof line == 18, "wrong line align");

static void setLine() { elem_line_add(line.id, line.x0, line.y0, line.x1, line.y1, line.r, line.g, line.b, line.a); }

static void commandLine(pipe_buffer *target) { parameters_to_call(target, &line, sizeof line, setLine); }

// draw string

static struct {
    int16_t id;
    int16_t x, y;
    uint16_t r, g, b, a;
    int16_t fontid;
} point;

_Static_assert(sizeof point == 16, "wrong point align");

static void setText(void *text) {
    elem_text_add(point.id, point.x, point.y, text, point.fontid, point.r, point.g, point.b, point.a);
    // g_free(text) in elem_text_destroy
}

void commandText(pipe_buffer *target) { parameters_alloc_to_call(target, &point, sizeof point, setText); }

// draw image

static struct {
    int16_t id;
    int16_t x, y;
    int16_t width, height;
    int16_t imageid;
} image;

_Static_assert(sizeof image == 12, "wrong image align");

static void setImage() { elem_image_add(image.id, image.x, image.y, image.width, image.height, image.imageid); }

static void commandImage(pipe_buffer *target) { parameters_to_call(target, &image, sizeof image, setImage); }

// load image

static struct {
    int16_t id;
    int16_t width, height;
} imageAdd;

_Static_assert(sizeof imageAdd == 6, "wrong imageAdd align");

static void setImageAdd(void *data) {
    bitmap_add(imageAdd.id, imageAdd.width, imageAdd.height, data);
    // g_free(data) in image_rem
}

static void commandImageAdd(pipe_buffer *target) {
    parameters_alloc_to_call(target, &imageAdd, sizeof imageAdd, setImageAdd);
}

// remove image

static struct { int16_t id; } imageid;

_Static_assert(sizeof imageid == 2, "wrong imageid align");

static void setImageRem() { bitmap_rem(imageid.id); }

static void commandImageRem(pipe_buffer *target) { parameters_to_call(target, &imageid, sizeof imageid, setImageRem); }

// load font

static struct {
    int16_t id;
    int16_t height;
    int16_t style, variant, weight, stretch;
} font;

_Static_assert(sizeof font == 12, "wrong font align");

static void setFont(void *family) {
    font_elem_add(font.id, font.height, family, font.style, font.variant, font.weight, font.stretch);
    int16_t lineheight, baseline, ascent, descent;
    get_font_metrics(font.id, &lineheight, &baseline, &ascent, &descent);
    pipe_output_write(&lineheight, sizeof lineheight);
    pipe_output_write(&baseline, sizeof baseline);
    pipe_output_write(&ascent, sizeof ascent);
    pipe_output_write(&descent, sizeof descent);
    pipe_output_flush();
    g_free(family);
}

static void commandFont(pipe_buffer *target) { parameters_alloc_to_call(target, &font, sizeof font, setFont); }

// remove font

static struct { int16_t id; } fontid;

_Static_assert(sizeof fontid == 2, "wrong fontid align");

static void setFontRem() { font_elem_rem(fontid.id); }

static void commandFontRem(pipe_buffer *target) { parameters_to_call(target, &fontid, sizeof fontid, setFontRem); }

// split text

static struct {
    int16_t fontid;
    int16_t edge;
    int16_t indent;
} split;

_Static_assert(sizeof split == 6, "wrong split align");

static void splitText(void *text) {
    int16_t *out = font_split_text(split.fontid, text, split.edge, split.indent);
    if (out == NULL) {
        int16_t value = 0;
        pipe_output_write(&value, sizeof value);
        pipe_output_flush();
    } else {
        int length = (1 + *out) * sizeof(int16_t);
        pipe_output_write(out, length);
        pipe_output_flush();
        g_free(out);
    }
    g_free(text);
}

static void commandSplit(pipe_buffer *target) { parameters_alloc_to_call(target, &split, sizeof split, splitText); }

// text rect

static struct { int16_t fontid; } textrect;

static void rectText(void *text) {
    int16_t width, height;
    font_rect_text(textrect.fontid, text, &width, &height);
    pipe_output_write(&width, sizeof width);
    pipe_output_write(&height, sizeof width);
    pipe_output_flush();
    g_free(text);
}

_Static_assert(sizeof textrect == 2, "wrong textrect align");

static void commandRect(pipe_buffer *target) {
    parameters_alloc_to_call(target, &textrect, sizeof textrect, rectText);
}

// app window size

static struct {
    int16_t x, y;
    int16_t width, height;
} size;

_Static_assert(sizeof size == 8, "wrong size align");

static void setSize() {
    gtk_window_move(GTK_WINDOW(top), size.x, size.y);
    gtk_window_resize(GTK_WINDOW(top), size.width, size.height);
    // gtk_widget_show(top);
}

static void commandSize(pipe_buffer *target) { parameters_to_call(target, &size, sizeof size, setSize); }

// app window title

static void setTitle(void *title) {
    gtk_window_set_title(GTK_WINDOW(top), title);
    g_free(title);
}

static void commandTitle(pipe_buffer *target) { parameters_alloc_to_call(target, NULL, 0, setTitle); }

// layout

static struct {
    int16_t id;
    int16_t parent_id;
    int16_t x, y;
    int16_t width, height;
} layout;

_Static_assert(sizeof layout == 12, "wrong layout align");

static void setLayout() {
    layout_create(layout.id, layout.parent_id);
    layout_size(layout.id, layout.x, layout.y, layout.width, layout.height);
}

static void commandLayout(pipe_buffer *target) { parameters_to_call(target, &layout, sizeof layout, setLayout); }

// layout drop

static struct { int16_t id; } layoutid;

static void dropLayout() { layout_destroy(layoutid.id); }

static void commandLayoutDrop(pipe_buffer *target) {
    parameters_to_call(target, &layoutid, sizeof layoutid, dropLayout);
}

// layout size

static struct {
    int16_t id;
    int16_t x, y;
    int16_t width, height;
} layoutSize;

_Static_assert(sizeof layoutSize == 10, "wrong layoutSize align");

static void sizeLayout() {
    layout_size(layoutSize.id, layoutSize.x, layoutSize.y, layoutSize.width, layoutSize.height);
}

static void commandLayoutSize(pipe_buffer *target) {
    parameters_to_call(target, &layoutSize, sizeof layoutSize, sizeLayout);
}

// layout raise

static void raiseLayout() { layout_raise(layoutid.id); }

static void commandLayoutRaise(pipe_buffer *target) {
    parameters_to_call(target, &layoutid, sizeof layoutid, raiseLayout);
}

// window

static struct {
    int16_t id;
    int16_t layout_id;
    int16_t x, y;
    int16_t width, height;
} window;

_Static_assert(sizeof window == 12, "wrong window align");

static void setWindow() {
    window_create(window.id, window.layout_id);
    window_size(window.id, window.x, window.y, window.width, window.height);
}

static void commandWindow(pipe_buffer *target) { parameters_to_call(target, &window, sizeof window, setWindow); }

// window size

static struct {
    int16_t id;
    int16_t x, y;
    int16_t width, height;
} windowSize;

_Static_assert(sizeof windowSize == 10, "wrong windowSize align");

static void setWindowSize() {
    window_size(windowSize.id, windowSize.x, windowSize.y, windowSize.width, windowSize.height);
}

static void commandWindowSize(pipe_buffer *target) {
    parameters_to_call(target, &windowSize, sizeof windowSize, setWindowSize);
}

// drop window command

static void dropWindow() { window_destroy(windowid.id); }

static void commandWindowDrop(pipe_buffer *target) {
    parameters_to_call(target, &windowid, sizeof windowid, dropWindow);
}

// window raise

static void raiseWindow() { window_raise(windowid.id); }

static void commandWindowRaise(pipe_buffer *target) {
    parameters_to_call(target, &windowid, sizeof windowid, raiseWindow);
}

// menu node

static struct {
    int16_t id;
    int16_t parent;
} menu;

_Static_assert(sizeof menu == 4, "wrong menu align");

static void addMenu(void *label) {
    menu_node_add(menu.id, menu.parent, label);
    // g_free(label) is eternal
}

static void commandMenu(pipe_buffer *target) { parameters_alloc_to_call(target, &menu, sizeof menu, addMenu); }

// menu item

static char *menuItemLabel = NULL;
static pipe_buffer *menu_target = NULL;

static void addMenuItem(void *action) {
    menu_item_add(menu.id, menu.parent, menuItemLabel, action);
    // g_free(action) is eternal
    // g_free(menuItemLabel) is eternal
}

static void setMenuItem(void *label) {
    menuItemLabel = label;
    parameters_alloc_to_call(menu_target, NULL, 0, addMenuItem);
}

static void commandMenuItem(pipe_buffer *target) {
    menu_target = target;
    parameters_alloc_to_call(target, &menu, sizeof menu, setMenuItem);
}

// clipboard

static struct { int16_t id; } clipboardtypeid;

_Static_assert(sizeof clipboardtypeid == 2, "wrong copy align");

static void clipboardGet() { request_clipboard(clipboardtypeid.id); }

static void commandClipboardGet(pipe_buffer *target) {
    parameters_to_call(target, &clipboardtypeid, sizeof clipboardtypeid, clipboardGet);
}

static void clipboardPut(void *data) {
    set_clipboard(clipboardtypeid.id, data);
    g_free(data);
}

static void commandClipboardPut(pipe_buffer *target) {
    parameters_alloc_to_call(target, &clipboardtypeid, sizeof clipboardtypeid, clipboardPut);
}

// dispatch

void callcommand(char command, pipe_buffer *target) {
    switch (command) {
    // application
    case 'S':
        commandSize(target);
        break;
    case 'T':
        commandTitle(target);
        break;
    case 'X':
        commandExit(target);
        break;
    case 'V':
        commandVersion();
        break;

    // layout
    case 'Y':
        commandLayout(target);
        break;
    case 'Q':
        commandLayoutDrop(target);
        break;
    case 'H':
        commandLayoutSize(target);
        break;
    case 'J':
        commandLayoutRaise(target);
        break;

    // window
    case 'D':
        commandWindow(target);
        break;
    case 'Z':
        commandWindowSize(target);
        break;
    case 'O':
        commandWindowDrop(target);
        break;
    case 'A':
        commandWindowRaise(target);
        break;

    // draw
    case 'W':
        commandShow(target);
        break;
    case 'C':
        commandClear(target);
        break;
    case 'F':
        commandFill(target);
        break;
    case 'L':
        commandLine(target);
        break;
    case 'U':
        commandText(target);
        break;
    case 'I':
        commandImage(target);
        break;

    // image
    case 'B':
        commandImageAdd(target);
        break;
    case 'M':
        commandImageRem(target);
        break;

    // font
    case 'N':
        commandFont(target);
        break;
    case 'K':
        commandFontRem(target);
        break;
    case 'P':
        commandSplit(target);
        break;
    case 'R':
        commandRect(target);
        break;

    // menu
    case 'E':
        commandMenu(target);
        break;
    case 'G':
        commandMenuItem(target);
        break;

    // clipboard
    case '1':
        commandClipboardGet(target);
        break;
    case '2':
        commandClipboardPut(target);
        break;

    default:
        printf("unknown command, char = %d\n", command);
        exit(EXIT_FAILURE);
    }
}
