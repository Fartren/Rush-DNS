#include "server.h"

#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../parsing/dns_reader.h"
#include "../parsing/hashmap.h"
#include "../serialization/dns_message.h"

server_info *server_alloc(void)
{
    server_info *si = NULL;

    if ((si = calloc(1, sizeof(server_info))) == NULL)
        return NULL;

    if ((si->addr = calloc(1, sizeof(struct sockaddr_in))) == NULL)
    {
        free(si);
        return NULL;
    }

    if ((si->addr6 = calloc(1, sizeof(struct sockaddr_in6))) == NULL)
    {
        free(si->addr);
        free(si);
        return NULL;
    }
    return si;
}

void server_free(server_info *server_info)
{
    if (server_info != NULL)
    {
        if (server_info->addr != NULL)
        {
            free(server_info->addr);
        }
        if (server_info->addr6 != NULL)
        {
            free(server_info->addr6);
        }
        free(server_info);
    }
}

server_sockets create_sockets(struct server_info *si, int type)
{
    server_sockets sockets = { -1, -1 };
    if (si->addrlen > 0)
    {
        sockets.sock = socket(si->addr->sin_family, type, si->flags);
        if (sockets.sock < 0)
        {
            fprintf(stderr, "Could not create socket\n");
            exit(1);
        }

        if (bind(sockets.sock, (struct sockaddr *)si->addr,
                 sizeof(struct sockaddr_in))
            < 0)
        {
            fprintf(stderr, "Could not bind\n");
            exit(1);
        }
    }
    if (si->addrlen6 > 0)
    {
        sockets.sock6 = socket(si->addr6->sin6_family, type, si->flags);
        if (sockets.sock6 < 0)
        {
            fprintf(stderr, "Could not create socket\n");
            exit(1);
        }

        if (bind(sockets.sock6, (struct sockaddr *)si->addr6,
                 sizeof(struct sockaddr_in6))
            < 0)
        {
            fprintf(stderr, "Could not bind\n");
            exit(1);
        }
    }
    return sockets;
}

int foo = 0;

void *process(void *ptr)
{
    foo++;
    if (foo != 1)
    {
        return NULL;
    }
    if (!ptr)
        pthread_exit(0);
    int *sock = (int *)ptr;

    char request_size[2];
    int nread = 0;
    size_t nwrite = 0;
    char *request = NULL;

    printf("counter: %d\n", counter);
    while (counter == 0)
    {
        printf("count est valide\n");
        //__asm__("int $0x3");
        // read the message from client and copy it in buffer
        nread = read(*sock, request_size, 2);
        if (nread == -1)
        {
            fprintf(stderr, "Failed to read request size: %d\n", nread);
            break;
        }
        uint16_t req_size = ntohs(*(uint16_t *)request_size);
        printf("req_size: %hu\n", req_size);
        request = malloc(req_size * sizeof(char));
        if (request == NULL)
        {
            fprintf(stderr, "Failed to allocate buffer for request\n");
            continue;
        }
        else
        {
            nread = read(*sock, request, req_size);
            if (nread == -1)
            {
                fprintf(stderr, "Failed to read request\n");
                break;
            }
        }

        // Parsing DNS Request
        // struct response_info {
        //  emun response_type;
        //}
        // struct dns_query *query = dns_query_parse((uint8_t *)buf,
        // nread,response));

        // Build Response from DNS query
        // char *response = (char *) write_dns(query, (size_t *) &nread);

        // Test value it should be removed at the end
        dns_rcode rcode = 0;
        struct dns_query *query =
            dns_query_parse((uint8_t *)request, nread, &rcode);
        // Build Response from DNS query
        char *response = (char *)dns_message_write(query, &nwrite, rcode);

        // char response[57] =
        //     "\x00\x35\xb1\x4c\x81\x80\x00\x01\x00\x01\x00\x00\x00\x01\x06\x67"
        //     "\x6f\x6f"
        //     "\x67\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\xc0\x0c\x00\x01"
        //     "\x00\x01\x00\x00\x00\xc1\x00\x04\xac\xd9\x16\x8e\x00\x00\x29\x10"
        //     "\x00\x00\x00\x00\x00\x00\x00";
        // response[2] = request[0];
        // response[3] = request[1];
        // nread = 57;
        ssize_t sended_data = send(*sock, response, nwrite, 0);
        *(uint16_t *)request_size = htons(nwrite);
        send(*sock, request_size, 2, 0);
        printf("sended_data: %zu\n", sended_data);
        if (sended_data != (ssize_t)nwrite)
            fprintf(stderr, "Error sending response: %zu != %zu\n", sended_data,
                    nwrite);
        else
            break;
    }
    if (request != NULL)
        free(request);
    close(*sock);
    pthread_exit(0);
}

