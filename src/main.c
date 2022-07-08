#include <ctype.h>
#include <getopt.h>
#include <json-c/json.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "errors.h"
#include "parsing/dns_reader.h"
#include "parsing/hashmap.h"
#include "parsing/node.h"
#include "parsing/parse_json.h"
#include "server/ip_tools.h"
#include "server/server.h"

server_info *si;
hashmap *map;
atomic_uint counter = ATOMIC_VAR_INIT(0);

void INThandler(int sig)
{
    signal(sig, SIG_IGN);
    counter = 1;
    server_free(si);
    hashmap_free(*map);
    exit(0);
    signal(SIGINT, INThandler);
}

void printHelper(void)
{
    printf("---Welcome To DNS Server---\n");
    printf("Usage: dsnd [OPTION]... [FILE]...\n");
    printf("Resolve Domain Name\n");
    printf("-f [FILE]       Use config file\n");
    printf("-c              Only check if the config file is valid\n");
    printf("-h              Display helper\n");
    printf("-4 ipv4:port    Configure server IPv4\n");
    printf("-6 [ipv6]:port  Configure server IPv6\n");
}

int main(int argc, char **argv)
{
    // ======= Arg Parsing ============
    int opt = 0;
    // bool check = false;
    // char *file = NULL;
    int ipv4 = 0;
    int ipv6 = 0;
    signal(SIGINT, INThandler);
    init_nodes();
    // Arg Parsing
    bool check = false;
    char *file = NULL;
    server_info *si = NULL;
    // char response[] =
    //     "\x10\xc4\x01\x20\x00\x01\x00\x00\x00\x00\x00\x01\x07\x65"
    //     "\x78\x61\x6d\x70\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\x00"
    //     "\x00\x29\x10\x00\x00\x00\x00\x00\x00\x0c\x00\x0a\x00\x08\x4d\xea"
    //     "\x85\xe7\x72\xd1\x1d\x8f";
    // ;
    // dns_rcode rcode;
    // struct dns_query *query =
    //    dns_query_parse((const uint8_t *)response, 0x34, &rcode);
    // printf("%hu\n", query->questions->qtype);
    // exit(0);

    if ((si = server_alloc()) == NULL)
    {
        error("Server Memory Allocation\n");
    }

    while ((opt = getopt(argc, argv, "cf:h:4:6:")) != -1)
    {
        switch (opt)
        {
        case 'c':
            check = true;
            break;
        case 'f':
            // Load Json File
            file = optarg;
            break;
        case '4':
            ipv4_converter(si, optarg);
            if (si->addrlen == 0)
            {
                server_free(si);
                fprintf(stderr, "Bad IPv4\n");
                return -1;
            }
            ipv4++;
            break;
        case '6':
            ipv6_converter(si, optarg);
            if (si->addrlen6 == 0)
            {
                server_free(si);
                fprintf(stderr, "Bad IPv6\n");
                return -1;
            }
            ipv6++;
            break;
        case 'h':
            printHelper();
            return 0;
            break;
        default:
            printHelper();
            server_free(si);
            return 0;
        }
    }
    // ========= Json Parsing ==========
    //
    // if (!file)
    // {
    //     fprintf(stderr, "Error: invalid argument for option -f\n");
    //     server_free(si);
    //     exit(1);
    // }
    // Json Parsing

    if (!file)
    {
        server_free(si);
        error("Invalid argument for option -f\n");
    }

    struct hashmap hashmap = hashmap_init(128);
    map = &hashmap;

    json_object *parsed_json = parse_json(file, &hashmap);

    map = &hashmap;
    if (check)
    {
        server_free(si);
        hashmap_free(*map);
        json_object_put(parsed_json);
        return 0;
    }

    // ======== Server Init ===========
    if (ipv4 == 0 && ipv6 == 0)
    {
        server_free(si);
        hashmap_free(*map);
        json_object_put(parsed_json);
        error("No address:port to bind");
    }
    si->flags = 0;
    server(si);

    // freeing json (and also a part of hashmap ...)
    json_object_put(parsed_json);
    return 0;
}
