/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 */


#include <string.h>
#include <limits.h>
#include <stddef.h>


#include "http_structure_helpers.h"

/* Get key from a generic key/value structure. Returns INT_MIN if not found */
int64_t kv_int64_string_get_key(char                      *val,
                                const kv_int64_string_t   *structure,
                                int64_t                   max)
{
    for (int64_t index = 0; index < max; ++index)
    {
        if (strcmp(structure[index].val, val) == 0) return structure[index].key;
    }
    return INT_MIN;
}


/* Get value from a generic key/value structure. Returns NULL if not found */
char* kv_int64_string_get_val(int64_t                   key,
                              const kv_int64_string_t   *structure,
                              int64_t                   max)
{
    for (int64_t index = 0; index < max; ++index)
    {
        if ( structure[index].key == key ) return structure[index].val;
    }
    return NULL;
}


/* Convert the status code enum into a string representation */
char* get_status_code_string(HTTP_STATUS_CODE code)
{
    return kv_int64_string_get_val(
            code,
            HTTP_STATUS_CODE_ARRAY,
            HTTP_STATUS_CODE_ARRAY_SIZE);
}


/* Get the matching CONTENT TYPE ENUM from the given string */
HTTP_CONTENT_TYPE_ENUM http_content_type_enum_from_string(const char *content_type_str)
{
    HTTP_CONTENT_TYPE_ENUM iterate = HTTP_CONTENT_TYPE_NULL;
    for ( ; iterate < HTTP_CONTENT_TYPE_ENUM_END; iterate++)
    {
        if (strcmp(HTTP_CONTENT_TYPE_ARRAY[iterate], content_type_str) == 0)
        {
            return iterate;
        }
    }
    return HTTP_CONTENT_TYPE_NULL;
}


/* Get the matching REQUEST METHOD ENUM from the given string */
HTTP_REQUEST_METHOD_ENUM http_method_enum_from_string(const char *header_method_str)
{
    HTTP_REQUEST_METHOD_ENUM header_method = HTTP_METHOD_NULL;
    for ( ; header_method < HTTP_METHOD_ENUM_END; header_method++)
    {
        if (strcmp(HTTP_REQUEST_METHOD_ARRAY[header_method], header_method_str) == 0)
        {
            return header_method;
        }
    }
    return HTTP_METHOD_NULL;
}

/*
 * Route is matching with request when the methods are matching and the URI are
 * fully or partially matching (depending on the sub_routes value)
 */
bool is_route_matching_with_request(http_request *req, const http_route *route)
{
    return (req->method == route->method) &&
           ( ( route->sub_routes && strncmp(req->uri, route->uri, strlen(route->uri)) == 0 ) ||
             (!route->sub_routes && strcmp( req->uri, route->uri)                     == 0 )
           );
}

