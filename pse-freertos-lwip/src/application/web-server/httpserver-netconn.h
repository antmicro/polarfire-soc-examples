/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef __HTTPSERVER_NETCONN_H__
#define __HTTPSERVER_NETCONN_H__

#include "lwip/debug.h"

#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG         LWIP_DBG_ON
#endif

#ifndef HTTPD_ROUTE_DEBUG
#define HTTPD_ROUTE_DEBUG   LWIP_DBG_OFF
#endif

void http_server_netconn_init(void);
void http_server_netconn_thread(void *arg);

/*------------------------------------------------------------------------------
 * External functions.
 */

/*------------------------------------------------------------------------------
 *
 */
void handle_trigger_request(char * buf, u16_t len);

#include "http_server.h"

void http_server_parse_request
(
        char          *input_buffer,
        u16_t         input_buffer_len,
        http_request  *rq
);

#endif /* __HTTPSERVER_NETCONN_H__ */
