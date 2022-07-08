#include "parse_json.h"

#include "../errors.h"
#include "hashmap.h"

// TODO: ADD BOOL CREATION NODE

// Check for ipv6 address validity
bool isValidIpv6Address(char *ipAddress)
{
    struct sockaddr_in6 sa;
    int result = inet_pton(AF_INET6, ipAddress, &(sa.sin6_addr));
    return result != 0;
}

// Check for ipv4 address validity
bool isValidIpv4Address(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &sa.sin_addr);
    return result != 0;
}

// Check if data is valid for type
void check_data_type(char **data_str, int data_length, char *type,
                     json_object *record)
{
    if (strcmp(type, "A") == 0)
    {
        for (int i = 0; i < data_length; i++)
        {
            if (!isValidIpv4Address(data_str[i]))
            {
                error("bad IPV4 adress for record type 'A' in "
                      "record:\n%s\n",
                      json_object_get_string(record));
            }
        }
    }
    if (strcmp(type, "AAAA") == 0)
    {
        for (int i = 0; i < data_length; i++)
        {
            if (!isValidIpv6Address(data_str[i]))
            {
                error("bad IPV6 adress for record type 'AAAA' in "
                      "record:\n%s\n",
                      json_object_get_string(record));
            }
        }
    }
}

// Prepare data
void prepare_check_datas(json_object *data, int data_length, char *type,
                         json_object *record)
{
    char *data_str[data_length];

    for (int i = 0; i < data_length; i++)
    {
        data_str[i] =
            (char *)json_object_get_string(json_object_array_get_idx(data, i));
    }

    check_data_type(data_str, data_length, type, record);
}

// Parse a single record
int parse_record(json_object *record)
{
    struct json_object *name = NULL;
    struct json_object *type = NULL;
    struct json_object *ttl = NULL;
    struct json_object *data = NULL;
    size_t count = 0;

    json_object_object_foreach(record, key, val)
    {
        (void)key;
        count += 1;
    }

    if (count != 4)
    {
        error("Only 4 fields allowed in a record:\n%s\n",
              json_object_get_string(record));
    }

    if (!json_object_object_get_ex(record, "name", &name)
        || !json_object_object_get_ex(record, "type", &type)
        || !json_object_object_get_ex(record, "TTL", &ttl)
        || !json_object_object_get_ex(record, "data", &data))
    {
        error("Wrong field value in record:\n%s\n",
              json_object_get_string(record));
    }

    if (json_object_get_type(name) != json_type_string
        || json_object_get_type(type) != json_type_string
        || json_object_get_type(ttl) != json_type_int
        || json_object_get_type(data) != json_type_array)
    {
        error("bad field type in record\n%s\n", json_object_get_string(record));
    }

    // name checks
    char *name_str = (char *)json_object_get_string(name);
    if (!name_str || strcmp(name_str, "") == 0)
    {
        error("empty name in record\n%s\n", json_object_get_string(record));
    }

    // type checks
    char *type_str = (char *)json_object_get_string(type);

    if (strcmp(type_str, "A") != 0 && strcmp(type_str, "AAAA") != 0
        && strcmp(type_str, "SOA") != 0 && strcmp(type_str, "NS") != 0
        && strcmp(type_str, "TXT") != 0)
    {
        warn("Ignored record type: \n%s\n", type_str);
        return 1;
    }

    int ttl_int = json_object_get_int(ttl);
    if (ttl_int > INT_MAX)
    {
        error("Record TTL not valid\n%s\n", json_object_get_string(record));
    }

    // data checks
    int data_length = json_object_array_length(data);
    if (data_length == 0)
    {
        error("Empty data in record:\n%s\n", json_object_get_string(record));
    }

    prepare_check_datas(data, data_length, type_str, record);
    return 0;
}

char *get_name(json_object *record)
{
    struct json_object *name;
    json_object_object_get_ex(record, "name", &name);
    return (char *)json_object_get_string(name);
}

