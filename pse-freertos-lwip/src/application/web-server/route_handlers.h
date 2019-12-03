/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef WEB_SERVER_ROUTE_HANDLERS_H_
#define WEB_SERVER_ROUTE_HANDLERS_H_

#include "lwip/debug.h"

#ifndef HTTPD_ROUTE_HANDLER_DEBUG
#define HTTPD_ROUTE_HANDLER_DEBUG LWIP_DBG_ON
#endif

void route_index(http_request *request, http_response *response);
void route_static(http_request *request, http_response *response);
void route_template_index(http_request *req, http_response *res);
void route_404(http_request *request, http_response *response);

#endif /* WEB_SERVER_ROUTE_HANDLERS_H_ */
