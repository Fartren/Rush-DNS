#ifndef PARSE_JSON_H_
#define PARSE_JSON_H_

#include <arpa/inet.h>
#include <json-c/json.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "hashmap.h"

json_object *parse_json(char *path, struct hashmap *hashmap);

#endif /* !PARSE_JSON_H_ */
