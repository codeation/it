#include "terminal.h"
#include <fcntl.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_STREAM_PATH "/tmp/it_fifo_stream_"
#define FIFO_INPUT_PATH "/tmp/it_fifo_input_"
#define FIFO_OUTPUT_PATH "/tmp/it_fifo_output_"
#define FIFO_EVENT_PATH "/tmp/it_fifo_event_"

static FILE *handle_stream = NULL;
static FILE *handle_input = NULL;
static FILE *handle_output = NULL;
static FILE *handle_event = NULL;

static FILE *pipe_open(char *name, char *suffix, char *filemode) {
    GString *buff = g_string_new(NULL);
    g_string_printf(buff, "%s%s", name, suffix);
    FILE *handle = fopen(buff->str, filemode);
    if (handle == NULL) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }
    g_string_free(buff, TRUE);
    return handle;
}

void pipe_init(char *pipe_suffix, GIOFunc func) {
    handle_output = pipe_open(FIFO_OUTPUT_PATH, pipe_suffix, "w");
    handle_event = pipe_open(FIFO_EVENT_PATH, pipe_suffix, "w");
    handle_input = pipe_open(FIFO_INPUT_PATH, pipe_suffix, "r");
    handle_stream = pipe_open(FIFO_STREAM_PATH, pipe_suffix, "r");
    g_io_add_watch(g_io_channel_unix_new(fileno(handle_input)), G_IO_IN, func, io_input());
    g_io_add_watch(g_io_channel_unix_new(fileno(handle_stream)), G_IO_IN, func, io_stream());
}

void pipe_done() {
    if (fclose(handle_input) != 0) {
        perror("pipe close failed (i)");
    }
    if (fclose(handle_stream) != 0) {
        perror("pipe close failed (s)");
    }
    if (fclose(handle_output) != 0) {
        perror("pipe close failed (o)");
    }
    if (fclose(handle_event) != 0) {
        perror("pipe close failed (e)");
    }
}

void pipe_output_write(void *data, int length) {
    if (fwrite(data, length, 1, handle_output) == 0) {
        perror("pipe write (o)");
    }
}

void pipe_output_flush() { fflush(handle_output); }

void pipe_event_write(void *data, int length) {
    if (fwrite(data, length, 1, handle_event) == 0) {
        perror("pipe write (e)");
    }
}

void pipe_event_flush() { fflush(handle_event); }
