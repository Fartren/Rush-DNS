#include "hashmap.h"

#include <assert.h>
#include <stdint.h>

#include "../errors.h"

// The hash function
__attribute__((no_sanitize("integer"))) uint64_t
hmap_hash_string(const unsigned char *key)
{
    assert(key != NULL);

    uint64_t ret = 5381U;

    unsigned char c;
    while ((c = *key++))
        ret = ((ret << 5U) + ret) + c;

    return ret % HASHMAP_SIZE;
}

// Used to init and return a new hashmap
hashmap hashmap_init(size_t size)
{
    hashmap hashmap;
    hashmap.size = size;
    hashmap.defined_soa = false;

    hashmap.table = malloc(size * sizeof(zone_node *));
    if (!hashmap.table)
    {
        error("Allocation of hashmap failed");
    }

    // Init:
    for (size_t i = 0; i < size; i++)
    {
        hashmap.table[i] = NULL;
    }
    return hashmap;
}

// Used to insert a node in the hashmap
void hashmap_insert(hashmap hashmap, zone_node *node)
{
    // Getting hashmap line from ndd:
    const char *ndd = node->name;
    int hash = hmap_hash_string((const unsigned char *)ndd);
    zone_node *first_node = hashmap.table[hash];

    // Inserting in head of the hashline:
    if (first_node == node)
        error("in JSON/Hashmap: Loop detected (wrong nexted node)\n");

    node->next = first_node;
    hashmap.table[hash] = node;
}

// Pretty printing a single node. Sub function of hashmap_prettyprint()
void hashmap_prettyprint_node(zone_node *node)
{
    if (node == NULL)
    {
        // printf("    EMPTY\n");
    }
    while (node != NULL)
    {
        printf("    name: %s | next: %p\n", node->name, (void *)node->next);
        node = node->next;
    }
}

// Pretty printing the whole hashmap
void hashmap_prettyprint(hashmap hashmap)
{
    for (size_t i = 0; i < hashmap.size; i++)
    {
        printf("Node %zu:\n", i);
        hashmap_prettyprint_node(hashmap.table[i]);
    }
}

void hashmap_free(hashmap hashmap)
{
    for (size_t i = 0; i < hashmap.size; i++)
    {
        zone_node *next_node = hashmap.table[i];
        while (next_node)
        {
            zone_node *temp = next_node;
            next_node = temp->next;
            for (size_t i = 0; i < IANA_CODE_COUNT; i++)
            {
                // freeing all fields for a specific entry:
                for (size_t j = 0; j < temp->entries[i].size; j++)
                {
                    free(temp->entries[i].data[j].field);
                }
                free(temp->entries[i].data);
            }
            free(temp);
        }
    }
    free(hashmap.table);
}

static zone_node *hashmap_get_zone_node(hashmap hashmap, const char *ndd)
{
    // Getting hashmap line from ndd:
    int hash = hmap_hash_string((const unsigned char *)ndd);
    zone_node *node = hashmap.table[hash];
    while (node)
    {
        if (strcmp(node->name, (const char *)ndd) == 0)
            return node;
        node = node->next;
    }
    return NULL;
}

dns_entry *hashmap_get_entry(hashmap hashmap, const char *ndd, IANA code)
{
    zone_node *node = hashmap_get_zone_node(hashmap, ndd);
    if (node == NULL)
        return NULL;
    return &node->entries[IANA_compress(code)];
}

// Return a new node if none is already allocated for the specified ndd.
// Else return the already created one.
zone_node *hashmap_try_create_zone(hashmap hashmap, const char *ndd)
{
    if (ndd == NULL)
        return NULL;
    zone_node *node = hashmap_get_zone_node(hashmap, ndd);
    if (node == NULL)
    {
        node = create_node(ndd);
        if (node != NULL)
            hashmap_insert(hashmap, node);
    }
    return node;
}
