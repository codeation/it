#include "terminal.h"
#include <gtk/gtk.h>

enum State {
    COMMAND, // reading a single-letter command before callback
    DATA,    // reading user data before callback
    LEAD,    // reading user data before reading size of allocated buffer
    SIZE,    // reading size of allocated buffer before reading its contents
    BUFFER,  // reading contents of the allocated buffer before callback
};

typedef struct _PipeBuffer {
    char *buffer;
    uint32_t size;
    enum State state;
    guint in_id;
    guint hup_id;
    union {
        char command;
        void (*call_func)();
        struct {
            void (*data_func)(gpointer data);
            char *data;
            uint32_t size;
        } malloc;
    } way;
} PipeBuffer;

static PipeBuffer sync_chan, stream_chan;

gboolean io_is_sync(PipeBuffer *target) { return target == &sync_chan; }

void io_buffer_call(PipeBuffer *target, void *buffer, uint32_t size, void (*call_func)()) {
    target->buffer = buffer;
    target->size = size;
    target->state = DATA;
    target->way.call_func = call_func;
}

void io_buffer_malloc_call(PipeBuffer *target, void *buffer, uint32_t size, void (*data_func)(gpointer data)) {
    target->way.malloc.data_func = data_func;
    if (buffer == NULL) {
        target->buffer = (char *)&(target->way.malloc.size);
        target->size = sizeof target->way.malloc.size;
        target->state = SIZE;
    } else {
        target->buffer = buffer;
        target->size = size;
        target->state = LEAD;
    }
}

static void reset_buffer(PipeBuffer *target) {
    target->buffer = &target->way.command;
    target->size = sizeof target->way.command;
    target->state = COMMAND;
}

static void on_data_received(PipeBuffer *target) {
    switch (target->state) {
    case COMMAND: // first way: call command after command byte is ready
        reset_buffer(target);
        call_command(target->way.command, target);
        break;
    case DATA: // second way: call func after user data is ready
        reset_buffer(target);
        target->way.call_func();
        break;
    case LEAD: // third way: get size of allocated memory after user data is ready
        target->buffer = (char *)&(target->way.malloc.size);
        target->size = sizeof target->way.malloc.size;
        target->state = SIZE;
        break;
    case SIZE: // third way: allocating memory after the size is read
        target->way.malloc.data = g_malloc(target->way.malloc.size + 1);
        target->way.malloc.data[target->way.malloc.size] = 0;
        if (target->way.malloc.size == 0) {
            reset_buffer(target);
            target->way.malloc.data_func(target->way.malloc.data); // func must free the data after all
            // g_free(target->data);
            break;
        }
        target->buffer = target->way.malloc.data;
        target->size = target->way.malloc.size;
        target->state = BUFFER;
        break;
    case BUFFER: // third way: call data func after user data and allocated memory are ready
        reset_buffer(target);
        target->way.malloc.data_func(target->way.malloc.data); // func must free the data after all
        // g_free(target->data);
        break;
    }
}

static gboolean async_read_chan(GIOChannel *source, GIOCondition condition, gpointer data) {
    PipeBuffer *target = data;
    while (TRUE) {
        gsize len = 0;
        GIOStatus status = g_io_channel_read_chars(source, target->buffer, target->size, &len, NULL);
        switch (status) {
        case G_IO_STATUS_NORMAL:
            if (len < target->size) {
                // shift
                target->buffer += len;
                target->size -= len;
                // no more data
                return TRUE;
            }
            // else call next func after getting data
            on_data_received(target);
            break;
        case G_IO_STATUS_AGAIN:
            // no more data
            return TRUE;
        default:
            printf("read status: %d\n", status);
            exit(EXIT_FAILURE);
        }
    }
}

static gboolean chan_error_func(GIOChannel *source, GIOCondition condition, gpointer data) {
    perror("connection has been broken");
    exit(EXIT_FAILURE);
    return TRUE;
}

static void io_start(GIOChannel *chan, PipeBuffer *target, gboolean is_stream) {
    reset_buffer(target);
    GIOStatus status = g_io_channel_set_flags(chan, G_IO_FLAG_NONBLOCK, NULL);
    if (status != G_IO_STATUS_NORMAL) {
        printf("set flag status: %d\n", status);
        exit(EXIT_FAILURE);
    }
    status = g_io_channel_set_encoding(chan, NULL, NULL);
    if (status != G_IO_STATUS_NORMAL) {
        printf("set encoding status: %d\n", status);
        exit(EXIT_FAILURE);
    }
    if (is_stream) {
        g_io_channel_set_buffer_size(chan, 65536UL);
    }
    target->in_id = g_io_add_watch(chan, G_IO_IN, async_read_chan, target);
    target->hup_id = g_io_add_watch(chan, G_IO_HUP, chan_error_func, target);
}

void io_input_start(GIOChannel *chan) { io_start(chan, &sync_chan, FALSE); }
void io_stream_start(GIOChannel *chan) { io_start(chan, &stream_chan, TRUE); }

void io_stop(PipeBuffer *target) {
    g_source_remove(target->hup_id);
    g_source_remove(target->in_id);
    target->in_id = 0;
}

gboolean io_exited() { return sync_chan.in_id == 0 && stream_chan.in_id == 0; }