void event_loop_tcp(server_sockets sockets)
{
    int sock, sock6;
    pthread_t thread;
    printf("waiting for tcp requests...\n");

    for (;;)
    {
        if (sockets.sock > 0)
        {
            // create socket for the incoming IPV4 connection
            sock = accept(sockets.sock, NULL, NULL);
            if (sock > 0)
            {
                pthread_create(&thread, 0, process, (void *)&sock);
                pthread_detach(thread);
            }
        }
        if (sockets.sock6 > 0)
        {
            // create socket for the incoming IPV6 connection
            sock6 = accept(sockets.sock6, NULL, NULL);
            if (sock6 > 0)
            {
                pthread_create(&thread, 0, process, (void *)&sock6);
                pthread_detach(thread);
            }
        }
    }
}

void event_udp(int sock)
{
    ssize_t nread = 0;
    char buf[BUF_SIZE];
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Reading UDP Datagram from socket
    nread = recvfrom(sock, buf, BUF_SIZE, 0, (struct sockaddr *)&client_addr,
                     &client_addr_len);
    if (nread == -1)
    {
        // Ignore failed request
        return;
    }

    // Parsing DNS Request
    dns_rcode rcode = 0;
    struct dns_query *query = dns_query_parse((uint8_t *)buf, nread, &rcode);
    // Build Response from DNS query
    char *response = (char *)dns_message_write(query, (size_t *)&nread, rcode);

    // Test value it should be removed at the end
    // char response[55] =
    //     "\xb1\x4c\x81\x80\x00\x01\x00\x01\x00\x00\x00\x01\x06\x67\x6f\x6f"
    //     "\x67\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\xc0\x0c\x00\x01"
    //     "\x00\x01\x00\x00\x00\xc1\x00\x04\xac\xd9\x16\x8e\x00\x00\x29\x10"
    //     "\x00\x00\x00\x00\x00\x00\x00";

    // replace transaction id in the hardcoded response
    // response[0] = buf[0];
    // response[1] = buf[1];
    // hardcode send size
    // nread = 55;

    if (sendto(sock, response, nread, 0, (struct sockaddr *)&client_addr,
               client_addr_len)
        != nread)
        fprintf(stderr, "UDP server: Error sending response\n");
}

void *event_loop_udp(void *udp_sockets)
{
    if (!udp_sockets)
    {
        pthread_exit(0);
    }
    server_sockets *sockets = (server_sockets *)udp_sockets;

    printf("waiting for udp requests...\n");

    while (counter == 0)
    {
        event_udp(sockets->sock);
        event_udp(sockets->sock6);
    }
    pthread_exit(0);
}

int server(struct server_info *si)
{
    pthread_t thread;

    // Create connection socket
    server_sockets sockets_tcp = create_sockets(si, SOCK_STREAM);
    server_sockets sockets_udp = create_sockets(si, SOCK_DGRAM);

    // Listen + Bind IPv4 connection
    if (sockets_tcp.sock > 0 && listen(sockets_tcp.sock, MAX_CLIENT) < 0)
    {
        fprintf(stderr, "%c: error: cannot listen on port\n",
                si->addr->sin_port);
        return 1;
    }

    // Listen + Bind IPv6 connection
    if (sockets_tcp.sock6 > 0 && listen(sockets_tcp.sock6, MAX_CLIENT) < 0)
    {
        fprintf(stderr, "%c: error: cannot listen on port\n",
                si->addr6->sin6_port);
        return 1;
    }

    // Start Receiving Request
    pthread_create(&thread, 0, event_loop_udp, (void *)&sockets_udp);
    pthread_detach(thread);
    event_loop_tcp(sockets_tcp);

    // Closing sockets
    if (sockets_tcp.sock >= 0)
        close(sockets_tcp.sock);
    if (sockets_tcp.sock6 >= 0)
        close(sockets_tcp.sock6);
    if (sockets_udp.sock >= 0)
        close(sockets_udp.sock);
    if (sockets_udp.sock6 >= 0)
        close(sockets_udp.sock6);
    return 0;
}
