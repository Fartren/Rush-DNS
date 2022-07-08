#include "dns_reader.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../errors.h"

struct dns_query no_error =
{
    .header =
    {
        .qr = 1,
        .opcode = 0,
        .aa = 0,
        .tc = 0,
        .rd = 0,
        .ra = 0,
        .z = 0,
        .rcode = 0,
    },
};
struct dns_query form_error =
{
    .header =
    {
        .qr = 1,
        .opcode = 0,
        .aa = 0,
        .tc = 0,
        .rd = 0,
        .ra = 0,
        .z = 0,
        .rcode = 1,
    },
};

struct dns_query server_failure =
{
    .header =
    {
        .qr = 1,
        .opcode = 0,
        .aa = 0,
        .tc = 0,
        .rd = 0,
        .ra = 0,
        .z = 0,
        .rcode = 2,
    },
};

struct dns_query nxdomain =
{
    .header =
    {
        .qr = 1,
        .opcode = 0,
        .aa = 0,
        .tc = 0,
        .rd = 0,
        .ra = 0,
        .z = 0,
        .rcode = 3,
    },
};

struct dns_query not_imp =
{
    .header =
    {
        .qr = 1,
        .opcode = 0,
        .aa = 0,
        .tc = 0,
        .rd = 0,
        .ra = 0,
        .z = 0,
        .rcode = 4,
    },
};

struct dns_query refued =
{
    .header =
    {
        .qr = 1,
        .opcode = 0,
        .aa = 0,
        .tc = 0,
        .rd = 0,
        .ra = 0,
        .z = 0,
        .rcode = 5,
    },
};

void invert_repr(dns_query_header *header)
{
    uint16_t *network_repr = (uint16_t *)header;
    for (unsigned int i = 0; i < sizeof(*header) / 2; ++i)
    {
        network_repr[i] = htons(network_repr[i]);
    }
}

struct dns_query *dns_query_parse(const uint8_t *buffer, size_t len,
                                  dns_rcode *response)
{
    if (sizeof(struct dns_query_header) >= len)
    {
        *response = FORM_ERROR;
        return NULL;
    }
    dns_query *query = malloc(sizeof(dns_query));
    if (!query)
        warn("A query was not successfully allocated\n");

    // Filling header:
    memcpy(&query->header, buffer, sizeof(dns_query_header));
    invert_repr(&query->header);
    buffer = buffer + sizeof(dns_query_header);
    len -= sizeof(dns_query_header);

    // Questions
    if (query->header.qdcount == 0)
    {
        query->questions = NULL;
    }
    else
    {
        query->questions =
            malloc(sizeof(dns_query_question) * query->header.qdcount);
        if (query->questions == NULL)
        {
            dns_query_free(query);
            *response = SERVER_FAILURE;
            return NULL;
        }
    }
    for (uint16_t i = 0; i < query->header.qdcount; ++i)
    {
        query->questions[i].qname = buffer;
        while (len >= 1) // While there is something to read.
        {
            if (buffer[0] == 0)
            {
                len -= 1;
                buffer += 1;
                break; // end.
            }
            if (buffer[0] + 1 < len)
            {
                len -= buffer[0] + 1;
                buffer += buffer[0] + 1;
            }
            else // error:
            {
                dns_query_free(query);
                *response = FORM_ERROR;
                return NULL;
            }
        }
        if (len < 4)
        { // Not enough for QTYPE and QCLASS
            dns_query_free(query);
            *response = FORM_ERROR;
            return NULL;
        }
        char byte_2[2] = { buffer[0], buffer[1] };
        query->questions[i].qtype = ntohs(*(uint16_t *)byte_2);
        len -= 2;
        buffer += 2;
        printf("type inside: %hu\n", query->questions[i].qtype);
        byte_2[0] = buffer[0];
        byte_2[1] = buffer[1];
        query->questions[i].qclass = ntohs(*(uint16_t *)byte_2);
        len -= 2;
        buffer += 2;
    }

    // TODO(elie): lire RR record

    // if (len != 0)
    // {
    //     warn("Too many data left %zu in dns_query\n", len);
    //     dns_query_free(query);
    //     *response = FORM_ERROR;
    //     return NULL;
    // }

    return query;
}

// Freeing the whole dns_query
void dns_query_free(struct dns_query *query)
{
    if (query->questions != NULL)
    {
        free(query->questions);
    }
    free(query);
}
