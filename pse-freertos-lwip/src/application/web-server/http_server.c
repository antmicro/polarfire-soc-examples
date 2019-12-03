/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <stddef.h>
#include <string.h>

#include "http_server.h"

char *http_request_get_query_value(http_request *req, char *key)
{
    for (int i=0; i<req->query_pairs_count; i++)
    {
        if (0 == strcmp(req->query_pairs[i].key, key))
        {
            return req->query_pairs[i].value;
        }
    }
    return NULL;
}

