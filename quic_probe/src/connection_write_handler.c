/*
 * connection_write_handler.c
 *
 *  Created on: Apr 18, 2018
 *      Author: sergei
 */
#include <memory.h>
#include <stdlib.h>
#include "connection_write_handler.h"
#include "engine_structs.h"

extern int is_debug;

void try_fix_empty_path(text_t **path_ref)
{
    if ((*path_ref)->size == 0)
    {
        destroy_text(*path_ref);
        *path_ref = init_text_from_const("/");
    }
}


void send_header(stream_parameters_t* stream_ref, lsquic_stream_t *stream)
{
    connection_parameters* connection = (connection_parameters*)stream_ref->connection_params_ref;
    try_fix_empty_path(&stream_ref->path);

    char* method = stream_ref->data_pointer->request_args->header_only? "HEAD" : "GET";

    lsquic_http_header_t headers_arr[] = {
            {
                    .name  = { .iov_base = ":method",       .iov_len = 7, },
                    .value = { .iov_base = (void *)method,          .iov_len = strlen(method), },
            },
            {
                    .name  = { .iov_base = ":scheme",       .iov_len = 7, },
                    .value = { .iov_base = (void *)"HTTP",          .iov_len = 4, }
            },
            {
                    .name  = { .iov_base = ":path",         .iov_len = 5, },
                    .value = { .iov_base = (void *) stream_ref->path->text,
                            .iov_len = stream_ref->path->size },
            },
            {
                    .name  = { ":authority",     10, },
                    .value = { .iov_base = (void *) connection->hostname->text,
                            .iov_len = connection->hostname->size },
            },


//            {
//                    .name  = { .iov_base = "accept",         .iov_len = 6, },
//                    .value = { .iov_base = (void *) "*/*",   .iov_len = 3 },
//            },
//            {
//                    .name  = { .iov_base = "content-type", .iov_len = 12, },
//                    .value = { .iov_base = "application/octet-stream", .iov_len = 24, },
//            },
//            {
//                    .name  = { .iov_base = "content-length", .iov_len = 14, },
//                    .value = { .iov_base = (void *) empty_payload,
//                            .iov_len = strlen(empty_payload), },
//            },
            // always redirects to youtube.com
//            {
//                    .name  = { .iov_base = "origin",         .iov_len = 6, },
//                    .value = { .iov_base = (void *) "https://www.youtube.com",   .iov_len = 23 },
//            },
//            {
//                    .name  = { .iov_base = "referer",         .iov_len = 7, },
//                    .value = { .iov_base = (void *) "https://www.youtube.com/",   .iov_len = 24 },
//            }
    };
    lsquic_http_headers_t headers = {
            .count = sizeof(headers_arr) / sizeof(headers_arr[0]),
            .headers = headers_arr,
    };
    if (0 != lsquic_stream_send_headers(stream, &headers, 1))
    {
        report_stream_error(init_text_from_const("cannot send headers"), errno, stream_ref,
                            init_text_from_const(__FILE__), __LINE__, QUIC_CLIENT_CODE_SOCKET_FAIL);
        return;
    }
}


void http_client_on_write (lsquic_stream_t *stream, lsquic_stream_ctx_t *st_h)
{
    if (stream == NULL)
        return;
    stream_parameters_t* stream_ref = (stream_parameters_t*) st_h;
    if (is_debug)
    {
        // test
        printf("stream on write:%s\n", stream_ref->data_pointer->final_url.url_data);
    }

    if (stream_ref->is_header_sent)
    {
        // close stream and read all incoming packets
        lsquic_stream_shutdown(stream, 1);
        lsquic_stream_wantread(stream, 1);
    }
    else
    {
        send_header(stream_ref, stream);
        stream_ref->is_header_sent = 1;
    }
}
