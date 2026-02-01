#include "terminal.h"
#include <gtk/gtk.h>

typedef struct _PipeBuffer {
    char *buffer;
    gsize size;
    void (*ready_func)(PipeBuffer *target);
    guint in_id;
    guint hup_id;
    union {
        char command;
        void (*call_func)();
        struct {
            void (*data_func)(gpointer data);
            char *data;
            int32_t size;
        } malloc;
    } way;
} PipeBuffer;

static PipeBuffer sync_chan, stream_chan;

gboolean io_is_sync(PipeBuffer *target) { return target == &sync_chan; }

// first way: call command after command byte is ready

static void reset_buffer(PipeBuffer *target);

static void call_command(PipeBuffer *target) {
    reset_buffer(target);
    callcommand(target->way.command, target);
}

static void reset_buffer(PipeBuffer *target) {
    target->buffer = &target->way.command;
    target->size = sizeof target->way.command;
    target->ready_func = call_command;
}

// second way: call func after buffer is ready

static void call_buffer_func(PipeBuffer *target) {
    reset_buffer(target);
    target->way.call_func();
}

void io_buffer_call(PipeBuffer *target, void *buffer, int size, void (*call_func)()) {
    target->buffer = buffer;
    target->size = size;
    target->ready_func = call_buffer_func;
    target->way.call_func = call_func;
}

// third way: call data func after both buffer and malloc are ready

static void call_data_func(PipeBuffer *target) {
    reset_buffer(target);
    target->way.malloc.data_func(target->way.malloc.data); // func must free the data after all
    // g_free(target->data);
}

static void read_alloc_data(PipeBuffer *target) {
    target->way.malloc.data = g_malloc(target->way.malloc.size + 1);
    target->way.malloc.data[target->way.malloc.size] = 0;
    if (target->way.malloc.size == 0) {
        call_data_func(target);
    } else {
        target->buffer = target->way.malloc.data;
        target->size = target->way.malloc.size;
        target->ready_func = call_data_func;
    }
}

static void read_alloc_size(PipeBuffer *target) {
    target->buffer = (char *)&(target->way.malloc.size);
    target->size = sizeof target->way.malloc.size;
    target->ready_func = read_alloc_data;
}

void io_buffer_malloc_call(PipeBuffer *target, void *buffer, int size, void (*data_func)(gpointer data)) {
    target->way.malloc.data_func = data_func;
    if (buffer == NULL) {
        read_alloc_size(target);
    } else {
        target->buffer = buffer;
        target->size = size;
        target->ready_func = read_alloc_size;
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
            target->ready_func(target);
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

static void io_start(FILE *source, PipeBuffer *target, gboolean is_stream) {
    reset_buffer(target);
    GIOChannel *chan = g_io_channel_unix_new(fileno(source));
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
    g_io_channel_unref(chan);
}

void io_input_start(FILE *source) { io_start(source, &sync_chan, FALSE); }
void io_stream_start(FILE *source) { io_start(source, &stream_chan, TRUE); }

void io_stop(PipeBuffer *target) {
    g_source_remove(target->hup_id);
    g_source_remove(target->in_id);
    target->in_id = 0;
}

gboolean io_exited() { return sync_chan.in_id == 0 && stream_chan.in_id == 0; }
