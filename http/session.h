#ifndef SESSION_H_
#define SESSION_H_

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "response.h"
#include "server.h"

#define BUFSIZE 8192
#define MAX_URL 1024
#define HTTP_TIMEOUT 20

typedef struct http_handler http_handler;
typedef struct http_stream http_stream;
typedef struct http_session http_session;

struct http_handler {
    void* (*event_handler)(void*);
    void* (*timer_handler)(void*);
};

struct http_stream {
    char* buf;
    int total;
    // int anchor;
    int length;
    int capacity;
};

enum http_state {
    HTTP_STATE_INIT,
    HTTP_STATE_READ,
    HTTP_STATE_WRITE,
    HTTP_STATE_ERROR,
    HTTP_STATE_SEMI_CLOSE,
    HTTP_STATE_END
};

enum http_error {
    HTTP_ERROR_BAD_REQUEST,
    HTTP_ERROR_NOT_IMPLEMENTED,
};

struct http_session {
    http_handler handler;
    http_server* server;
    http_parser parser;
    http_stream stream;
    enum http_state state;
    enum http_error error;
    int timeout;
    int timer;
    // int events;
    int socket;
    char* url;
    int urllen;
    struct http_parser_url urltok;
    http_response response;
};

http_session* http_session_init(http_server* server, int socket, int timer);

int http_session_loop_modify(http_session* session, int events);

void http_session_end_session(http_session* session);

#endif