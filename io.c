#include "terminal.h"
#include <gtk/gtk.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct pipe_buffer {
    char *buffer;
    int size;
    void (*next_func)(pipe_buffer *target);
    char command;
    void (*call_func)();
    int32_t alloc_length;
    char *data;
    void (*last_func)(void *);
    guint in_id;
    guint hup_id;
} pipe_buffer;

static pipe_buffer sync_chan, stream_chan;

static void reset_buffer(pipe_buffer *target);

static void call_func(pipe_buffer *target) {
    reset_buffer(target);
    target->call_func();
}

void parameters_to_call(pipe_buffer *target, void *buffer, int size, void (*f)()) {
    target->buffer = buffer;
    target->size = size;
    target->next_func = call_func;
    target->call_func = f;
}

static void call_last_func(pipe_buffer *target) {
    reset_buffer(target);
    target->last_func(target->data); // func must free the data after all
    // free(target->data);
}

static void read_alloc_data(pipe_buffer *target) {
    target->data = malloc(target->alloc_length + 1);
    target->data[target->alloc_length] = 0;
    if (target->alloc_length == 0) {
        call_last_func(target);
    } else {
        target->buffer = target->data;
        target->size = target->alloc_length;
        target->next_func = call_last_func;
    }
}

static void read_alloc_length(pipe_buffer *target) {
    target->buffer = (char *)&(target->alloc_length);
    target->size = sizeof target->alloc_length;
    target->next_func = read_alloc_data;
}

void parameters_alloc_to_call(pipe_buffer *target, void *buffer, int size, void (*f)(void *)) {
    target->last_func = f;
    if (buffer == NULL) {
        read_alloc_length(target);
    } else {
        target->buffer = buffer;
        target->size = size;
        target->next_func = read_alloc_length;
    }
}

static void call_command(pipe_buffer *target) {
    reset_buffer(target);
    callcommand(target->command, target);
}

static void reset_buffer(pipe_buffer *target) {
    target->buffer = &target->command;
    target->size = sizeof target->command;
    target->next_func = call_command;
}

gboolean async_read_chan(GIOChannel *source, GIOCondition condition, gpointer data) {
    pipe_buffer *target = data;
    gsize len = 0;
    GIOStatus status = g_io_channel_read_chars(source, target->buffer, (gsize)target->size, &len, NULL);
    if (status != G_IO_STATUS_NORMAL) {
        printf("read status: %d\n", status);
        exit(EXIT_FAILURE);
    }
    if (len < target->size) {
        // shift
        target->buffer += len;
        target->size -= len;
    } else {
        // call next func after getting data
        target->next_func(target);
    }
    return TRUE;
}

gboolean chan_error_func(GIOChannel *source, GIOCondition condition, gpointer data) {
    perror("connection has been broken");
    exit(EXIT_FAILURE);
    return TRUE;
}

static void io_start(FILE *source, pipe_buffer *target, gboolean is_stream) {
    reset_buffer(target);
    GIOChannel *chan = g_io_channel_unix_new(fileno(source));
    GIOStatus status = g_io_channel_set_encoding(chan, NULL, NULL);
    if (status != G_IO_STATUS_NORMAL) {
        printf("set encoding status: %d\n", status);
        exit(EXIT_FAILURE);
    }
    if (is_stream) {
        g_io_channel_set_buffer_size(chan, 64 * 1024);
    }
    target->in_id = g_io_add_watch(chan, G_IO_IN, async_read_chan, target);
    target->hup_id = g_io_add_watch(chan, G_IO_HUP, chan_error_func, target);
    g_io_channel_unref(chan);
}

void io_input_start(FILE *source) { io_start(source, &sync_chan, FALSE); }
void io_stream_start(FILE *source) { io_start(source, &stream_chan, TRUE); }

void io_stop(pipe_buffer *target) {
    g_source_remove(target->hup_id);
    g_source_remove(target->in_id);
    target->in_id = 0;
}

gboolean io_exited() { return sync_chan.in_id == 0 && stream_chan.in_id == 0; }