char *get_type(json_object *record)
{
    struct json_object *type;
    json_object_object_get_ex(record, "type", &type);
    return (char *)json_object_get_string(type);
}

int get_ttl(json_object *record)
{
    struct json_object *ttl;
    json_object_object_get_ex(record, "TTL", &ttl);
    return json_object_get_int(ttl);
}

char **get_data(json_object *record, size_t *len, int *data_int)
{
    struct json_object *data;
    json_object_object_get_ex(record, "data", &data);

    size_t data_length = json_object_array_length(data);

    char **data_str = malloc(sizeof(char *) * data_length);

    for (size_t i = 0; i < data_length; i++)
    {
        data_str[i] =
            (char *)json_object_get_string(json_object_array_get_idx(data, i));
        data_int[i] =
            json_object_get_string_len(json_object_array_get_idx(data, i));
    }
    *len = data_length;
    return data_str;
}

char *convert_name(char *name);
// Parse multiple records
void parse_records(json_object *zone, struct hashmap *hashmap)
{
    int zone_length = json_object_array_length(zone);

    for (int i = 0; i < zone_length; i++)
    {
        json_object *record = json_object_array_get_idx(zone, i);
        int skip_type = parse_record(record);

        if (skip_type)
            continue;

        char *name = get_name(record);
        char *type = get_type(record);
        int ttl = get_ttl(record);
        size_t length = 0;

        struct json_object *datao;
        json_object_object_get_ex(record, "data", &datao);
        size_t data_length = json_object_array_length(datao);
        int *data_int = malloc(sizeof(int) * data_length);

        char **data = get_data(record, &length, data_int);

        zone_node *node = hashmap_try_create_zone(*hashmap, convert_name(name));
        if (!node)
            error("In JSON detected in hashmap insertion.\n");

        IANA code = type_to_iana(type);
        dns_entry *entry = get_entry(node, code);

        // Checking doublon. Aka if entry size is > 0 before adding anything in
        // it.
        if (entry->size != 0)
            error("Erreur doublon\n");

        // Checking if defining SOA:
        if (code == SOA)
        {
            if (hashmap->defined_soa
                == true) // If defining SOA when it's also defined before, exit.
            {
                error("Double definition of SOA\n");
            }
            hashmap->defined_soa = true;
        }

        for (size_t i = 0; i < length; i++)
        {
            size_t len = data_int[i];
            char *field = calloc(len + 1, sizeof(char));
            strncpy(field, data[i], len);

            add_entry_data(entry, ttl, field);
        }
        free(data_int);
        free(data);
    }
}

// Parse JSON file
struct json_object *parse_json(char *path, struct hashmap *hashmap)
{
    FILE *fp;
    struct json_object *parsed_json;
    struct json_object *zone;

    fp = fopen(path, "r");

    if (fp == NULL)
        error("file '%s' does not exist\n", path);

    fseek(fp, 0, SEEK_END);

    int json_length = ftell(fp);

    if (json_length == 0)
    {
        fclose(fp);
        error("File is empty\n");
    }

    char buffer[json_length + 1];

    rewind(fp);

    size_t r = fread(buffer, json_length, 1, fp);
    (void)r; // TODO(xelfi_): check this result
    buffer[json_length] = '\0';

    fclose(fp);

    parsed_json = json_tokener_parse(buffer);

    if (!parsed_json)
    {
        error("Wrong JSON\n");
    }

    if (!json_object_object_get_ex(parsed_json, "Zone", &zone))
    {
        error("No 'Zone' field detected\n");
    }

    if (json_object_get_type(zone) != json_type_array)
    {
        error("'Zone' must be a list of records\n");
    }

    parse_records(zone, hashmap);
    // Checking if SOA have been defined once, if not: error
    if (hashmap->defined_soa == false)
    {
        error("No SOA defined\n");
    }
    return parsed_json;
}
