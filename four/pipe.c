#include "terminal.h"
#include <gtk/gtk.h>

#define FIFO_STREAM_PATH "/tmp/it_fifo_stream_"
#define FIFO_INPUT_PATH "/tmp/it_fifo_input_"
#define FIFO_OUTPUT_PATH "/tmp/it_fifo_output_"
#define FIFO_EVENT_PATH "/tmp/it_fifo_event_"

static FILE *handle_output = NULL;
static FILE *handle_event = NULL;

static GIOChannel *chan_stream = NULL;
static GIOChannel *chan_input = NULL;

static FILE *pipe_open(char *name, char *suffix, char *filemode) {
    GString *buff = g_string_new(NULL);
    g_string_printf(buff, "%s%s", name, suffix);
    FILE *handle = fopen(buff->str, filemode);
    if (handle == NULL) {
        perror("pipe open failed");
        exit(EXIT_FAILURE);
    }
    g_string_free(buff, TRUE);
    return handle;
}

static void pipe_close(FILE *handle) {
    if (fclose(handle) != 0) {
        perror("pipe close failed");
        exit(EXIT_FAILURE);
    }
}

static GIOChannel *chan_open(char *name, char *suffix, char *filemode) {
    GString *buff = g_string_new(NULL);
    g_string_printf(buff, "%s%s", name, suffix);
    GIOChannel *chan = g_io_channel_new_file(buff->str, filemode, NULL);
    if (chan == NULL) {
        printf("channel open failed\n");
        exit(EXIT_FAILURE);
    }
    g_string_free(buff, TRUE);
    return chan;
}

static void chan_close(GIOChannel *chan) {
    GIOStatus status = g_io_channel_shutdown(chan, FALSE, NULL);
    if (status != G_IO_STATUS_NORMAL) {
        printf("channel shutdown status: %d\n", status);
        exit(EXIT_FAILURE);
    }
    g_io_channel_unref(chan);
}

void pipe_init(char *pipe_suffix) {
    handle_output = pipe_open(FIFO_OUTPUT_PATH, pipe_suffix, "w");
    handle_event = pipe_open(FIFO_EVENT_PATH, pipe_suffix, "w");
    chan_input = chan_open(FIFO_INPUT_PATH, pipe_suffix, "r");
    chan_stream = chan_open(FIFO_STREAM_PATH, pipe_suffix, "r");
    io_input_start(chan_input);
    io_stream_start(chan_stream);
}

void pipe_done() {
    chan_close(chan_input);
    chan_close(chan_stream);
    pipe_close(handle_output);
    pipe_close(handle_event);
}

void pipe_output_write(const void *data, const int length) {
    if (fwrite(data, length, 1, handle_output) == 0) {
        perror("pipe write (o)");
        exit(EXIT_FAILURE);
    }
}

void pipe_output_write_string(const char *data) {
    g_assert(data);
    int32_t length = (int32_t)strlen(data);
    pipe_output_write(&length, sizeof length);
    pipe_output_write(data, length);
}

void pipe_output_flush() { (void)fflush(handle_output); }

void pipe_event_write(const void *data, const int length) {
    if (fwrite(data, length, 1, handle_event) == 0) {
        perror("pipe write (e)");
        exit(EXIT_FAILURE);
    }
}

void pipe_event_write_string(const char *data) {
    g_assert(data);
    int32_t length = (int32_t)strlen(data);
    pipe_event_write(&length, sizeof length);
    pipe_event_write(data, length);
}

void pipe_event_flush() { (void)fflush(handle_event); }
