#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>

#include "eventloop.h"

int el_init(size_t size)
{
    int efd = epoll_create(size);
    if (efd < 0)
        fprintf(stderr, "el_init epoll_create failed: %s", strerror(errno));

    return efd;
}

int el_register(int efd, int events, int fd, void* data, int modify) 
{
    struct epoll_event epoll_ev = {0, {0}};
    epoll_ev.events = events;
    epoll_ev.data.ptr = data;

    int op = modify ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    if (epoll_ctl(efd, op, fd, &epoll_ev) < 0) {
        fprintf(stderr, "el_register epoll_ctl failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

int el_deregister(int efd, int fd)
{
    struct epoll_event epoll_ev = {0, {0}};
    epoll_ctl(efd, EPOLL_CTL_DEL, fd, &epoll_ev);
    return 0;
}

int el_wait(int efd, struct epoll_event* epoll_evs, size_t maxevents)
{
    int nfd = epoll_wait(efd, epoll_evs, maxevents, 1);
    if (nfd < 0) 
        fprintf(stderr, "el_wait epoll_wait failed: %s", strerror(errno));

    return nfd;
}