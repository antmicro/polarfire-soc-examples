/*
 * Copyright 2019 Microchip Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * Generates enum of all possible HTTP request methods and their string
 * equivalents in an array (HTTP_METHOD).
 *
 * Generates enum and array for content-type (HTTP_CONTENT_TYPE)
 */

#ifndef WEB_SERVER_HTTP_STRUCTURES_H_
#define WEB_SERVER_HTTP_STRUCTURES_H_

#include <stdint.h>

/* ------------------ HTTP_REQUEST_METHOD ---------------------- */
#define ALL_HTTP_METHODS(ENUM_METHODS_ACTION) \
        ENUM_METHODS_ACTION(NULL) \
        ENUM_METHODS_ACTION(GET) \
        ENUM_METHODS_ACTION(HEAD) \
        ENUM_METHODS_ACTION(POST) \
        ENUM_METHODS_ACTION(PUT) \
        ENUM_METHODS_ACTION(DELETE) \
        ENUM_METHODS_ACTION(CONNECT) \
        ENUM_METHODS_ACTION(OPTIONS) \
        ENUM_METHODS_ACTION(TRACE) \
        ENUM_METHODS_ACTION(PATCH) \
        ENUM_METHODS_ACTION(ENUM_END)

/*
 * Longest method is 7 characters long
 */
#define HTTP_REQUEST_METHOD_MAX_STRING_LEN 7

#define CREATE_HTTP_METHOD_ENUM_ACTION(KEY) HTTP_METHOD_##KEY,
#define CREATE_HTTP_METHOD_ARRAY_ACTION(KEY) #KEY,

/*
 * It's a convoluted way to create a enum and a array, but this way both
 * HTTP_REQUEST_METHOD_ENUM and HTTP_REQUEST_METHOD_ARRAY are data-driven from
 * the same source and guaranteed to be consistent without any extra manual
 * maintenance.
 */

typedef enum
{
    ALL_HTTP_METHODS(CREATE_HTTP_METHOD_ENUM_ACTION)
} HTTP_REQUEST_METHOD_ENUM;

static const char *HTTP_REQUEST_METHOD_ARRAY[] =
{
    ALL_HTTP_METHODS(CREATE_HTTP_METHOD_ARRAY_ACTION)
};


/* ------------------ HTTP_STATUS_CODE ---------------------- */

typedef enum
{
    /* Info */
    HTTP_STATUS_CONTINUE                   = 100,

    /* Success */
    HTTP_STATUS_OK                         = 200,
    HTTP_STATUS_CREATED                    = 201,
    HTTP_STATUS_ACCEPTED                   = 202,
    HTTP_STATUS_NO_CONTENT                 = 204,

    /* Redirection */
    HTTP_STATUS_MOVED_PERMANENTLY          = 301,
    HTTP_STATUS_FOUND                      = 302,
    HTTP_STATUS_NOT_MODIFIED               = 304,

    /* Client Error */
    HTTP_STATUS_BAD_REQUEST                = 400,
    HTTP_STATUS_UNAUTHORIZED               = 401,
    HTTP_STATUS_FORBIDDEN                  = 403,
    HTTP_STATUS_NOT_FOUND                  = 404,

    /* Server Error */
    HTTP_STATUS_INTERNAL_SERVER_ERROR      = 500,
    HTTP_STATUS_NOT_IMPLEMENTED            = 501,
    HTTP_STATUS_BAD_GATEWAY                = 502,
    HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED = 505,
} HTTP_STATUS_CODE;


typedef struct {
    int64_t key;
    char     *val;
} kv_int64_string_t;


static const kv_int64_string_t HTTP_STATUS_CODE_ARRAY[] =
{
    { .key=HTTP_STATUS_CONTINUE,                   .val="Continue" },

    { .key=HTTP_STATUS_OK,                         .val="OK" },
    { .key=HTTP_STATUS_CREATED,                    .val="Created" },
    { .key=HTTP_STATUS_ACCEPTED,                   .val="Accepted" },
    { .key=HTTP_STATUS_NO_CONTENT,                 .val="No Content" },

    { .key=HTTP_STATUS_MOVED_PERMANENTLY,          .val="Moved Permanently" },
    { .key=HTTP_STATUS_FOUND,                      .val="Found" },
    { .key=HTTP_STATUS_NOT_MODIFIED,               .val="Not Modified" },

    { .key=HTTP_STATUS_BAD_REQUEST,                .val="Bad Request" },
    { .key=HTTP_STATUS_UNAUTHORIZED,               .val="Unauthorized" },
    { .key=HTTP_STATUS_FORBIDDEN,                  .val="Forbidden" },
    { .key=HTTP_STATUS_NOT_FOUND,                  .val="Not Found" },

    { .key=HTTP_STATUS_INTERNAL_SERVER_ERROR,      .val="Internal Server Error" },
    { .key=HTTP_STATUS_NOT_IMPLEMENTED,            .val="Not Implemented" },
    { .key=HTTP_STATUS_BAD_GATEWAY,                .val="Bad Gateway" },
    { .key=HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED, .val="HTTP Version Not Supported" },
};

#define HTTP_STATUS_CODE_ARRAY_SIZE (sizeof(HTTP_STATUS_CODE_ARRAY) / sizeof(HTTP_STATUS_CODE_ARRAY[0]))


/* ------------------ HTTP_CONTENT_TYPE ---------------------- */


#define ALL_HTTP_CONTENT_TYPES(ENUM_CONTENT_ACTION) \
        ENUM_CONTENT_ACTION(NULL,       null) \
        ENUM_CONTENT_ACTION(TEXT_HTML,  text/html) \
        ENUM_CONTENT_ACTION(TEXT_PLAIN, text/plain) \
        ENUM_CONTENT_ACTION(TEXT_CSS,   text/css) \
        ENUM_CONTENT_ACTION(TEXT_JS,    text/javascript) \
        ENUM_CONTENT_ACTION(IMAGE_JPEG, image/jpg) \
        ENUM_CONTENT_ACTION(IMAGE_GIF,  image/gif) \
        ENUM_CONTENT_ACTION(IMAGE_PNG,  image/png) \
        ENUM_CONTENT_ACTION(IMAGE_ICO,  image/vnd.microsoft.icon) \
        ENUM_CONTENT_ACTION(IMAGE_XICO, image/x-icon) \
        ENUM_CONTENT_ACTION(ENUM_END,   end)


#define CREATE_HTTP_CONTENT_TYPE_ENUM_ACTION(ENUM_KEY, STRING_KEY) HTTP_CONTENT_TYPE_##ENUM_KEY,
#define CREATE_HTTP_CONTENT_TYPE_ARRAY_ACTION(ENUM_KEY, STRING_KEY) #STRING_KEY,

/*
 * It's a convoluted way to create a enum and a array, but this way both
 * HTTP_REQUEST_METHOD_ENUM and HTTP_REQUEST_METHOD_ARRAY are data-driven from
 * the same source and guaranteed to be consistent without any extra manual
 * maintenance.
 */

typedef enum
{
    ALL_HTTP_CONTENT_TYPES(CREATE_HTTP_CONTENT_TYPE_ENUM_ACTION)
} HTTP_CONTENT_TYPE_ENUM;

static const char *HTTP_CONTENT_TYPE_ARRAY[] =
{
    ALL_HTTP_CONTENT_TYPES(CREATE_HTTP_CONTENT_TYPE_ARRAY_ACTION)
};


#endif /* WEB_SERVER_HTTP_REQUEST_TYPES_H_ */
