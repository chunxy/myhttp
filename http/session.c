#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../eventloop.h"
#include "response.h"
#include "server.h"
#include "session.h"

static void http_stream_init(http_stream* stream);
static int http_session_read(http_session* session);
static int http_session_write(http_session* session);
// static int http_session_error(http_session* session);
static void* http_session_event_handler(void* arg);
static void* http_session_timer_handler(void* arg);

static void http_session_reset_timeout(http_session* session, int timeout);

static void http_session_process_request(http_session* session);

static int callback_on_url(http_parser* parser, const char* at, size_t length);
static int callback_on_headers_complete(http_parser* parser);
static int callback_on_message_complete(http_parser* parser);
static http_parser_settings settings = {
    .on_url = callback_on_url,
    .on_headers_complete = callback_on_headers_complete,
    .on_message_complete = callback_on_message_complete,
};

http_session* http_session_init(http_server* server, int socket, int timer)
{
    http_session* session = malloc(sizeof(http_session));
    session->handler = (http_handler){0};
    session->handler.event_handler = http_session_event_handler;
    session->handler.timer_handler = http_session_timer_handler;
    session->server = server;
    http_parser_init(&session->parser, HTTP_REQUEST);
    session->parser.data = session;
    session->stream = (http_stream){0};
    session->state = HTTP_STATE_INIT;
    session->timeout = HTTP_TIMEOUT;
    session->timer = timer;
    session->socket = socket;
    session->urllen = 0;
    http_parser_url_init(&session->urltok);
    http_response_init(&session->response);

    return session;
}

int http_session_loop_modify(http_session* session, int events)
{
    el_register(session->server->loop, events, session->socket, session, 1);
    return 0;
}

void http_session_end_session(http_session* session)
{
    el_deregister(session->server->loop, session->socket);
    el_deregister(session->server->timer, session->timer);
    close(session->socket);
    free(session->stream.buf);
    free(session->url);
    free(session->response.buf);
    free(session);
}

void http_stream_init(http_stream* stream)
{
    stream->buf = (char*)calloc(BUFSIZE, 1);
    stream->capacity = BUFSIZE;
    //assert
}

int http_session_read(http_session* session)
{
    http_stream* stream = &session->stream;
    int nread, total = 0, nparse;
    do {
        nread = read(session->socket, stream->buf + stream->length, stream->capacity - stream->length);
        if (nread > 0) {
            total += nread;
            stream->total += nread;
            stream->length += nread;
        }
    } while ((nread > 0 || errno == EINTR) && stream->length != stream->capacity);

    if (total != 0) {
        nparse = http_parser_execute(&session->parser, &settings, stream->buf, total);
        stream->length -= nparse;
        memmove(stream->buf, stream->buf + nparse, stream->length);
        if (session->parser.http_errno == HPE_PAUSED) {
            http_session_process_request(session);
            session->state = HTTP_STATE_WRITE;
        } else if (session->parser.http_errno != HPE_OK) {
            session->error = HTTP_ERROR_BAD_REQUEST;
            session->state = HTTP_STATE_ERROR;
        }
    } else
        session->state = HTTP_STATE_SEMI_CLOSE;

    if (session->state == HTTP_STATE_WRITE ||
        session->state == HTTP_STATE_ERROR ||
        session->state == HTTP_STATE_SEMI_CLOSE)
        http_session_loop_modify(session, EPOLLOUT | EPOLLONESHOT);
    else
        http_session_loop_modify(session, EPOLLIN | EPOLLONESHOT);

    return total;
}

int http_session_write(http_session* session)
{
    http_response* response = &session->response;
    http_response_buffer_headers(response);

    int nwrite, total = 0;
    do {
        nwrite = write(session->socket, response->buf + response->anchor, response->size - response->anchor);
        if (nwrite > 0) {
            total += nwrite;
            response->anchor += nwrite;
        }
    } while ((nwrite > 0 || errno == EINTR) && response->size > response->anchor);

    write(session->socket, session->response.body, session->response.content_length);
    munmap(session->response.body, session->response.content_length);

    if (total != response->size)
        session->state = HTTP_STATE_END;
    else {
        http_parser_pause(&session->parser, 0);
        http_session_loop_modify(session, EPOLLIN | EPOLLONESHOT);
        session->state = HTTP_STATE_READ;
    }

    return total;
}

void* http_session_event_handler(void* arg)
{
    http_session* session = (http_session*)arg;
    switch (session->state) {
        case HTTP_STATE_INIT:
            if (session->server->memused > MAX_MEM) {
                //error response
                return 0;
            }
            session->state = HTTP_STATE_READ;
            http_stream_init(&session->stream);
            session->url = (char*)calloc(MAX_URL, 1);
        case HTTP_STATE_READ:
            http_session_read(session);
            break;
        case HTTP_STATE_WRITE:
            http_session_write(session);
            break;
        case HTTP_STATE_ERROR:

            break;
        case HTTP_STATE_SEMI_CLOSE:
            session->state = HTTP_STATE_END;
            break;
        default:
            break;
    }

    if (session->state == HTTP_STATE_END)
        http_session_end_session(session);
    else
        http_session_reset_timeout(session, HTTP_TIMEOUT);
}

void* http_session_timer_handler(void* arg)
{
    http_session* session = (http_session*)arg;
    session->timeout -= 1;
    if (session->timeout == 0)
        http_session_end_session(session);
}

void http_session_reset_timeout(http_session* session, int timeout)
{
    session->timeout = timeout;
}

void http_session_process_request(http_session* session)
{
    if (session->parser.method != HTTP_GET) {
        session->error = HTTP_ERROR_NOT_IMPLEMENTED;
        session->state = HTTP_STATE_ERROR;
        return;
    }
    char filename[256] = "./index.html";
    if (session->urltok.field_set & 1 << UF_PATH) {
        int off = session->urltok.field_data[UF_PATH].off;
        int len = session->urltok.field_data[UF_PATH].len;
        if (len > 1) {
            strncpy(filename + 1, session->url + off, len);
            filename[1 + len] = '\0';
        }
    }

    int fd = open(filename, O_RDONLY);
    struct stat filestat;
    fstat(fd, &filestat);
    session->response.body = mmap(0, filestat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    session->response.content_length = filestat.st_size;
    close(fd);
    // sendfile(session->socket, fd, NULL, filestat.st_size);
}

int callback_on_url(http_parser* parser, const char* at, size_t length)
{
    http_session* session = (http_session*)parser->data;
    strncpy(session->url, at, length);
    session->urllen = length;
    http_parser_parse_url(session->url, length, 0, &session->urltok);

    return 0;
}

int callback_on_headers_complete(http_parser* parser)
{
    // http_session* session = (http_session*)parser->data;

    return 0;
}

int callback_on_message_complete(http_parser* parser)
{
    http_parser_pause(parser, 1);
    return 0;
}