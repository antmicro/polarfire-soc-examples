/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef WEB_SERVER_ROUTES_H_
#define WEB_SERVER_ROUTES_H_

#include <stddef.h>

#include "http_structures.h"
#include "http_server.h"
#include "route_handlers.h"

/* The first route match will stop any further processing, therefore order the
 * routes from most specific first and more generic last */
static const http_route http_routes[] = {
  { .method = HTTP_METHOD_GET, .sub_routes = false,  .uri = "/index.html",   .cb = route_static  },
  { .method = HTTP_METHOD_GET, .sub_routes = false,  .uri = "/",             .cb = route_static  },
  { .method = HTTP_METHOD_GET, .sub_routes = false,  .uri = "/index-ne.htm", .cb = route_index  },
  { .method = HTTP_METHOD_GET, .sub_routes = false,  .uri = "/template.htm", .cb = route_template_index},
  { .method = HTTP_METHOD_GET, .sub_routes = false,  .uri = "/favicon.ico",  .cb = route_static },
  { .method = HTTP_METHOD_GET, .sub_routes = true,   .uri = "/resources",    .cb = route_static },
  { .method = HTTP_METHOD_GET, .sub_routes = true,   .uri = "/vendors",      .cb = route_static },
};

#define HTTP_ROUTES_SIZE (sizeof(http_routes)/sizeof(http_routes[0]))

static const HTTP_REQUEST_HANDLER fallback_404_cb = route_404;

#endif /* WEB_SERVER_ROUTES_H_ */
