#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

typedef struct {
    char method[8];
    char path[256];
    char version[16];
} HttpRequest;

typedef struct {
    int status_code;
    char status_text[32];
    char content_type[64];
    char *body;
    size_t body_length;
} HttpResponse;

int read_http_request(int fd, HttpRequest *req);
int send_http_response(int fd, const HttpResponse *res);

#endif
