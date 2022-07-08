#ifndef HASHMAP_H_
#define HASHMAP_H_

#include "node.h"

typedef struct hashmap
{
    size_t size;
    bool defined_soa; // Boolean used to check if the JSON file has one SOA
    zone_node **table;

} hashmap;

hashmap hashmap_init(size_t size);
void hashmap_free(hashmap hashmap);
void hashmap_insert(hashmap hashmap, zone_node *node);
void hashmap_prettyprint(hashmap hashmap);
dns_entry *hashmap_get_entry(hashmap hashmap, const char *ndd, IANA code);
zone_node *hashmap_try_create_zone(hashmap hashmap, const char *ndd);

extern hashmap *map;

#endif /* !HASHMAP_H_ */
