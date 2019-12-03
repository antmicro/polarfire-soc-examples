/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * SVN $Revision: 4971 $
 * SVN $Date: 2013-01-11 22:14:31 +0000 (Fri, 11 Jan 2013) $
 */
#include <assert.h>

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "config/hardware/hw_platform.h"

#include "mpfs_hal/mss_coreplex.h"
#include "mpfs_hal/mss_plic.h"

#include "config/software/drivers/mss_mac/mss_ethernet_mac_user_config.h"
#include "drivers/mss_mac/mac_registers.h"
#include "drivers/mss_mac/pse_mac_regs.h"
#include "drivers/mss_mac/mss_ethernet_mac.h"

#include "utils/configure_network.h"
#include "web-server/http_structures.h"

#include "httpserver-netconn.h"
#include "http_structures.h"
#include "http_structure_helpers.h"
#include "http_server.h"
#include "routes.h"
#include "httpserver-netconn.h"


#if LWIP_NETCONN


void http_server_parse_query_pairs(http_request *req)
{
    /* This query parses doesn't handle the escaped & = ? properly and
     * could be improved by allocating the memory dynamically instead
     * of depending on fixed memory buffers.
     *
     * URI query key/value pair example:
     * index.htm?first_arg=value_for_this_arg&second=value2
     *           ^        ^                  ^
     *           begin    middle             end                     */

    bool do_query_parsing = true;
    char *query_begin = req->query_starts + 1;
    int pair_idx = 0;

    while (pair_idx < HTTP_QUERY_PAIRS && do_query_parsing)
    {
        char *query_middle = strchr(query_begin, '=');

        if (query_middle == NULL)
        {
            /* If we didn't find valid pair, then stop processing */
            break;
        }

        char *query_end = strchr(query_middle, '&');

        if (query_end == NULL)
        {
            /* Did not find next query, use the string length instead */
            query_end = query_middle + strlen(query_middle);
            do_query_parsing = false;
        }

        /* key=value pair found */
        /* copy key into the structure */
        strncpy(req->query_pairs[pair_idx].key,
               query_begin,
               query_middle - query_begin);

        /* Making sure the key string is null terminated */
        req->query_pairs[pair_idx].key[query_middle - query_begin] = '\0';

        /* Copy value into the structure. Start with offset +1 to ignore first
         * character which should be = and copy 1 less to compensate */
        strncpy(req->query_pairs[pair_idx].value,
               query_middle + 1,
               query_end - query_middle - 1);

        /* Making sure the value string is null terminated */
        req->query_pairs[pair_idx].value[query_end - query_middle - 1] = '\0';

        /* next query starts where the current ended */
        query_begin = query_end + 1;
        pair_idx++;
    }
    req->query_pairs_count = pair_idx;
}


void http_server_parse_request
(
        char          *input_buffer,
        u16_t         input_buffer_len,
        http_request  *req
)
{
    /* Longest HTTP method is 7 characters long, +1 for string termination*/
    char header_method_str[HTTP_REQUEST_METHOD_MAX_STRING_LEN+1];

    /* Separate the request method from the header and calculate where
     * parsing of the header should continue.
     */
    int method_end = 0;
    for (; method_end < HTTP_REQUEST_METHOD_MAX_STRING_LEN; method_end++)
    {
        if (input_buffer[method_end] == ' ')
        {
            break;
        }
    }

    strncpy(header_method_str, input_buffer, method_end);
    header_method_str[method_end] = 0;

    /* Match the method STRING with the method ENUM*/
    req->method = http_method_enum_from_string(header_method_str);

    /* Parse the URI part, start at method_end + 1 space */
    int uri_end = method_end + 1;
    for (; input_buffer[uri_end] != ' '; uri_end++);
    strncpy(req->uri, input_buffer+method_end+1, uri_end-method_end-1);
    req->uri[uri_end-method_end-1] = 0;

    LWIP_DEBUGF(HTTPD_DEBUG,("http_server_parse_request: Parsing %s %s\r\n",
            header_method_str, req->uri));

    /* Find if the URI contains ? question mark, NULL if not present */
    req->query_starts = strchr(req->uri, '?');
    if (req->query_starts != NULL)
    {
        /* If query string exists inside the URI then terminate the URI before
         * the query starts. */
        *req->query_starts = '\0';

        /* Then parse all the key/value pairs */
        http_server_parse_query_pairs(req);
    }

    /* TODO parse the rest of the header and cover edge cases, check for
     * content_type and length on the requests as well. Populate the body
     * if it's present... */
}


void http_server_execute_matching_route
(
        http_request  *req,
        http_response *res
)
{
    /* Go through all routes and call the callback on the matches */
    bool route_found = false;
    for (uint64_t route_ix = 0; route_ix < HTTP_ROUTES_SIZE; ++route_ix)
    {
        if (is_route_matching_with_request(req, &http_routes[route_ix]))
        {
            /* Matching route found for the request, execute the callback */
            LWIP_DEBUGF(HTTPD_ROUTE_DEBUG,("http_server_execute_matching_route: Serving matched route\r\n"));
            http_routes[route_ix].cb(req, res);
            route_found = true;
            break;
        }
    }

    /* No matching route found, execute the fallback callback show 404 */
    if (!route_found)
    {
        LWIP_DEBUGF(HTTPD_ROUTE_DEBUG,("http_server_execute_matching_route: Handling 404\r\n"));
        fallback_404_cb(req, res);
    }
}


