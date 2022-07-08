#ifndef DNS_READER_H
#define DNS_READER_H

#include <stddef.h>
#include <stdint.h>

typedef struct dns_query_header
{
    uint16_t id;
    uint8_t qr : 1;
    uint8_t opcode : 4;
    uint8_t aa : 1;
    uint8_t tc : 1;
    uint8_t rd : 1;
    uint8_t ra : 1;
    uint8_t z : 3;
    uint8_t rcode : 4;
    uint16_t qdcount, ancount, nscount, arcount;
} dns_query_header;

typedef enum dns_rcode
{
    NO_ERROR = 0,
    FORM_ERROR = 1,
    SERVER_FAILURE = 2,
    NXDOMAIN = 3,
    NOT_IMP = 4,
    REFUSED = 5
} dns_rcode;

typedef struct dns_query_question
{
    const uint8_t *qname;
    uint16_t qtype;
    uint16_t qclass;
} dns_query_question;

typedef struct dns_rr_record
{
    const uint8_t *name;
    uint16_t type, class, ttl, rdlength;
    const uint8_t *rdata;
} dns_rr_record;

typedef struct dns_query
{
    dns_query_header header;
    dns_query_question *questions;
    dns_rr_record *rr_answers, *rr_authorities, *rr_additionnals;
} dns_query;

struct dns_query *dns_query_parse(const uint8_t *buffer, size_t len,
                                  dns_rcode *response);

extern struct dns_query no_error;
extern struct dns_query form_error;
extern struct dns_query server_failure;
extern struct dns_query nxdomain;
extern struct dns_query not_imp;
extern struct dns_query refused;
void dns_query_free(struct dns_query *query);
#endif /* !DNS_READER_H */
