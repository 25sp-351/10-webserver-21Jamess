#include "handler.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "http.h"

static void handle_static(const char *path, HttpResponse *res);
static void handle_calc(const char *path, HttpResponse *res);
static void handle_sleep(const char *path, HttpResponse *res);

void handle_client(int client_fd) {
    HttpRequest req;
    HttpResponse res;

    while (read_http_request(client_fd, &req) == 0) {
        if (strcmp(req.method, "GET") != 0) {
            res.status_code = 405;
            strcpy(res.status_text, "Method Not Allowed");
            strcpy(res.content_type, "text/plain");
            res.body_length = strlen("405 Method Not Allowed");
            res.body        = strdup("405 Method Not Allowed");
            send_http_response(client_fd, &res);
            free(res.body);
            break;
        }

        if (strncmp(req.path, "/static/", 8) == 0) {
            handle_static(req.path + 7, &res);
        } else if (strncmp(req.path, "/calc/", 6) == 0) {
            handle_calc(req.path + 5, &res);
        } else if (strncmp(req.path, "/sleep/", 7) == 0) {
            handle_sleep(req.path + 6, &res);
        } else {
            res.status_code = 404;
            strcpy(res.status_text, "Not Found");
            strcpy(res.content_type, "text/plain");
            res.body_length = strlen("404 Not Found");
            res.body        = strdup("404 Not Found");
        }

        send_http_response(client_fd, &res);
        free(res.body);
    }
}

static void handle_static(const char *path, HttpResponse *res) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "./static%s", path);

    int fd = open(full_path, O_RDONLY);
    if (fd < 0) {
        res->status_code = 404;
        strcpy(res->status_text, "Not Found");
        strcpy(res->content_type, "text/plain");
        res->body_length = strlen("404 Not Found");
        res->body        = strdup("404 Not Found");
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        res->status_code = 500;
        strcpy(res->status_text, "Internal Server Error");
        strcpy(res->content_type, "text/plain");
        res->body_length = strlen("500 Internal Server Error");
        res->body        = strdup("500 Internal Server Error");
        return;
    }

    size_t size      = st.st_size;
    res->body_length = size;
    res->body        = malloc(size);
    read(fd, res->body, size);
    close(fd);

    res->status_code = 200;
    strcpy(res->status_text, "OK");
    if (strstr(path, ".html"))
        strcpy(res->content_type, "text/html");
    else if (strstr(path, ".png"))
        strcpy(res->content_type, "image/png");
    else if (strstr(path, ".jpg") || strstr(path, ".jpeg"))
        strcpy(res->content_type, "image/jpeg");
    else
        strcpy(res->content_type, "application/octet-stream");
}

static void handle_calc(const char *path, HttpResponse *res) {
    char op[8];
    long a;
    long b;
    if (sscanf(path, "/%7[^/]/%ld/%ld", op, &a, &b) != 3) {
        res->status_code = 400;
        strcpy(res->status_text, "Bad Request");
        strcpy(res->content_type, "text/plain");
        res->body_length = strlen("400 Bad Request");
        res->body        = strdup("400 Bad Request");
        return;
    }

    long result;
    if (strcmp(op, "add") == 0) {
        result = a + b;
    } else if (strcmp(op, "mul") == 0) {
        result = a * b;
    } else if (strcmp(op, "div") == 0) {
        if (b == 0) {
            res->status_code = 400;
            strcpy(res->status_text, "Bad Request");
            strcpy(res->content_type, "text/plain");
            res->body_length = strlen("400 Division by zero");
            res->body        = strdup("400 Division by zero");
            return;
        }
        result = a / b;
    } else {
        res->status_code = 404;
        strcpy(res->status_text, "Not Found");
        strcpy(res->content_type, "text/plain");
        res->body_length = strlen("404 Not Found");
        res->body        = strdup("404 Not Found");
        return;
    }

    char buf[64];
    int len =
        snprintf(buf, sizeof(buf),
                 "<html><body><h1>Result: %ld</h1></body></html>", result);

    res->status_code = 200;
    strcpy(res->status_text, "OK");
    strcpy(res->content_type, "text/html");
    res->body_length = len;
    res->body        = strdup(buf);
}

static void handle_sleep(const char *path, HttpResponse *res) {
    long secs;
    if (sscanf(path, "/%ld", &secs) != 1) {
        res->status_code = 400;
        strcpy(res->status_text, "Bad Request");
        strcpy(res->content_type, "text/plain");
        res->body_length = strlen("400 Bad Request");
        res->body        = strdup("400 Bad Request");
        return;
    }

    sleep(secs);

    char buf[64];
    int len          = snprintf(buf, sizeof(buf), "Slept %ld seconds\n", secs);

    res->status_code = 200;
    strcpy(res->status_text, "OK");
    strcpy(res->content_type, "text/plain");
    res->body_length = len;
    res->body        = strdup(buf);
}