void http_server_send_response(struct netconn *conn, http_response *res)
{
    /* The page handler was executed, now create the response */

    if (res->content_length == -1)
    {
        /* Calculate the content type length, if the body is populated and the
         * content length was not set already */
        res->content_length = (res->body == NULL) ? 0 : strlen(res->body);
    }

    /* Generate response buffer from the response structure */
    char http_hdr_buf[HTTP_RESPONSE_HDR_LEN];

    sprintf(http_hdr_buf, "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n",
            res->status,
            get_status_code_string(res->status),
            HTTP_CONTENT_TYPE_ARRAY[res->content_type],
            res->content_length);

    if (res->gziped)
    {
        strcat(http_hdr_buf, "Content-Encoding: gzip\r\n");
    }

    strcat(http_hdr_buf, "\r\n");

    /* Send the HTTP header.
     * https://www.nongnu.org/lwip/2_0_x/group__netconn__tcp.html
     * NETCONN_NOFLAG: For const static data, so no need to copy the data.
     * NETCONN_COPY:   For dynamic data pointers which needs to be copied. */
    netconn_write(conn, http_hdr_buf, strlen(http_hdr_buf), NETCONN_COPY);

    /* Send the body if it's populated and the length is a positive integer */
    if (res->body != NULL && res->content_length > 0)
    {
        /* Copy the data or not depending if the body contains pointer to
         * static const data or a dynamic data in a stack. */
        if (res->static_body) {
            netconn_write(conn, res->body, res->content_length, NETCONN_NOFLAG);
        }
        else {
            netconn_write(conn, res->body, res->content_length, NETCONN_COPY);
        }
    }
}


static void
http_server_netconn_serve
(
    struct netconn *conn
)
{
    struct netbuf *input_netbuf;
    char *input_buffer;
    u16_t input_buffer_len;
    err_t err;

    /* Read the data from the port, blocking if nothing yet there. 
    We assume the request (the part we care about) is in one netbuf */
    err = netconn_recv(conn, &input_netbuf);


    if(err == ERR_OK)
    {
        char header_uri[HTTP_URI_MAX_LEN];
        http_query_pair http_query_pairs[HTTP_QUERY_PAIRS];
        char response_buffer[HTTP_RESPONSE_BODY_LEN];

        /* Prepare the request structure to be populated as the request is parsed */
        http_request request = {
                .method            = HTTP_METHOD_NULL,
                .uri               = header_uri,
                .query_starts      = NULL,
                .query_pairs       = http_query_pairs,
                .query_pairs_count = 0,
                .header            = NULL,
                .content_type      = HTTP_CONTENT_TYPE_NULL,
                .content_length    = 0,
                .body              = NULL,
        };

        netbuf_data(input_netbuf, (void**)&input_buffer, &input_buffer_len);
        /* Read the packet and now parsing it. Assumption/simplification was
         * made that the whole request does fit into a single packet.
         * More robust web-server should handle multi-packet requests. */

        http_server_parse_request(input_buffer, input_buffer_len, &request);

        /* Route handler will populate the response, but give it default values.
         * The content_length is set to -1, when its untouched by the handler
         * then the post handler logic will try to detect it's size, this works
         * well with strings, but not with binary data. If handler is going to
         * send binary data then content_length needs to be set by the handler.
         * If it's set to 0 it will handle case where response has no body  */
        http_response response = {
                .status         = HTTP_STATUS_OK,
                .content_type   = HTTP_CONTENT_TYPE_TEXT_HTML,
                .content_length = -1,
                .body           = response_buffer,
                .static_body    = false,
                .gziped         = false,
        };

        http_server_execute_matching_route(&request, &response);

        http_server_send_response(conn, &response);
    }
    /* Close the connection (server closes in HTTP) */
    netconn_close(conn);
  
    /* Delete the buffer (netconn_recv gives us ownership,
      so we have to make sure to deallocate the buffer) */
    netbuf_delete(input_netbuf);
}


/* The main networking thread function, never returns! */
void
http_server_netconn_thread(void *arg)
{
    struct netconn *conn, *newconn;
    err_t err;
    (void)arg;

    /* Create a new TCP connection handle */
#if LWIP_IPV6
    /*
     * The following will allow connection with IPV6 and IPV4 so works for dual
     * stack as well.
     * */
    conn = netconn_new(NETCONN_TCP_IPV6);
    LWIP_ERROR("http_server: invalid conn", (conn != NULL), return;);
  
    /* Bind to port 80 (HTTP) with default IP address */
    netconn_bind(conn, IP6_ADDR_ANY, 80);
#else /* Must be IPV4 only */
    /* Create a new TCP connection handle */
    conn = netconn_new(NETCONN_TCP);
    LWIP_ERROR("http_server: invalid conn", (conn != NULL), return;);
  
    /* Bind to port 80 (HTTP) with default IP address */
    netconn_bind(conn, IP_ADDR_ANY, 80);
#endif
    /* Put the connection into LISTEN state */
    LWIP_DEBUGF(HTTPD_DEBUG,("HTTP server is listening\r\n"));
    netconn_listen(conn);
  
    do {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK)
        {
            http_server_netconn_serve(newconn);
            netconn_delete(newconn);
        }
    } while(err == ERR_OK);
    LWIP_DEBUGF(HTTPD_DEBUG,
                ("http_server_netconn_thread: netconn_accept received error %d, shutting down!\r\n",
                err));
    netconn_close(conn);
    netconn_delete(conn);
}


/** Initialize the HTTP server (start its thread) */
void
http_server_netconn_init(void)
{
  sys_thread_new("http_server_netconn", http_server_netconn_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
}

#endif /* LWIP_NETCONN*/
