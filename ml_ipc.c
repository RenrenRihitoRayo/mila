#pragma once
#include "mipc.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define MIPC_PREFIX "/tmp/mipc-"

static void make_path(char *path, const char *name) {
    strcpy(path, MIPC_PREFIX);
    strcat(path, name);
    strcat(path, ".sock");
}

static int read_all(int fd, void *buf, size_t len) {
    char *p = buf;

    while (len) {
        ssize_t n = read(fd, p, len);
        if (n <= 0)
            return -1;

        p += n;
        len -= n;
    }

    return 0;
}

static int write_all(int fd, const void *buf, size_t len) {
    const char *p = buf;

    while (len) {
        ssize_t n = write(fd, p, len);
        if (n <= 0)
            return -1;

        p += n;
        len -= n;
    }

    return 0;
}

int mipc_server_open(mipc_server *server, const char *name) {
    char path[108];
    struct sockaddr_un addr;

    make_path(path, name);

    server->fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server->fd < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    unlink(path);

    if (bind(server->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        return -1;

    if (listen(server->fd, 5) < 0)
        return -1;

    return 0;
}

int mipc_server_accept(mipc_server *server) {
    return accept(server->fd, NULL, NULL);
}

void mipc_server_close(mipc_server *server) { close(server->fd); }

int mipc_client_connect(mipc_client *client, const char *name) {
    char path[108];
    struct sockaddr_un addr;

    make_path(path, name);

    client->fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client->fd < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);

    return connect(client->fd, (struct sockaddr *)&addr, sizeof(addr));
}

void mipc_client_close(mipc_client *client) { close(client->fd); }

int mipc_send(int fd, const void *data, size_t size) {
    uint32_t len = (uint32_t)size;

    if (write_all(fd, &len, sizeof(len)))
        return -1;

    if (write_all(fd, data, size))
        return -1;

    return 0;
}

int mipc_recv(int fd, void **data) {
    uint32_t len;
    char *buf;

    if (read_all(fd, &len, sizeof(len)))
        return -1;

    buf = malloc(len + 1);
    if (!buf)
        return -1;

    if (read_all(fd, buf, len)) {
        free(buf);
        return -1;
    }

    buf[len] = '\0';

    *data = buf;

    return (int)len;
}
