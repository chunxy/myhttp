#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "net.h"

int net_open_listenfd(char* port, size_t qsize)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, rc, optval = 1, ret;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;

    if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
        fprintf(stderr, "net_open_listenfd getaddrinfo failed (%s): %s", port, gai_strerror(rc));
        return -2;
    }

    for (p = listp; p != NULL; p = p->ai_next) {
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        if (close(listenfd) < 0) {
            fprintf(stderr, "net_open_listenfd close failed: %s", strerror(errno));
            ret = -1;
            goto end;
        }
    }

    if (!p || listen(listenfd, qsize) < 0) {
        fprintf(stderr, "net_open_listenfd listen failed: %s", strerror(errno));
        close(listenfd);
        ret = -1;
    } else
        ret = listenfd;

end:
    freeaddrinfo(listp);
    return ret;
}

int net_open_clientfd(char* hostname, char* port)
{
    struct addrinfo hints, *listp, *p;
    int clientfd, rc, ret;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;

    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) < 0) {
        fprintf(stderr, "net_open_clientfd getaddrinfo failed (%s:%s): %s", hostname, port, gai_strerror(rc));
        ret = -2;
        goto end;
    }

    for (p = listp; p != NULL; p = p->ai_next) {
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;

        if (connect(clientfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        if (close(clientfd) < 0) {
            fprintf(stderr, "net_open_clientfd close failed: %s", strerror(errno));
            ret = -1;
            goto end;
        }
    }

    if (!p)
        ret = -1;
    else
        ret = clientfd;

end:
    freeaddrinfo(listp);
    return ret;
}

int net_accept(int listenfd, struct sockaddr* addr, socklen_t* addrlen)
{
    int clientfd = accept(listenfd, addr, addrlen);
    if (clientfd < 0)
        fprintf(stderr, "net_accept accpet failed: %s", strerror(errno));

    return clientfd;
}