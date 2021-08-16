#include <fcntl.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "../eventloop.h"
#include "../net.h"
#include "../threadpool.h"
#include "server.h"
#include "session.h"

static void* http_server_accept(void* arg);
static void* http_server_time(void* arg);

http_server* http_server_init(char* port)
{
    http_server* server = malloc(sizeof(http_server));
    server->handler = http_server_accept;
    server->socket = net_open_listenfd(port, MAX_LISTENQ);
    fcntl(server->socket, F_SETFL, O_NONBLOCK);
    server->loop = el_init(MAX_EVENTQ);
    server->timer = el_init(MAX_EVENTQ);
    server->sche = tp_init(4);
    server->memused = 0;

    return server;
}

int http_server_start(http_server* server)
{
    el_register(server->loop, EPOLLIN | EPOLLET, server->socket, server, 0);
    // pthread_t timer;
    // pthread_create(&timer, NULL, http_server_time, server);
    // pthread_detach(timer);

    struct epoll_event epoll_evs[1000];
    while (1) {
        int nevents = el_wait(server->loop, epoll_evs, 1000);
        for (int i = 0; i < nevents; i++) {
            http_handler* handler = (http_handler*)epoll_evs[i].data.ptr;
            function func = (function)handler->event_handler;
            tp_submit(server->sche, func, epoll_evs[i].data.ptr);
        }
        usleep(1000);
    }
    return -1;
}

void* http_server_time(void* arg)
{
    http_server* server = (http_server*)arg;

    struct epoll_event epoll_evs[100];
    while (1) {
        int nevents = el_wait(server->timer, epoll_evs, 100);
        for (int i = 0; i < nevents; i++) {
            http_handler* handler = (http_handler*)epoll_evs[i].data.ptr;
            function* func = (function*)handler->timer_handler;
            // tp_submit(server->sche, *func, epoll_evs[i].data.ptr);
        }
    }
}

void* http_server_accept(void* arg)
{
    http_server* server = (http_server*)arg;
    struct sockaddr addr;
    socklen_t addrlen;
    int clientfd, timerfd;

    while ((clientfd = net_accept(server->socket, &addr, &addrlen)) > 0) {
        fcntl(clientfd, F_SETFL, O_NONBLOCK);
        timerfd = timerfd_create(CLOCK_MONOTONIC, 0);

        http_session* session = http_session_init(server, clientfd, timerfd);
        el_register(server->loop, EPOLLIN | EPOLLONESHOT, clientfd, session, 0);

        struct itimerspec ts = {0};
        ts.it_value.tv_sec = 1;
        ts.it_interval.tv_sec = 1;
        timerfd_settime(server->timer, 0, &ts, NULL);
        el_register(server->timer, EPOLLIN | EPOLLET, timerfd, session, 0);
    }
}