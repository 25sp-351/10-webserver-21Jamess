#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int read_http_request(int fd, HttpRequest *req) {
    char buffer[1024];
    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
    if (n <= 0)
        return -1;
    buffer[n] = '\0';

    char *eol = strstr(buffer, "\r\n");
    if (!eol)
        return -1;
    *eol = '\0';

    if (sscanf(buffer, "%7s %255s %15s", req->method, req->path,
               req->version) != 3) {
        return -1;
    }

    return 0;
}

int send_http_response(int fd, const HttpResponse *res) {
    char header[1024];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %zu\r\n"
                        "\r\n",
                        res->status_code, res->status_text, res->content_type,
                        res->body_length);

    write(fd, header, hlen);
    if (res->body_length > 0 && res->body)
        write(fd, res->body, res->body_length);

    return 0;
}
