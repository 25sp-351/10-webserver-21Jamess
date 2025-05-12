#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

static void print_usage(const char *prog);

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    int opt;

    while ((opt = getopt(argc, argv, "p:h")) != -1) {
        switch (opt) {
            case 'p':
                port = strtol(optarg, NULL, 10);
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    start_server(port);
    return 0;
}

static void print_usage(const char *prog) {
    printf("Usage: %s [-p port]\n", prog);
}
