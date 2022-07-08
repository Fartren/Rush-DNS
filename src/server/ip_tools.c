#define _GNU_SOURCE
#include "ip_tools.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void ipv4_converter(server_info *server, char *ipv4)
{
    char *addr_str = NULL;
    char *port_str = ipv4;
    addr_str = strtok_r(port_str, ":", &port_str);

    printf("Addr: %s  Port: %s\n", addr_str, port_str);
    if (inet_pton(AF_INET, addr_str, &server->addr->sin_addr) != 1)
    {
        server->addrlen = 0;
        return;
    }
    long port = 0;
    if ((port = atoi(port_str)) == 0)
    {
        server->addrlen = 0;
    }
    else if (port <= 0 || port >= 65535)
    {
        server->addrlen = 0;
    }
    else
    {
        server->addr->sin_port = htons(port);
        server->addr->sin_family = AF_INET;
        server->addrlen = 4;
    }
}

void ipv6_converter(server_info *server, char *ipv6)
{
    char *rest = NULL;
    char *port_str = NULL;
    char *addr_str = ipv6;
    strtok_r(addr_str, "]", &port_str);
    addr_str = strtok_r(addr_str, "[", &rest);
    port_str = strtok_r(port_str, ":", &rest);

    if (inet_pton(AF_INET6, addr_str, &server->addr6->sin6_addr) != 1)
    {
        server->addrlen6 = 0;
        return;
    }
    long port = 0;
    if ((port = atol(port_str)) == 0)
    {
        server->addrlen6 = 0;
    }
    else if (port <= 0 || port >= 65535)
    {
        server->addrlen6 = 0;
    }
    else
    {
        server->addr6->sin6_port = htons(port);
        server->addr6->sin6_family = AF_INET6;
        server->addrlen6 = 16;
    }
}
