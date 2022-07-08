#ifndef NODE_H_
#define NODE_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IANA_CODE_COUNT 6 // Number of IANA code really implemented.
#define DEFAULT_CAPACITY 3 // Default capacity of a vector for an entry type.
#define HASHMAP_SIZE 128U // Hashmap size (also used in modulo hash function).

// Referencing all IANA Code entries from 0 to 255
// Ref: https://fr.wikipedia.org/wiki/Liste_des_enregistrements_DNS
typedef enum IANA
{
    A = 1,
    NS = 2,
    SOA = 6,
    MX = 15,
    TXT = 16,
    AAAA = 28,

    iana_len = 256
} IANA;

int IANA_compress(IANA code);

typedef struct dns_data
{
    size_t TTL;
    char *field;
} dns_data;

// Represents a DNS entry of a certain type (example: 'AAAA')
typedef struct dns_entry
{
    // IANA code; <-- for testing purposes
    size_t size;
    size_t capacity;
    dns_data *data; // TTL and field*
} dns_entry;

typedef struct zone_node
{
    const char *name;
    dns_entry entries[IANA_CODE_COUNT]; //**entries; //entries[IANA_CODE_COUNT];
    struct zone_node *next; // Used for hashmap.
} zone_node;

int init_nodes(void); // Function to call once at the beginning of the program.
zone_node *create_node(const char *name); // Used to create a single node.
dns_entry *get_entry(zone_node *node,
                     IANA code); // Get entry vector according to IANA code.

void add_entry_data(dns_entry *entry, size_t TTL, char *field);
dns_data *get_entry_data(dns_entry *entry, size_t offset);
void dns_entry_prettyprint(dns_entry *entry);
IANA type_to_iana(char *type);

#endif /* !NODE_H_ */
