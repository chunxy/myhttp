#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "http/server.h"

int main(int argc, char** argv)
{
    signal(SIGPIPE, SIG_IGN);
    http_server* server = http_server_init(argv[1]);
    http_server_start(server);

    return 0;
}