#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_

#include <sys/epoll.h>

int el_init(size_t size);

int el_register(int efd, int events, int fd, void* data, int modify);

int el_deregister(int efd, int fd);

int el_wait(int efd, struct epoll_event* evs, size_t maxevents);

#endif