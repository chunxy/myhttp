#ifndef RESPONSE_H_
#define RESPONSE_H_

#define WRITER_BUFSIZE 1024
typedef struct http_header http_header;
typedef struct http_response http_response;
typedef struct http_writer http_writer;

struct http_header {
    const char* key;
    const char* val;
    http_header* next;
};

struct http_response {
    http_header* headers;
    void* body;
    int content_length;
    int status;

    char* buf;
    int size;
    int anchor;
    int capacity;
    int* memused;
};

void http_response_init(http_response* response);

void http_response_status(http_response* response, int status);

void http_response_body(http_response* response, const char* body);

void http_response_header(http_response* response, const char* key, const char* val);

void http_response_buffer_headers(http_response* http_response);

void http_response_printf(http_response* response, const char* fmt, ...);

#endif