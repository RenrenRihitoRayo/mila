#pragma once

#define MILA_PROTO
#include "../mila.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

Value *native_socket(Env *env, int argc, Value **argv) {
    int domain = argc > 0 ? GET_INTEGER(argv[0]) : AF_INET;
    int type = argc > 1 ? GET_INTEGER(argv[1]) : SOCK_STREAM;
    int proto = argc > 2 ? GET_INTEGER(argv[2]) : 0;
    int fd = socket(domain, type, proto);
    return vopaque((void *)(long)fd);
}

Value *native_bind(Env *env, int argc, Value **argv) {
    int fd = GET_INTEGER(argv[0]);
    const char *ip = GET_STRING(argv[1]);
    int port = GET_INTEGER(argv[2]);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ip ? inet_addr(ip) : INADDR_ANY;

    return vbool(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
}

Value *native_listen(Env *env, int argc, Value **argv) {
    int fd = GET_INTEGER(argv[0]);
    int backlog = GET_INTEGER(argv[1]);
    return vbool(listen(fd, backlog) == 0);
}

Value *native_accept(Env *env, int argc, Value **argv) {
    int fd = GET_INTEGER(argv[0]);
    int cfd = accept(fd, NULL, NULL);
    return vopaque((void *)(long)cfd);
}

Value *native_connect(Env *env, int argc, Value **argv) {
    int fd = GET_INTEGER(argv[0]);
    const char *ip = GET_STRING(argv[1]);
    int port = GET_INTEGER(argv[2]);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    return vbool(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
}

Value *native_send(Env *env, int argc, Value **argv) {
    int fd = GET_INTEGER(argv[0]);
    const char *msg = GET_STRING(argv[1]);
    int len = msg ? strlen(msg) : 0;
    return vint(send(fd, msg, len, 0));
}

Value *native_recv(Env *env, int argc, Value **argv) {
    int fd = GET_INTEGER(argv[0]);
    int size = GET_INTEGER(argv[1]);

    char *buf = malloc(size + 1);
    int r = recv(fd, buf, size, 0);

    if (r <= 0) {
        free(buf);
        return vstring_dup("");
    }

    buf[r] = 0;
    return vstring_take(buf);
}

Value *native_close(Env *env, int argc, Value **argv) {
    int fd = GET_INTEGER(argv[0]);
    return vbool(close(fd) == 0);
}

void env_register_socket_ext(Env* g) 
{
    env_register_native(g, "socket.socket", native_socket);
    env_register_native(g, "socket.bind", native_bind);
    env_register_native(g, "socket.listen", native_listen);
    env_register_native(g, "socket.connect", native_connect);
    env_register_native(g, "socket.send", native_send);
    env_register_native(g, "socket.recv", native_recv);
    env_register_native(g, "socket.close", native_close);
}