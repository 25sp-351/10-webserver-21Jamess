#include "server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "handler.h"

#define DEFAULT_PORT 80

static void *connection_thread(void *arg);

void start_server(int port) {
    int listen_fd;
    struct sockaddr_in addr;
    int optval = 1;

    listen_fd  = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 128) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, connection_thread,
                           (void *)(intptr_t)client_fd) != 0) {
            perror("pthread_create");
            close(client_fd);
        } else {
            pthread_detach(tid);
        }
    }
}

static void *connection_thread(void *arg) {
    int fd = (int)(intptr_t)arg;
    handle_client(fd);
    close(fd);
    return NULL;
}
