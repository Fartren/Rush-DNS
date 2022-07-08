#ifndef DNS_MESSAGE_H
#define DNS_MESSAGE_H

#include "../parsing/dns_reader.h"

// dns_response_type analyse_query(struct dns_query *query);
uint8_t *dns_message_write(struct dns_query *query, size_t *len_response,
                           dns_rcode rcode);

#endif /* DNS_MESSAGE_H */
