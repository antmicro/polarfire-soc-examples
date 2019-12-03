/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef WEB_SERVER_HTTP_SERVER_H_
#define WEB_SERVER_HTTP_SERVER_H_

#include <stdbool.h>
#include "http_structures.h"

#define HTTP_URI_MAX_LEN         255 /* https://stackoverflow.com/questions/417142 */
#define HTTP_RESPONSE_HDR_LEN    255 /* https://stackoverflow.com/questions/686217 */
#define HTTP_RESPONSE_BODY_LEN   4000
#define HTTP_QUERY_PAIRS         5
#define HTTP_QUERY_PAIR_STR_SIZE 20


/* https://tools.ietf.org/html/rfc3986#section-3.4 */
typedef struct {
    char key[HTTP_QUERY_PAIR_STR_SIZE];
    char value[HTTP_QUERY_PAIR_STR_SIZE];
} http_query_pair;


typedef struct {
    HTTP_REQUEST_METHOD_ENUM method;
    char                     *uri;
    char                     *query_starts;
    http_query_pair          *query_pairs;
    unsigned int             query_pairs_count;
    char                     *header;
    HTTP_CONTENT_TYPE_ENUM   content_type;
    int                      content_length;
    char                     *body;
} http_request;


typedef struct {
    HTTP_STATUS_CODE       status;
    HTTP_CONTENT_TYPE_ENUM content_type;
    int                    content_length;
    char                   *body;
    bool                   static_body;
    bool                   gziped;
} http_response;


typedef void (*HTTP_REQUEST_HANDLER)(http_request *, http_response *);


typedef struct {
    HTTP_REQUEST_METHOD_ENUM method;
    char                     *uri;
    bool                     sub_routes;
    HTTP_REQUEST_HANDLER     cb;
} http_route;


char *http_request_get_query_value(http_request *req, char *key);

#endif /* WEB_SERVER_HTTP_SERVER_H_ */
