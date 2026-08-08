#ifndef ML_IPC_H
#define ML_IPC_H

#include <stddef.h>

typedef struct {
    int fd;
} ml_ipc_server;

typedef struct {
    int fd;
} ml_ipc_client;

/* Server */
int ml_ipc_server_open(mipc_server *server, const char *name);
int ml_ipc_server_accept(mipc_server *server);
void ml_ipc_server_close(mipc_server *server);

/* Client */
int ml_ipc_client_connect(mipc_client *client, const char *name);
void ml_ipc_client_close(mipc_client *client);

/* Communication */
int ml_ipc_send(int fd, const void *data, size_t size);
int ml_ipc_recv(int fd, void **data); /* mallocs buffer, caller frees */

}

#endif
