#include "terminal.h"
#include <gtk/gtk.h>
#include <stdint.h>
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
} pipe_buffer;

static pipe_buffer sync_chan, stream_chan;

static void reset_buffer(pipe_buffer *target);

pipe_buffer *io_input() {
    reset_buffer(&sync_chan);
    return &sync_chan;
}

pipe_buffer *io_stream() {
    reset_buffer(&stream_chan);
    return &stream_chan;
}

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
    // read data into buffer
    int len = read(g_io_channel_unix_get_fd(source), target->buffer, target->size);
    if (len <= 0) {
        perror("read error");
        exit(EXIT_FAILURE);
    } else if (len < target->size) {
        // shift
        target->buffer += len;
        target->size -= len;
    } else {
        // call next func after getting data
        target->next_func(target);
    }
    return TRUE;
}
