#include <sys/socket.h>

int net_open_listenfd(char* port, size_t qsize);

int net_open_clientfd(char* hostname, char* port);

int net_accept(int listenfd, struct sockaddr* addr, socklen_t* addrlen);