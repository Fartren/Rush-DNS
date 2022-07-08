#ifndef SERVER_H
#define SERVER_H
#define BUF_SIZE 512
#define MAX_CLIENT 32
#include <netinet/in.h>
#include <stdatomic.h>

extern atomic_uint counter;

typedef struct server_info
{
    int flags;
    socklen_t addrlen;
    socklen_t addrlen6;
    struct sockaddr_in *addr;
    struct sockaddr_in6 *addr6;
} server_info;

typedef struct server_sockets
{
    int sock;
    int sock6;
} server_sockets;

server_info *server_alloc(void);
void server_free(server_info *server_info);
server_sockets create_sockets(struct server_info *si, int type);
void event_loop_tcp(server_sockets sockets);
void *event_loop_udp(void *sockets);
int server(struct server_info *si);

#endif // SERVER_H
