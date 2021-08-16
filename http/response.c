#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "response.h"

void http_response_init(http_response* response)
{
    memset(response, 0, sizeof(http_response));
    response->status = 200;

    response->buf = calloc(WRITER_BUFSIZE, 1);
    response->capacity = WRITER_BUFSIZE;
    //writer-related initialization
}

void http_response_status(http_response* response, int status)
{
    response->status = status;
}

void http_response_body(http_response* response, const char* body)
{
    response->body = body;
    response->content_length = strlen(body);
}

void http_response_header(http_response* response, const char* key, const char* val)
{
    http_header* header = calloc(1, sizeof(http_header));
    header->key = key;
    header->val = val;
    if (!response->headers)
        response->headers = header;
    else
        response->headers->next = header;
}

void http_response_buffer_headers(http_response* response)
{
    http_response_printf(response, "HTTP/1.1 200 OK\r\n");
    http_header* header = response->headers;
    while (header) {
        http_response_printf(response, "%s: %s\r\n", header->key, header->val);
        header = header->next;
    }
    http_response_printf(response, "%s: %d\r\n", "Content-length", response->content_length);
    http_response_printf(response, "\r\n", 2);
}

void http_response_printf(http_response* response, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    int bytes = vsnprintf(response->buf + response->size, response->capacity - response->size, fmt, va);
    if (bytes + response->size > response->capacity) {
        // *writer->memused -= writer->capacity;
        while (bytes + response->size > response->capacity)
            response->capacity *= 2;
        // *writer->memused += writer->capacity;
        response->buf = (char*)realloc(response->buf, response->capacity);
        // assert(writer->buf != NULL);
        bytes += vsnprintf(response->buf + response->size, response->capacity - response->size, fmt, va);
    }
    response->size += bytes;

    va_end(va);
}