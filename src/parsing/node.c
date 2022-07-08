#include "node.h"

#include "../errors.h"

// We need mutable so we can init it.
// NOLINTNEXTLINE (cppcoreguidelines-avoid-non-const-global-variables)
static int iana_compressed[iana_len] = { 0 };

static void init_iana_compressed(void)
{
    iana_compressed[A] = 0; // 1
    iana_compressed[NS] = 1; // 2
    iana_compressed[SOA] = 2; // 6
    iana_compressed[MX] = 3; // 15
    iana_compressed[TXT] = 4; // 16
    iana_compressed[AAAA] = 5; // 28
}

// Returning the compressed int for a IANA code
int IANA_compress(IANA code)
{
    return iana_compressed[code];
}

// MUST BE CALLED //FIXME: obligatoire(xelfi_)
int init_nodes(void)
{
    init_iana_compressed();
    return 0;
}

// IANA string version to IANA enum
IANA type_to_iana(char *type)
{
    if (strcmp(type, "A") == 0)
        return A;
    if (strcmp(type, "NS") == 0)
        return NS;
    if (strcmp(type, "MX") == 0)
        return MX;
    if (strcmp(type, "TXT") == 0)
        return TXT;
    if (strcmp(type, "AAAA") == 0)
        return AAAA;
    if (strcmp(type, "SOA") == 0)
        return SOA;
    return 0;
}

/* ======================= Node operations ======================= */

// Used to create and init a node, returning it.
zone_node *create_node(const char *name)
{
    zone_node *node = calloc(sizeof(zone_node), 1);
    node->name = name;

    for (int i = 0; i < IANA_CODE_COUNT; i++)
    {
        node->entries[i].size = 0;
        node->entries[i].capacity = DEFAULT_CAPACITY;
        node->entries[i].data = calloc(sizeof(dns_data), DEFAULT_CAPACITY);
        // node->entries[i].code = i; // FIXME (to remove xelfi_)
    }
    return node;
}

// Get a dns_entry of a certain IANA code for a specific node.
dns_entry *get_entry(zone_node *node, IANA code)
{
    return &node->entries[IANA_compress(code)];
}

/* ======================= DNS_ENTRY operations ======================= */

// Add a new dns_data{TTL, field} in a <dns_entry>
void add_entry_data(dns_entry *entry, size_t TTL, char *field)
{
    if (entry->size >= entry->capacity)
    {
        entry->capacity *= 2;
        entry->data = realloc(entry->data, sizeof(dns_data) * entry->capacity);
        if (!entry->data)
            error("Alerte rouge ! Realloc failed for a vector resize. New "
                  "capacity = %zu\n",
                  entry->capacity);
    }
    dns_data dns_record_data = { TTL, field };
    entry->data[entry->size] = dns_record_data;
    entry->size = entry->size + 1;
}

// Get a dns_data{TTL, field} at <offset> in a <dns_entry>
dns_data *get_entry_data(dns_entry *entry, size_t offset)
{
    if (offset >= entry->size)
    {
        return NULL;
    }
    return &entry->data[offset];
}

// Pretty print a full dns_entry
void dns_entry_prettyprint(dns_entry *entry)
{
    printf("Pretty printing dns entry:\n");
    if (!entry)
    {
        printf("NULL\n");
        return;
    }
    for (size_t i = 0; i < entry->size; i++)
    {
        dns_data data = entry->data[i];
        printf("%zu) TTL:%zu, Field: %s\n", i, data.TTL, data.field);
    }
}
