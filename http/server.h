#ifndef SERVER_H_
#define SERVER_H_

#include <netinet/in.h>

#include "../threadpool.h"

#define MAX_EVENTQ 8192
#define MAX_LISTENQ 1024
#define MAX_MEM 91920000

typedef struct http_server http_server;

struct http_server {
    void* (*handler)(void*);
    int socket;
    int loop;
    int timer;
    int memused;
    scheduler* sche;
    struct sockaddr_in addr;
};

http_server* http_server_init(char* port);

//int http_server_validate();

int http_server_start(http_server* server);

#endif