#include "terminal.h"
#include <gtk/gtk.h>

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

void pipe_init(char *pipe_suffix) {
    handle_output = pipe_open(FIFO_OUTPUT_PATH, pipe_suffix, "w");
    handle_event = pipe_open(FIFO_EVENT_PATH, pipe_suffix, "w");
    handle_input = pipe_open(FIFO_INPUT_PATH, pipe_suffix, "r");
    handle_stream = pipe_open(FIFO_STREAM_PATH, pipe_suffix, "r");
    io_input_start(handle_input);
    io_stream_start(handle_stream);
}

void pipe_done() {
    if (fclose(handle_input) != 0) {
        perror("pipe close failed (i)");
        exit(EXIT_FAILURE);
    }
    if (fclose(handle_stream) != 0) {
        perror("pipe close failed (s)");
        exit(EXIT_FAILURE);
    }
    if (fclose(handle_output) != 0) {
        perror("pipe close failed (o)");
        exit(EXIT_FAILURE);
    }
    if (fclose(handle_event) != 0) {
        perror("pipe close failed (e)");
        exit(EXIT_FAILURE);
    }
}

void pipe_output_write(const void *data, const int length) {
    if (fwrite(data, length, 1, handle_output) == 0) {
        perror("pipe write (o)");
        exit(EXIT_FAILURE);
    }
}

void pipe_output_write_string(const char *data) {
    int32_t length = strlen(data);
    pipe_output_write(&length, sizeof length);
    pipe_output_write(data, length);
}

void pipe_output_flush() { fflush(handle_output); }

void pipe_event_write(const void *data, const int length) {
    if (fwrite(data, length, 1, handle_event) == 0) {
        perror("pipe write (e)");
        exit(EXIT_FAILURE);
    }
}

void pipe_event_write_string(const char *data) {
    int32_t length = strlen(data);
    pipe_event_write(&length, sizeof length);
    pipe_event_write(data, length);
}

void pipe_event_flush() { fflush(handle_event); }
