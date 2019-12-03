/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 */


#ifndef WEB_SERVER_HTTP_STRUCTURE_HELPERS_H_
#define WEB_SERVER_HTTP_STRUCTURE_HELPERS_H_

#include <stdbool.h>
#include <stddef.h>

#include "http_server.h"
#include "routes.h"

int64_t kv_int64_string_get_key(char                    *val,
                                const kv_int64_string_t *structure,
                                int64_t                 max);


char* kv_int64_string_get_val(int64_t                 key,
                              const kv_int64_string_t *structure,
                              int64_t                 max);


char* get_status_code_string(HTTP_STATUS_CODE code);


HTTP_CONTENT_TYPE_ENUM http_content_type_enum_from_string(const char *content_type_str);


HTTP_REQUEST_METHOD_ENUM http_method_enum_from_string(const char *header_method_str);


bool is_route_matching_with_request(http_request *request, const http_route *route);


#endif /* WEB_SERVER_HTTP_STRUCTURE_HELPERS_H_ */
