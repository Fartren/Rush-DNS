#include "dns_message.h"

#include <stdlib.h>
#include <string.h>

#include "../parsing/hashmap.h"

// Calculate size
static size_t dns_label_size(const uint8_t *name)
{
    size_t ret = 0;

    while (name[0] != 0)
    {
        ret += name[0];
        name += name[0];
    }

    return ret;
}

static size_t dns_rr_record_size(struct dns_rr_record *record)
{
    return dns_label_size(record->name) + record->rdlength + sizeof(*record)
        - sizeof(record->name) - sizeof(record->rdata);
}

static void print_hex(const char *ndd)
{
    while (*ndd != 0)
    {
        if ('a' <= *ndd && *ndd <= 'z')
        {
            printf("%c", *ndd);
        }
        else
        {
            printf("%hhx", *ndd);
        }
        ++ndd;
    }
    printf("\n");
}

// Actually write
static uint8_t *dns_rr_record_write(uint8_t *buffer,
                                    const dns_rr_record *record)
{
    size_t writen_size = dns_label_size(record->name);
    memcpy(buffer, record->name, writen_size);
    buffer += writen_size;
    writen_size =
        sizeof(*record) - sizeof(record->name) - sizeof(record->rdata);
    memcpy(buffer, &record->type, writen_size);
    buffer += writen_size;
    memcpy(buffer, record->rdata, record->rdlength);
    return buffer + record->rdlength;
}

uint8_t *dns_message_write(struct dns_query *query, size_t *len_response,
                           dns_rcode rcode)
{
    struct dns_query response;
    if (rcode == FORM_ERROR)
    {
        response = form_error;
        response.questions = query->questions;
    }
    else if (rcode == SERVER_FAILURE)
    {
        response = server_failure;
        response.questions = query->questions;
    }
    else if (rcode == NO_ERROR)
    {
        response = no_error;
        response.header.qdcount = query->header.qdcount;
        response.questions = query->questions;

        dns_entry *e =
            hashmap_get_entry(*map, (const char *)query->questions->qname,
                              query->questions->qtype);
        // NAME unknown
        if (e == NULL)
        {
            response = nxdomain;
            response.questions = query->questions;
            response.rr_additionnals = query->rr_additionnals;
        }
        // Type not found
        else if (e->size == 0)
        {
            response = not_imp;
            response.questions = query->questions;
            response.rr_additionnals = query->rr_additionnals;
        }
        else
        {
            printf("Size good\n");
            printf("field: %s, ttl: %zu\n", (const uint8_t *)e->data->field,
                   e->data->TTL);
            if (query->header.arcount != 0)
            {
                response.rr_additionnals = query->rr_additionnals;
            }
            size_t rr_size = sizeof(*query->rr_answers)
                - sizeof(query->rr_answers->name)
                - sizeof(query->rr_answers->rdata) + strlen(e->data->field)
                + strlen((const char *)query->questions[0].qname);
            response.header.ancount = 1;
            printf("rr_size: %zu\n", rr_size);
            response.rr_answers = malloc(rr_size);
            if (query->rr_answers == NULL)
            {
                fprintf(stderr, "OOM\n");
                exit(42);
            }
            response.rr_answers[0].name = query->questions[0].qname;
            printf("rdata: %.*s\n", response.rr_answers[0].rdlength,
                   response.rr_answers[0].rdata);
            printf("name: %s\n", e->data->field);
            print_hex((const char *)query->questions[0].qname);
            response.rr_answers[0].rdata = (const uint8_t *)e->data->field;
            response.rr_answers[0].ttl = e->data->TTL;
            response.rr_answers[0].rdlength = strlen(e->data->field);
        }
    }
    response.header.id = query->header.id;
    query = &response;
    // first we compute size
    *len_response = sizeof(struct dns_query_header);
    for (size_t i = 0; i < query->header.ancount; ++i)
    {
        *len_response += dns_rr_record_size(query->rr_answers + i);
    }
    for (size_t i = 0; i < query->header.arcount; ++i)
    {
        *len_response += dns_rr_record_size(query->rr_additionnals + i);
    }
    // then we alloc one time
    uint8_t *ret = malloc(*len_response);
    size_t curr = sizeof(struct dns_query_header);
    memcpy(ret, query, sizeof(struct dns_query_header));
    for (size_t i = 0; i < query->header.ancount; ++i)
    {
        dns_rr_record_write(ret + curr, query->rr_answers + i);
        curr += dns_rr_record_size(query->rr_answers + i);
    }
    for (size_t i = 0; i < query->header.arcount; ++i)
    {
        dns_rr_record_write(ret + curr, query->rr_additionnals + i);
        curr += dns_rr_record_size(query->rr_additionnals + i);
    }

    return ret;
}
